#pragma once

#include <QWidget>
#include <QFileSystemModel>
#include <QListView>
#include <QString>
#include <QStringList>
#include <QPushButton>

class QCheckBox;

namespace MatchaEditor
{
class AssetBrowserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AssetBrowserWidget(const QString& projectAssetsPath, QWidget* parent = nullptr);

signals:
    void AssetDoubleClicked(const QString& assetPath);

private slots:
    void OnDirectoryNavigated(const QModelIndex& index);
    void OnBackButtonClicked();
    void OnForwardButtonClicked();

private:
    void NavigateToDirectory(const QString& directoryPath, bool addToHistory = true);
    void UpdateNavigationButtons();

private:
    QFileSystemModel* m_FileSystemModel;
    QListView* m_ListView;

    QStringList m_History;
    // -1 means "no history yet" - NavigateToDirectory's own addToHistory branch checks for this
    // sentinel explicitly, so it has to start here rather than at 0 (which would look identical to
    // "already showing history entry 0").
    int m_HistoryIndex = -1;

    QPushButton* m_BackButton;
    QPushButton* m_ForwardButton;
};
}  // namespace MatchaEditor
