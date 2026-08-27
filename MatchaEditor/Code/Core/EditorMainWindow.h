#pragma once

#include <QMainWindow>

namespace Matcha
{
class QtViewportWidget;

class EditorMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditorMainWindow(QtViewportWidget* viewport, QWidget* parent = nullptr);
};
}  // namespace Matcha
