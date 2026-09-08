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

    m_BackButton = new QPushButton("<", this);
    m_ForwardButton = new QPushButton(">", this);

    QHBoxLayout* navLayout = new QHBoxLayout();
    navLayout->addWidget(m_BackButton);
    navLayout->addWidget(m_ForwardButton);
    navLayout->addStretch();  // Push buttons to the left

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(navLayout);
    layout->addWidget(m_ListView);

    // Wire up the double-click event internally
    connect(m_ListView, &QListView::doubleClicked, this, &AssetBrowserWidget::OnDirectoryNavigated);
    connect(m_BackButton, &QPushButton::clicked, this, &AssetBrowserWidget::OnBackButtonClicked);
    connect(m_ForwardButton, &QPushButton::clicked, this, &AssetBrowserWidget::OnForwardButtonClicked);

    // Initialize history
    NavigateToDirectory(projectAssetsPath);
}

void AssetBrowserWidget::OnDirectoryNavigated(const QModelIndex& index)
{
    QString assetPath = m_FileSystemModel->filePath(index);
    if (m_FileSystemModel->isDir(index))
    {
        // If it's a directory, navigate into it
        NavigateToDirectory(assetPath);
    }
    else
    {
        // If it's a file, emit the double-clicked signal
        emit AssetDoubleClicked(assetPath);
    }
}

void AssetBrowserWidget::OnBackButtonClicked()
{
    if (m_HistoryIndex > 0)
    {
        --m_HistoryIndex;
        NavigateToDirectory(m_History[m_HistoryIndex], false);
    }
}

void AssetBrowserWidget::OnForwardButtonClicked()
{
    if (m_HistoryIndex < m_History.size() - 1)
    {
        ++m_HistoryIndex;
        NavigateToDirectory(m_History[m_HistoryIndex], false);
    }
}

void AssetBrowserWidget::NavigateToDirectory(const QString& directoryPath, bool addToHistory)
{
    m_ListView->setRootIndex(m_FileSystemModel->index(directoryPath));

    if (addToHistory)
    {
        // If we're not at the end of the history, truncate it
        if (m_HistoryIndex < m_History.size() - 1)
        {
            m_History = m_History.mid(0, m_HistoryIndex + 1);
        }

        m_History.append(directoryPath);
        ++m_HistoryIndex;
    }

    UpdateNavigationButtons();
}

void AssetBrowserWidget::UpdateNavigationButtons()
{
    m_BackButton->setEnabled(m_HistoryIndex > 0);
    m_ForwardButton->setEnabled(m_HistoryIndex < m_History.size() - 1);
}
}  // namespace MatchaEditor