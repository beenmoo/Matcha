#pragma once

#include <QMainWindow>

#include <memory>

namespace Matcha
{
class QtViewportWidget;
class Scene;
}  // namespace Matcha

namespace MatchaEditor
{
class ConsoleSink;

class EditorMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditorMainWindow(Matcha::Scene& scene, Matcha::QtViewportWidget* viewport, QWidget* parent = nullptr);
    ~EditorMainWindow() override;

private:
    std::shared_ptr<ConsoleSink> m_ConsoleSink;
};
}  // namespace Matcha
