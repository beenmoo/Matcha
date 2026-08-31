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
    explicit ComponentBoxWidget(const QString& title, bool isCollapsed, QWidget* parent = nullptr);
    void SetContent(QWidget* contentWidget);

signals:
    void CollapseStateChanged(bool isCollapsed);

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