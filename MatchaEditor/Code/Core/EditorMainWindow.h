#pragma once

#include <QMainWindow>

#include <memory>

namespace Matcha
{
class QtViewportWidget;
class EngineContext;
}  // namespace Matcha

namespace ads
{
class CDockManager;
}  // namespace ads

namespace MatchaEditor
{
class ConsoleSink;

class EditorMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditorMainWindow(Matcha::EngineContext& context, Matcha::QtViewportWidget* viewport, QWidget* parent = nullptr);
    ~EditorMainWindow() override;

private:
    ads::CDockManager* m_DockManager;
    std::shared_ptr<ConsoleSink> m_ConsoleSink;
};
}  // namespace Matcha
