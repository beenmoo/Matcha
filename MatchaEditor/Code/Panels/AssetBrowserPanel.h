#pragma once

#include <Matcha.h>
#include <DockWidget.h>

namespace ads
{
class CDockManager;
}

namespace MatchaEditor
{
class AssetBrowserWidget;

class AssetBrowserPanel : public ads::CDockWidget
{
    Q_OBJECT
public:
    explicit AssetBrowserPanel(ads::CDockManager* dockManager, Application& application, QWidget* parent = nullptr);

    AssetBrowserWidget* GetBrowserWidget() const
    {
        return m_BrowserWidget;
    }

private:
    AssetBrowserWidget* m_BrowserWidget;
};
}  // namespace MatchaEditor