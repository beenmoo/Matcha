#pragma once

#include <QWidget>
#include <QFileSystemModel>
#include <QListView>
#include <QString>

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

private:
    QFileSystemModel* m_FileSystemModel;
    QListView* m_ListView;
};
}  // namespace MatchaEditor
