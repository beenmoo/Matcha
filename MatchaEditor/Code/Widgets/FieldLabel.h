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
// still available as a tooltip.
inline QLabel* CreateFieldLabel(const QString& text, QWidget* parent)
{
    constexpr int width = 60;

    QLabel* label = new QLabel(parent);
    label->setFixedWidth(width);
    label->setStyleSheet("color: #b0b0b0; font-size: 10px;");
    label->setToolTip(text);
    label->setText(QFontMetrics(label->font()).elidedText(text, Qt::ElideRight, width));

    return label;
}
}  // namespace MatchaEditor
