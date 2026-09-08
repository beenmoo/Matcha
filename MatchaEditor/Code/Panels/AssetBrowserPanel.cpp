#include "AssetBrowserPanel.h"
#include "Widgets/AssetBrowserWidget.h"

#include <Matcha.h>

#include <QSplitter>

namespace MatchaEditor
{
AssetBrowserPanel::AssetBrowserPanel(ads::CDockManager* dockManager, Application& application, QWidget* parent)
    : ads::CDockWidget(dockManager, "Asset Browser", parent)
{
    QString assetsPath = QString::fromStdString(application.GetAssetsPath().string());
    m_BrowserWidget = new AssetBrowserWidget(assetsPath, this);
    setWidget(m_BrowserWidget);
}
}  // namespace MatchaEditor