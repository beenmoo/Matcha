#include "EditorMainWindow.h"
#include "ConsoleSink.h"
#include "Panels/ConsolePanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/MenuChrome.h"
#include "Core/Logger.h"
#include "Core/Qt/QtViewportWidget.h"

#include <spdlog/spdlog.h>

#include <DockAreaWidget.h>
#include <DockManager.h>

#include <QCloseEvent>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QString>

#include <algorithm>

namespace MatchaEditor
{
namespace
{
void AttachSink(const spdlog::sink_ptr& sink, const char* loggerName)
{
    if (auto logger = spdlog::get(loggerName))
        logger->sinks().push_back(sink);
}

void DetachSink(const spdlog::sink_ptr& sink, const char* loggerName)
{
    if (auto logger = spdlog::get(loggerName))
    {
        auto& sinks = logger->sinks();
        sinks.erase(std::remove(sinks.begin(), sinks.end(), sink), sinks.end());
    }
}

// A plain Qt-drawn pixmap, not extracted from any native HICON - sidesteps a Qt/Windows bug
// where converting certain system icons to a QPixmap asserts on the AND-mask's bitmap format.
QIcon MakePlaceholderIcon()
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(0x6a, 0x9a, 0x4a));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(pixmap.rect().adjusted(2, 2, -2, -2));

    return QIcon(pixmap);
}

// Qt-Advanced-Docking-System's CDockManager sets its own default stylesheet directly on itself
// during construction (an embedded resource, src/stylesheets/default.css upstream) - it leaves
// every tab/title-bar button icon at 16px, which is what makes tabs read as "big". Appending
// these rules after construction shrinks just the icons: later rules win Qt's CSS cascade for
// equal-specificity selectors, so this doesn't need to replace ADS's own sheet, just follow it.
// The tab's left margin is a second, smaller contributor - it's computed from QFontMetrics of
// the application's default font at each tab's construction time (DockWidgetTabPrivate::
// createLayout(), confirmed by reading ADS 4.5.0's actual source), not stylesheet-driven, so it
// can only shrink via a global QApplication font change - left alone here since that would
// affect text everywhere, not just tabs.
QString CompactDockChromeStyleSheet()
{
    return QStringLiteral(
        "ads--CTitleBarButton { qproperty-iconSize: 12px; }"
        "#tabCloseButton { qproperty-iconSize: 12px; padding: 0px -4px; }"
        "#tabsMenuButton { qproperty-iconSize: 12px; }"
        "#dockAreaCloseButton { qproperty-iconSize: 12px; }"
        "#detachGroupButton { qproperty-iconSize: 12px; }"
        "#dockAreaAutoHideButton { qproperty-iconSize: 12px; }"
        "#dockAreaMinimizeButton { qproperty-iconSize: 12px; }");
}
}  // namespace

