#include "Core/Editor.h"
#include "Core/Theme.h"

#include <QApplication>
#include <QGuiApplication>
#include <QStyleFactory>
#include <QSurfaceFormat>

namespace
{
QtMessageHandler g_PreviousMessageHandler = nullptr;

// EngineViewportWidget embeds a QWindow via createWindowContainer(), which forces its sibling dock
// areas to become native QWidgetWindows too (see that class's own comment for why the alternatives
// don't work). Qt's QWindow::setTransientParent() refuses to use one of those as a popup's
// transient parent and logs this warning every time a context menu opens anywhere near the
// viewport - harmlessly (Qt just leaves the popup without a transient parent) but constantly. This
// is an uncategorized qWarning(), so a logging-category filter can't target it alone; filtering the
// message text in a handler is the only selective option, so everything else still reaches Qt's own
// default handler unchanged.
void FilterAdsTransientParentWarning(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    if (type == QtWarningMsg && message.contains("must be a top level window"))
        return;

    if (g_PreviousMessageHandler)
        g_PreviousMessageHandler(type, context, message);
}
}  // namespace

// Does not include Core/EntryPoint.h: Qt requires QApplication to exist before any Qt object is
// constructed, and to own its own exec() loop - both incompatible with EntryPoint.h's generic
// main() (which unconditionally calls Application::Run()'s blocking loop). Editor's own QTimer
// calls Application::Tick() directly instead - see Editor's constructor.
int main(int argc, char** argv)
{
    g_PreviousMessageHandler = qInstallMessageHandler(FilterAdsTransientParentWarning);

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    // Must be set before QApplication is constructed. Avoids fractional-DPI rendering artifacts
    // in Qt6's automatic high-DPI scaling.
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);

    // Must be set before QApplication is constructed. Matches SDLWindow's explicit
    // SDL_GL_CONTEXT_MAJOR/MINOR_VERSION + PROFILE_MASK=CORE request - required for
    // EngineViewportWidget's own Qt-owned GL context to be compatible enough with the engine's
    // real SDL-owned context for WGL sharing to succeed (see EngineViewportWidget::
    // EnsureGLResourcesInitialized()), on top of the original reasoning: without this, Qt
    // negotiates some default context that may not match what glad was loaded against.
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4, 6);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication qapp(argc, argv);

    // Fusion draws everything in pure Qt with no native Windows GDI/theme calls - avoids a whole
    // class of native-integration bugs (e.g. HICON/HBITMAP-to-QPixmap conversion asserts) that
    // the default "windowsvista" style's native theming can hit.
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // Both must run before MatchaEditor::Editor (and thus EditorMainWindow, and thus any dock
    // widget/tab) is constructed below - see ApplyEditorFont's own comment for why the font
    // specifically can't just be applied later.
    MatchaEditor::ApplyEditorFont(qapp);

    // Our own dark QPalette, not an external stylesheet library - see Theme.cpp. Qt-Advanced-
    // Docking-System (EditorMainWindow.cpp) separately ships its own dark theme for dock chrome
    // (title bars, tab bars, close/float buttons); this covers every widget inside a panel.
    MatchaEditor::ApplyDarkTheme(qapp);

    Matcha::Application::ApplicationSpecification spec;
    spec.title = "Matcha Editor";
    spec.windowBackend = Matcha::WindowBackend::SDL;
    // Never shows a native SDL window - the engine renders into its own FrameBuffer instead, which
    // EngineViewportWidget displays via a separate, small Qt-owned GL context. See Editor.
    spec.headless = true;
    spec.commandLineArgs = {argc, argv};

    MatchaEditor::Editor editor(spec);
    editor.Show();

    return qapp.exec();
}
