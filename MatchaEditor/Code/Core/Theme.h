#pragma once

class QApplication;

namespace MatchaEditor
{
// Fusion honors QPalette for every widget it draws, so a dark theme here is just a dark
// QPalette - no external stylesheet library, no QSS file to keep in sync with Qt's own widget
// set. Deliberately neutral gray (not blue-purple, unlike Qt's commonly-copied "dark fusion"
// snippet) to match the Unity-editor look this project has been converging on. Separate from
// Qt-Advanced-Docking-System's own dark theme (see EditorMainWindow.cpp), which only styles
// dock chrome (title bars, tab bars, close/float buttons) - every widget inside a panel falls
// back to whatever the application palette says, which is what this sets.
void ApplyDarkTheme(QApplication& app);

// Registers the bundled Inter variable font (Resources/Resources.qrc, compiled into the binary -
// see that file's own comment for why this isn't a loose Assets/ file like Editor.qss) and makes
// it the application default. Call this before constructing anything that could build a dock
// widget/tab (Editor/EditorMainWindow) - ADS computes each tab's own layout from QFontMetrics of
// the application's font at the tab's construction time, not at paint time, so setting the font
// any later would leave tab spacing computed against whatever font was active before this ran.
void ApplyEditorFont(QApplication& app);
}  // namespace MatchaEditor