EditorMainWindow::EditorMainWindow(Matcha::EngineContext& context, Matcha::QtViewportWidget* viewport, QWidget* parent)
    : QMainWindow(parent),
      m_Context(context)
{
    setWindowIcon(MakePlaceholderIcon());
    resize(1600, 900);

    // Qt-Advanced-Docking-System (ads::CDockManager) replaces QMainWindow's own dock-widget
    // system entirely - it becomes the central widget, and every panel (viewport included) is
    // one of its dock widgets. This sidesteps the whole class of QMainWindow dock-area quirks
    // (central-widget-size-hint-dependent splitter guessing, docks only splitting within their
    // own top-level area) that the previous QDockWidget-based layout kept running into.
    m_DockManager = new ads::CDockManager(this);
    m_DockManager->setStyleSheet(m_DockManager->styleSheet() + CompactDockChromeStyleSheet());
    setCentralWidget(m_DockManager);

    MenuChrome menuChrome(this, context);

    // Unlike Scene::AddOnSceneChanged (which InspectorPanel/SceneHierarchyWidget have to
    // re-subscribe to on every scene swap, since it dies with the Scene it's attached to), these
    // two are SceneManager's own callbacks - SceneManager itself outlives any one Scene, so a
    // single subscription here covers every scene for the rest of the editor's lifetime.
    UpdateWindowTitle();
    context.GetSceneManager().AddOnDirtyChanged([this] { UpdateWindowTitle(); });
    context.GetSceneManager().AddOnSceneReplaced([this] { UpdateWindowTitle(); });

    // Built left-to-right by passing each previous call's returned CDockAreaWidget as the next
    // one's placement target, so Scene Hierarchy/Viewport/Inspector land in that explicit order
    // instead of being stacked/tabbed together. Each panel is also registered with menuChrome
    // right after creation, so View > Panels lists it and it can be reopened if closed.
    SceneHierarchyPanel* sceneHierarchyPanel = new SceneHierarchyPanel(m_DockManager, context, this);
    ads::CDockAreaWidget* sceneHierarchyArea = m_DockManager->addDockWidget(ads::LeftDockWidgetArea, sceneHierarchyPanel);
    menuChrome.AddPanel(sceneHierarchyPanel);

    ViewportPanel* viewportPanel = new ViewportPanel(m_DockManager, viewport, this);
    ads::CDockAreaWidget* viewportArea = m_DockManager->addDockWidget(ads::RightDockWidgetArea, viewportPanel, sceneHierarchyArea);
    menuChrome.AddPanel(viewportPanel);

    InspectorPanel* inspectorPanel = new InspectorPanel(m_DockManager, context, this);
    ads::CDockAreaWidget* inspectorArea = m_DockManager->addDockWidget(ads::RightDockWidgetArea, inspectorPanel, viewportArea);
    menuChrome.AddPanel(inspectorPanel);
    connect(sceneHierarchyPanel, &SceneHierarchyPanel::SelectionChanged, inspectorPanel, &InspectorPanel::SetSelectedEntities);

    ConsolePanel* consolePanel = new ConsolePanel(m_DockManager, this);
    ads::CDockAreaWidget* consoleArea = m_DockManager->addDockWidget(ads::BottomDockWidgetArea, consolePanel);
    menuChrome.AddPanel(consolePanel);

    // Initial panel sizes - ADS has no per-dock-widget size to set at addDockWidget() time, only
    // this, after the fact, against whichever splitter a given area ends up in. Values are
    // proportions (QSplitter::setSizes() semantics), not exact pixels - Qt scales them to
    // whatever space is actually available once the window is shown, they don't need to sum to
    // the window size. sceneHierarchyArea/viewportArea/inspectorArea share one horizontal
    // splitter (3 sizes); consoleArea's splitter has 2 children: that whole horizontal row as
    // one item, and Console as the other.
    m_DockManager->setSplitterSizes(sceneHierarchyArea, {250, 1000, 350});
    m_DockManager->setSplitterSizes(consoleArea, {700, 200});

    // Not parented to the console widget - it's owned by this shared_ptr and by whichever
    // logger sinks() vectors hold a copy, so ownership can't be split with Qt's parent/child
    // deletion. The connection is torn down safely regardless, since QObject disconnects its
    // signals/slots automatically once the receiving widget is destroyed.
    m_ConsoleSink = std::make_shared<ConsoleSink>();
    connect(m_ConsoleSink.get(), &ConsoleSink::MessageLogged, consolePanel, &ConsolePanel::AppendMessage);

    AttachSink(m_ConsoleSink, MT_CORE_LOGGER);
    AttachSink(m_ConsoleSink, MT_CLIENT_LOGGER);
}

EditorMainWindow::~EditorMainWindow()
{
    DetachSink(m_ConsoleSink, MT_CORE_LOGGER);
    DetachSink(m_ConsoleSink, MT_CLIENT_LOGGER);
}

void EditorMainWindow::closeEvent(QCloseEvent* event)
{
    if (ConfirmDiscardUnsavedChanges(this, m_Context))
        event->accept();
    else
        event->ignore();
}

void EditorMainWindow::UpdateWindowTitle()
{
    Matcha::SceneManager& sceneManager = m_Context.GetSceneManager();

    QString sceneName = sceneManager.GetFilePath().empty()
                             ? "Untitled"
                             : QFileInfo(QString::fromStdString(sceneManager.GetFilePath())).completeBaseName();

    setWindowTitle(QString("%1%2 - Matcha Editor").arg(sceneManager.IsDirty() ? "*" : "", sceneName));
}
}  // namespace MatchaEditor
