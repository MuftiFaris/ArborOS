#include "network-control.h"
#include "network-permission-control.h"

NetworkControl::NetworkControl(PrivacyManager* pm)
    : m_privacyManager(pm), m_vpnEnabled(false)
{
}

bool NetworkControl::requestNetworkAccess(const QString& appId)
{
    if (!m_privacyManager) return false;
    return m_privacyManager->requestPermission(appId, PrivacyManager::Network, "Network access requested");
}

void NetworkControl::setNetworkPermission(const QString& appId, PrivacyManager::PermissionState state)
{
    if (!m_privacyManager) return;
    m_privacyManager->setPermission(appId, PrivacyManager::Network, state);
}

void NetworkControl::logDNSQuery(const QString& appId, const QString& domain)
{
    if (!m_privacyManager) return;
    m_privacyManager->logAccess(appId, PrivacyManager::Network, QString("DNS: %1").arg(domain));
}

QStringList NetworkControl::getDNSQueries(const QString& appId, int minutesBack)
{
    if (!m_privacyManager) return QStringList();

    auto audit = m_privacyManager->getAuditTrailForApp(appId, 1);
    QStringList queries;
    for (const auto& rec : audit) {
        if (rec.category == PrivacyManager::Network && rec.details.startsWith("DNS: ")) {
            queries.append(rec.details.mid(5));
        }
    }
    return queries;
}

void NetworkControl::enableVPN(bool enable)
{
    m_vpnEnabled = enable;
}

bool NetworkControl::isVPNEnabled() const
{
    return m_vpnEnabled;
}
