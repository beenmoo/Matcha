#include "Core/Editor.h"

#include <QApplication>
#include <QFile>
#include <QGuiApplication>
#include <QStyleFactory>
#include <QSurfaceFormat>
#include <QTextStream>

// Does not include Core/EntryPoint.h: Qt requires QApplication to exist before any Qt object is
// constructed, and to own its own exec() loop - both incompatible with EntryPoint.h's generic
// main() (which unconditionally calls Application::Run()'s blocking loop). See Application::Tick()
// / Window::SetTickCallback for how the editor drives the engine loop instead.
int main(int argc, char** argv)
{
    // Must be set before QApplication is constructed. Avoids fractional-DPI rendering artifacts
    // in Qt6's automatic high-DPI scaling.
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);

    // Must be set before QApplication is constructed. Matches SDLWindow's explicit
    // SDL_GL_CONTEXT_MAJOR/MINOR_VERSION + PROFILE_MASK=CORE request - without this, Qt
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
    // the default "windowsvista" style's native theming can hit. Also QDarkStyleSheet's own
    // documented base style - its QSS is written assuming Fusion's rendering, not a native one.
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // QDarkStyleSheet (Vendor/qdarkstyle, compiled into the binary via AUTORCC - see
    // MatchaEditor/CMakeLists.txt) - the single source of truth for widget theming from here on,
    // applied once, app-wide, rather than each widget carrying its own hand-rolled stylesheet.
    QFile darkStyleSheet(":/qdarkstyle/dark/darkstyle.qss");

    if (darkStyleSheet.open(QFile::ReadOnly | QFile::Text))
        qApp->setStyleSheet(QTextStream(&darkStyleSheet).readAll());
    else
        MT_CORE_WARN("Failed to load QDarkStyleSheet - falling back to unstyled Fusion.");

    Matcha::Application::ApplicationSpecification spec;
    spec.title = "Matcha Editor";
    spec.windowBackend = Matcha::WindowBackend::Qt;
    spec.commandLineArgs = {argc, argv};

    MatchaEditor::Editor editor(spec);
    editor.Show();

    return qapp.exec();
}
