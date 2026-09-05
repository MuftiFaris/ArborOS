#ifndef NETWORK_CONTROL_H
#define NETWORK_CONTROL_H

#include "privacy-manager.h"
#include <QString>
#include <QStringList>

class NetworkControl
{
public:
    NetworkControl(PrivacyManager* pm);
    
    bool requestNetworkAccess(const QString& appId);
    void setNetworkPermission(const QString& appId, PrivacyManager::PermissionState state);
    
    void logDNSQuery(const QString& appId, const QString& domain);
    QStringList getDNSQueries(const QString& appId, int minutesBack = 60);
    
    void enableVPN(bool enable);
    bool isVPNEnabled() const;
    
private:
    PrivacyManager* m_privacyManager;
    bool m_vpnEnabled;
};

#endif
