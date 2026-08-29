#include "EditorMainWindow.h"
#include "ConsoleSink.h"
#include "Panels/ConsolePanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Core/Logger.h"
#include "Core/Qt/QtViewportWidget.h"

#include <spdlog/spdlog.h>

#include <QDockWidget>
#include <QMenuBar>
#include <QPainter>
#include <QPixmap>

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
}  // namespace

EditorMainWindow::EditorMainWindow(Matcha::Scene& scene, Matcha::QtViewportWidget* viewport, QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Matcha Editor");
    setWindowIcon(MakePlaceholderIcon());
    resize(1600, 900);

    setCentralWidget(viewport);

    QMenuBar* menuBar = new QMenuBar(this);
    menuBar->addMenu("File");
    menuBar->addMenu("Edit");
    menuBar->addMenu("View");
    setMenuBar(menuBar);

    // Inspector stays a stub for now - populated once the editor has per-entity property UI.
    SceneHierarchyPanel* sceneHierarchyPanel = new SceneHierarchyPanel(scene, this);
    addDockWidget(Qt::LeftDockWidgetArea, sceneHierarchyPanel);
    InspectorPanel* inspectorPanel = new InspectorPanel(this);
    addDockWidget(Qt::RightDockWidgetArea, inspectorPanel);
    ConsolePanel* consolePanel = new ConsolePanel(this);
    addDockWidget(Qt::BottomDockWidgetArea, consolePanel);

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
}  // namespace Matcha
