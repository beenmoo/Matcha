#pragma once

#include <QFontMetrics>
#include <QLabel>
#include <QSizePolicy>
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

// A field's value control (spin box, line edit, combo box, ...) otherwise imposes its own
// minimumSizeHint as a hard floor - once the Inspector panel narrows past that, Qt has nowhere
// left to shrink the row and instead clips the control or grows a horizontal scrollbar. Ignored
// tells the layout it may compress the control past that floor, down to the explicit minimumWidth
// given here instead, so narrowing the panel squishes fields rather than cutting them off.
inline void ApplySquishPolicy(QWidget* control, int minimumWidth = 20)
{
    QSizePolicy policy = control->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    control->setSizePolicy(policy);
    control->setMinimumWidth(minimumWidth);
}
}  // namespace MatchaEditor
