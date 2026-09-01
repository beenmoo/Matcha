#include "ComponentBoxWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>

namespace MatchaEditor
{
ComponentBoxWidget::ComponentBoxWidget(const QString& title, bool isCollapsed, QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 2, 0, 2);
    mainLayout->setSpacing(0);

    // 1. Create the header as a clickable button so it handles clicks natively
    m_HeaderButton = new QPushButton(QString("  ▼  %1").arg(title), this);
    m_HeaderButton->setCursor(Qt::PointingHandCursor);
    // Left-aligned text (Fusion centers QPushButton text by default) - see Editor.qss's
    // QPushButton#ComponentHeaderButton rule.
    m_HeaderButton->setObjectName("ComponentHeaderButton");
    mainLayout->addWidget(m_HeaderButton);

    // 2. Content container widget that holds your properties.
    m_ContentContainer = new QWidget(this);

    m_ContentLayout = new QVBoxLayout(m_ContentContainer);
    m_ContentLayout->setContentsMargins(6, 4, 6, 4);
    m_ContentLayout->setSpacing(4);

    mainLayout->addWidget(m_ContentContainer);
    setLayout(mainLayout);

    SetCollapsedState(isCollapsed);

    // 3. Hook up the click event to toggle collapse state
    connect(m_HeaderButton, &QPushButton::clicked, this, &ComponentBoxWidget::ToggleCollapse);
}

void ComponentBoxWidget::SetContent(QWidget* contentWidget)
{
    m_ContentLayout->addWidget(contentWidget);
}

void ComponentBoxWidget::ToggleCollapse()
{
    m_IsCollapsed = !m_IsCollapsed;

    SetCollapsedState(m_IsCollapsed);
}

void ComponentBoxWidget::SetCollapsedState(bool state)
{
    m_IsCollapsed = state;

    // Hide or show the inner content container instantly!
    m_ContentContainer->setVisible(!m_IsCollapsed);

    // Swap the arrow icon between down (expanded) and right (collapsed)
    QString currentTitle = m_HeaderButton->text();
    // Strip out existing arrow character safely

    if (m_IsCollapsed)
        currentTitle.replace("▼", "▶");
    else
        currentTitle.replace("▶", "▼");
    m_HeaderButton->setText(currentTitle);

    emit CollapseStateChanged(m_IsCollapsed);
}
}  // namespace MatchaEditor