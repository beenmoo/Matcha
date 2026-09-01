#include "Theme.h"
#include "Core/Logger.h"

#include <QApplication>
#include <QColor>
#include <QFile>
#include <QPalette>
#include <QTextStream>

namespace MatchaEditor
{
namespace
{
// Assets/ (this project's convention for loose runtime files - shaders load the same way, see
// Sandbox.cpp/SceneHierarchyWidget.cpp) is copied next to the exe and resolved relative to the
// process's working directory, not applicationDirPath() - matches how every other asset in this
// codebase is already loaded, so running the exe from a different CWD breaks the same way it
// already would for shaders, rather than in some new QSS-specific way.
QString LoadStyleSheet()
{
    QFile file("Assets/Styles/Editor.qss");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        MT_CORE_WARN("Failed to open Assets/Styles/Editor.qss - editor widgets will fall back to unstyled Fusion defaults.");
        return QString();
    }

    return QTextStream(&file).readAll();
}
}  // namespace

void ApplyDarkTheme(QApplication& app)
{
    QPalette palette;

    const QColor window(60, 60, 60);
    const QColor base(35, 35, 35);
    const QColor alternateBase(48, 48, 48);
    const QColor button(69, 69, 69);
    const QColor text(224, 224, 224);
    const QColor disabledText(110, 110, 110);
    const QColor highlight(58, 114, 176);

    palette.setColor(QPalette::Window, window);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, base);
    palette.setColor(QPalette::AlternateBase, alternateBase);
    palette.setColor(QPalette::ToolTipBase, window);
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::PlaceholderText, disabledText);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, button);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Light, button.lighter(150));
    palette.setColor(QPalette::Midlight, button.lighter(120));
    palette.setColor(QPalette::Mid, button.darker(120));
    palette.setColor(QPalette::Dark, button.darker(150));
    palette.setColor(QPalette::Shadow, QColor(20, 20, 20));
    palette.setColor(QPalette::Link, QColor(90, 155, 212));
    palette.setColor(QPalette::LinkVisited, QColor(120, 120, 200));
    palette.setColor(QPalette::Highlight, highlight);
    palette.setColor(QPalette::HighlightedText, Qt::white);

    // Base stays the same as Window when disabled (rather than a separate gray) so a disabled
    // line edit/combo box reads as "flattened into the panel" instead of just dimmer text on an
    // otherwise still-active-looking field.
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::Text, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::Base, window);
    palette.setColor(QPalette::Disabled, QPalette::Highlight, button);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledText);

    app.setPalette(palette);

    // Everything a QPalette can't express (per-instance axis colors, section-label boldness,
    // the one widget that needs left-aligned text, ...) lives in Editor.qss instead of scattered
    // setStyleSheet() calls in each widget's own .cpp - see that file's own header comment.
    app.setStyleSheet(LoadStyleSheet());
}
}  // namespace MatchaEditor
