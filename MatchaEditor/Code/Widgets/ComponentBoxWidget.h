#pragma once

#include <QWidget>

class QVBoxLayout;
class QPushButton;

namespace MatchaEditor
{
class ComponentBoxWidget : public QWidget
{
    Q_OBJECT
public:
    // removable adds a small "x" button to the header, next to the collapse arrow - for
    // components an entity isn't guaranteed to always have (TagComponent/TransformComponent,
    // which every entity gets from Scene::CreateEntity(), pass false; anything addable via "+ Add
    // Component" passes true - see InspectorPanel::RegisterComponentInspectors).
    explicit ComponentBoxWidget(const QString& title, bool isCollapsed, bool removable, QWidget* parent = nullptr);
    void SetContent(QWidget* contentWidget);

signals:
    void CollapseStateChanged(bool isCollapsed);
    void RemoveRequested();

private:
    void ToggleCollapse();
    void SetCollapsedState(bool state);

private:
    QVBoxLayout* m_ContentLayout;
    QWidget* m_ContentContainer;
    QPushButton* m_HeaderButton;
    bool m_IsCollapsed = false;
};
}  // namespace MatchaEditor