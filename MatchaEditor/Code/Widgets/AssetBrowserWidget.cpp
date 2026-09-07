#include "AssetBrowserWidget.h"

#include <QVBoxLayout>
#include <QDir>

namespace MatchaEditor
{
AssetBrowserWidget::AssetBrowserWidget(const QString& projectAssetsPath, QWidget* parent)
    : QWidget(parent)
{
    m_FileSystemModel = new QFileSystemModel(this);
    m_FileSystemModel->setRootPath(projectAssetsPath);
    m_FileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);

    m_ListView = new QListView(this);
    m_ListView->setModel(m_FileSystemModel);
    m_ListView->setRootIndex(m_FileSystemModel->index(projectAssetsPath));
    m_ListView->setViewMode(QListView::IconMode);
    m_ListView->setIconSize(QSize(64, 64));
    m_ListView->setGridSize(QSize(80, 100));
    m_ListView->setResizeMode(QListView::Adjust);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_ListView);

    // Wire up the double-click event internally
    connect(m_ListView, &QListView::doubleClicked, this, &AssetBrowserWidget::OnDirectoryNavigated);
}

void AssetBrowserWidget::OnDirectoryNavigated(const QModelIndex& index)
{
    QString assetPath = m_FileSystemModel->filePath(index);
    if (m_FileSystemModel->isDir(index))
    {
        // If it's a directory, navigate into it
        m_ListView->setRootIndex(index);
    }
    else
    {
        // If it's a file, emit the double-clicked signal
        emit AssetDoubleClicked(assetPath);
    }
}
}  // namespace MatchaEditor