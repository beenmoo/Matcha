#include "ComponentBoxWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>

namespace MatchaEditor
{
ComponentBoxWidget::ComponentBoxWidget(const QString& title, QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    // Header frame styling
    QFrame* headerFrame = new QFrame(this);
    headerFrame->setStyleSheet("background-color: #2b2b2b; border-radius: 4px; padding: 4px;");
    QVBoxLayout* headerLayout = new QVBoxLayout(headerFrame);

    QLabel* titleLabel = new QLabel("<b>" + title + "</b>", headerFrame);
    headerLayout->addWidget(titleLabel);
    mainLayout->addWidget(headerFrame);

    // Content area container
    QWidget* contentContainer = new QWidget(this);
    m_ContentLayout = new QVBoxLayout(contentContainer);
    contentContainer->setLayout(m_ContentLayout);

    mainLayout->addWidget(contentContainer);
    setLayout(mainLayout);
}

void ComponentBoxWidget::SetContent(QWidget* contentWidget)
{
    m_ContentLayout->addWidget(contentWidget);
}
}  // namespace MatchaEditor