#pragma once

#include <QDockWidget>

namespace MatchaEditor
{
class InspectorPanel : public QDockWidget
{
    Q_OBJECT
public:
    explicit InspectorPanel(QWidget* parent = nullptr);
    ~InspectorPanel() = default;

private:
};
}  // namespace MatchaEditor