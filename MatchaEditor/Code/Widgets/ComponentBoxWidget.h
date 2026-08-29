#pragma once

#include <QWidget>

class QVBoxLayout;

namespace MatchaEditor
{
class ComponentBoxWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ComponentBoxWidget(const QString& title, QWidget* parent = nullptr);
    void SetContent(QWidget* contentWidget);

private:
    QVBoxLayout* m_ContentLayout;
};
}  // namespace MatchaEditor