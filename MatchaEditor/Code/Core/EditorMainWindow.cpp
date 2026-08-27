#include "EditorMainWindow.h"
#include "Core/Qt/QtViewportWidget.h"

#include <QDockWidget>
#include <QMenuBar>
#include <QPainter>
#include <QPixmap>

namespace Matcha
{
namespace
{
void AddDock(QMainWindow* window, const QString& title, Qt::DockWidgetArea area)
{
    QDockWidget* dock = new QDockWidget(title, window);
    window->addDockWidget(area, dock);
}

// A plain Qt-drawn pixmap, not extracted from any native HICON - sidesteps a Qt/Windows bug
// where converting certain system icons to a QPixmap asserts on the AND-mask's bitmap format.
QIcon MakePlaceholderIcon()
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(0x6a, 0x9a, 0x4a));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(pixmap.rect().adjusted(2, 2, -2, -2));

    return QIcon(pixmap);
}
}  // namespace

EditorMainWindow::EditorMainWindow(QtViewportWidget* viewport, QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Hazelnut");
    setWindowIcon(MakePlaceholderIcon());
    resize(1600, 900);

    setCentralWidget(viewport);

    QMenuBar* menuBar = new QMenuBar(this);
    menuBar->addMenu("File");
    menuBar->addMenu("Edit");
    menuBar->addMenu("View");
    setMenuBar(menuBar);

    // Stubs for now - populated as the editor grows actual scene/inspector/logging UI.
    AddDock(this, "Scene Hierarchy", Qt::LeftDockWidgetArea);
    AddDock(this, "Inspector", Qt::RightDockWidgetArea);
    AddDock(this, "Console", Qt::BottomDockWidgetArea);
}
}  // namespace Matcha
