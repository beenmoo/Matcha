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
}  // namespace MatchaEditor
