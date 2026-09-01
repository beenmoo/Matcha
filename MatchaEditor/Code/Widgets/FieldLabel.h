#pragma once

#include <QFontMetrics>
#include <QLabel>
#include <QString>
#include <QWidget>

namespace MatchaEditor
{
// Every field row (FloatFieldWidget, Vec3ControlWidget, ...) starts with a label of this same
// fixed width, so labels line up across rows regardless of which field type follows. A label
// longer than that width is elided with "..." rather than raw-clipped, and the full text is
// still available as a tooltip. No font override - left at the default application font so it
// matches the Scene Hierarchy panel's text (which doesn't set one either) exactly.
inline QLabel* CreateFieldLabel(const QString& text, QWidget* parent)
{
    constexpr int width = 65;

    QLabel* label = new QLabel(parent);
    label->setFixedWidth(width);
    label->setToolTip(text);
    label->setText(QFontMetrics(label->font()).elidedText(text, Qt::ElideRight, width));

    return label;
}
}  // namespace MatchaEditor
