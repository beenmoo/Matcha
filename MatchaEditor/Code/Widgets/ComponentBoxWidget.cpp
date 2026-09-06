#include "ComponentBoxWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>

namespace MatchaEditor
{
ComponentBoxWidget::ComponentBoxWidget(const QString& title, bool isCollapsed, bool removable, QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 2, 0, 2);
    mainLayout->setSpacing(0);

    // Header row: the collapse-toggle button (stretched to fill) plus, if removable, a small "x"
    // button beside it - previously the header button alone filled this whole row, so it now
    // lives inside its own horizontal layout instead of being added to mainLayout directly.
    QWidget* headerRow = new QWidget(this);
    QHBoxLayout* headerLayout = new QHBoxLayout(headerRow);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(0);

    // 1. Create the header as a clickable button so it handles clicks natively
    m_HeaderButton = new QPushButton(QString("  ▼  %1").arg(title), headerRow);
    m_HeaderButton->setCursor(Qt::PointingHandCursor);
    // Left-aligned text (Fusion centers QPushButton text by default) - see Editor.qss's
    // QPushButton#ComponentHeaderButton rule.
    m_HeaderButton->setObjectName("ComponentHeaderButton");
    headerLayout->addWidget(m_HeaderButton, 1);

    if (removable)
    {
        QPushButton* removeButton = new QPushButton("x", headerRow);
        removeButton->setCursor(Qt::PointingHandCursor);
        removeButton->setObjectName("ComponentRemoveButton");
        removeButton->setToolTip("Remove component");
        removeButton->setFixedWidth(24);
        connect(removeButton, &QPushButton::clicked, this, &ComponentBoxWidget::RemoveRequested);
        headerLayout->addWidget(removeButton);
    }

    mainLayout->addWidget(headerRow);

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