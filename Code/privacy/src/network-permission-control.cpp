#include "network-permission-control.h"
#include <QDateTime>
#include <QDebug>
#include <QMutexLocker>
#include <QHostAddress>

NetworkPermissionControl::NetworkPermissionControl(PrivacyManager* privacyManager, QObject* parent)
    : QObject(parent), m_privacyManager(privacyManager), m_activeVPNId("")
{
    if (!m_privacyManager) {
        qCritical() << "NetworkPermissionControl: PrivacyManager is null";
        return;
    }

    // Initialize network interfaces
    initializeNetworkInterfaces();

    // Connect to privacy manager signals
    connect(m_privacyManager, QOverload<const QString&, int, int>::of(&PrivacyManager::permissionDecided),
            this, &NetworkPermissionControl::onAppPermissionChanged);
    connect(m_privacyManager, QOverload<int, bool>::of(&PrivacyManager::globalSettingChanged),
            this, &NetworkPermissionControl::onGlobalSettingChanged);

    qDebug() << "NetworkPermissionControl initialized";
}

NetworkPermissionControl::~NetworkPermissionControl()
{
    // Cleanup
}

bool NetworkPermissionControl::requestNetworkAccess(const QString& appId, const QString& appName,
                                                   const QString& reason)
{
    QMutexLocker locker(&m_mutex);

    // Check global setting first
    if (!isGlobalNetworkEnabled()) {
        qDebug() << "Network access denied for" << appId << "- global network disabled";
        emit networkAccessDecision(appId, false);
        logNetworkAccess({QDateTime::currentMSecsSinceEpoch(), appId, appName,
                         "denied_global_disabled", "network globally disabled", false});
        return false;
    }

    // Check if app requires VPN
    if (m_appsRequiringVPN.contains(appId) && !isVPNConnected()) {
        qDebug() << "Network access denied for" << appId << "- VPN required but not connected";
        emit networkAccessDecision(appId, false);
        logNetworkAccess({QDateTime::currentMSecsSinceEpoch(), appId, appName,
                         "denied_vpn_required", "VPN required for this app", false});
        return false;
    }

    // Request from PrivacyManager
    bool granted = m_privacyManager->requestPermission(appId, PrivacyManager::Network,
                                                       reason.isEmpty() ? "network_access" : reason);

    if (granted) {
        logNetworkAccess({QDateTime::currentMSecsSinceEpoch(), appId, appName,
                         "granted", reason, true});
    } else {
        logNetworkAccess({QDateTime::currentMSecsSinceEpoch(), appId, appName,
                         "denied", reason, false});
        emit networkAccessBlocked(appId, appName);
    }

    emit networkAccessDecision(appId, granted);
    return granted;
}

void NetworkPermissionControl::setAppNetworkPermission(const QString& appId, 
                                                      PrivacyManager::PermissionState state)
{
    if (!m_privacyManager) {
        qWarning() << "PrivacyManager not available";
        return;
    }

    m_privacyManager->setPermission(appId, PrivacyManager::Network, state);
    qDebug() << "Network permission for" << appId << "set to" << (int)state;
}

PrivacyManager::PermissionState NetworkPermissionControl::getAppNetworkPermission(const QString& appId)
{
    if (!m_privacyManager) {
        return PrivacyManager::Denied;
    }

    return m_privacyManager->getPermission(appId, PrivacyManager::Network);
}

void NetworkPermissionControl::setGlobalNetworkEnabled(bool enabled)
{
    if (!m_privacyManager) {
        qWarning() << "PrivacyManager not available";
        return;
    }

    m_privacyManager->setGlobalNetworkEnabled(enabled);
    emit globalNetworkStateChanged(enabled);
    qDebug() << "Global network" << (enabled ? "enabled" : "disabled");
}

bool NetworkPermissionControl::isGlobalNetworkEnabled() const
{
    if (!m_privacyManager) {
        return false;
    }

    return m_privacyManager->isGlobalNetworkEnabled();
}

QList<NetworkPermissionControl::DNSQuery> NetworkPermissionControl::getDNSQueries(int daysBack)
{
    QMutexLocker locker(&m_mutex);

    QList<DNSQuery> filtered;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000LL);

    for (const auto& query : m_dnsHistory) {
        if (query.timestamp >= cutoffTime) {
            filtered.append(query);
        }
    }

    return filtered;
}

QList<NetworkPermissionControl::DNSQuery> NetworkPermissionControl::getDNSQueriesByApp(const QString& appId, int daysBack)
{
    QMutexLocker locker(&m_mutex);

    QList<DNSQuery> filtered;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000LL);

    for (const auto& query : m_dnsHistory) {
        if (query.timestamp >= cutoffTime && query.appId == appId) {
            filtered.append(query);
        }
    }

    return filtered;
}

QList<QString> NetworkPermissionControl::getDomainsAccessedByApp(const QString& appId, int daysBack)
{
    QMutexLocker locker(&m_mutex);

    QSet<QString> domains;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000LL);

    for (const auto& query : m_dnsHistory) {
        if (query.timestamp >= cutoffTime && query.appId == appId) {
            domains.insert(query.domain);
        }
    }

    return domains.toList();
}

int NetworkPermissionControl::getDNSQueryCount(const QString& appId, int daysBack)
{
    QMutexLocker locker(&m_mutex);

    int count = 0;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000LL);

    for (const auto& query : m_dnsHistory) {
        if (query.timestamp >= cutoffTime && query.appId == appId) {
            count++;
        }
    }

    return count;
}

void NetworkPermissionControl::addIPFilter(const IPRange& range)
{
    QMutexLocker locker(&m_mutex);

    if (range.id.isEmpty()) {
        qWarning() << "IPRange id cannot be empty";
        return;
    }

    m_ipFilters[range.id] = range;
    qDebug() << "IP filter added:" << range.id << "(" << range.startIP << "-" << range.endIP << ")";
}

void NetworkPermissionControl::removeIPFilter(const QString& filterId)
{
    QMutexLocker locker(&m_mutex);

    if (m_ipFilters.remove(filterId)) {
        qDebug() << "IP filter removed:" << filterId;
    } else {
        qWarning() << "IP filter not found:" << filterId;
    }
}

QList<NetworkPermissionControl::IPRange> NetworkPermissionControl::getIPFilters()
{
    QMutexLocker locker(&m_mutex);
    return m_ipFilters.values();
}

bool NetworkPermissionControl::isIPBlocked(const QString& ipAddress)
{
    QMutexLocker locker(&m_mutex);

    for (const auto& filter : m_ipFilters) {
        if (filter.isBlacklist && isIPInRange(ipAddress, filter)) {
            return true;
        }
    }

    return false;
}

bool NetworkPermissionControl::isIPAllowed(const QString& ipAddress)
{
    QMutexLocker locker(&m_mutex);

    // If no whitelist filters, allow by default
    bool hasWhitelist = false;
    for (const auto& filter : m_ipFilters) {
        if (!filter.isBlacklist) {
            hasWhitelist = true;
            break;
        }
    }

    if (!hasWhitelist) {
        return true;
    }

    // Check against whitelist
    for (const auto& filter : m_ipFilters) {
        if (!filter.isBlacklist && isIPInRange(ipAddress, filter)) {
            return true;
        }
    }

    return false;
}

QList<NetworkPermissionControl::NetworkUsageRecord> NetworkPermissionControl::getNetworkUsage(int daysBack)
{
    QMutexLocker locker(&m_mutex);

    QList<NetworkUsageRecord> filtered;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000LL);

    for (const auto& record : m_networkUsage) {
        if (record.timestamp >= cutoffTime) {
            filtered.append(record);
        }
    }

    return filtered;
}

QList<NetworkPermissionControl::NetworkUsageRecord> NetworkPermissionControl::getNetworkUsageByApp(const QString& appId, int daysBack)
{
    QMutexLocker locker(&m_mutex);

    QList<NetworkUsageRecord> filtered;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000LL);

    for (const auto& record : m_networkUsage) {
        if (record.timestamp >= cutoffTime && record.appId == appId) {
            filtered.append(record);
        }
    }

    return filtered;
}

qint64 NetworkPermissionControl::getTotalBytesSent(const QString& appId, int daysBack)
{
    QMutexLocker locker(&m_mutex);

    qint64 total = 0;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000LL);

    for (const auto& record : m_networkUsage) {
        if (record.timestamp >= cutoffTime && record.appId == appId) {
            total += record.bytesSent;
        }
    }

    return total;
}

qint64 NetworkPermissionControl::getTotalBytesReceived(const QString& appId, int daysBack)
{
    QMutexLocker locker(&m_mutex);

    qint64 total = 0;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000LL);

    for (const auto& record : m_networkUsage) {
        if (record.timestamp >= cutoffTime && record.appId == appId) {
            total += record.bytesReceived;
        }
    }

    return total;
}

qint64 NetworkPermissionControl::getTotalNetworkUsage(const QString& appId, int daysBack)
{
    return getTotalBytesSent(appId, daysBack) + getTotalBytesReceived(appId, daysBack);
}

QList<NetworkPermissionControl::VPNConnection> NetworkPermissionControl::getAvailableVPNs()
{
    QMutexLocker locker(&m_mutex);
    return m_vpnConnections;
}

bool NetworkPermissionControl::isVPNConnected() const
{
    QMutexLocker locker(&m_mutex);
    return !m_activeVPNId.isEmpty();
}

QString NetworkPermissionControl::getActiveVPNId() const
{
    QMutexLocker locker(&m_mutex);
    return m_activeVPNId;
}

void NetworkPermissionControl::setAppsRequireVPN(const QList<QString>& appIds)
{
    QMutexLocker locker(&m_mutex);

    m_appsRequiringVPN.clear();
    for (const QString& appId : appIds) {
        m_appsRequiringVPN.insert(appId);
    }

    qDebug() << "Apps requiring VPN updated:" << m_appsRequiringVPN.count();
}

QList<QString> NetworkPermissionControl::getAppsRequiringVPN()
{
    QMutexLocker locker(&m_mutex);
    return m_appsRequiringVPN.toList();
}

bool NetworkPermissionControl::canAppAccessNetworkWithoutVPN(const QString& appId)
{
    QMutexLocker locker(&m_mutex);
    return !m_appsRequiringVPN.contains(appId);
}

QList<NetworkPermissionControl::NetworkInterface> NetworkPermissionControl::getNetworkInterfaces()
{
    QMutexLocker locker(&m_mutex);
    return m_networkInterfaces;
}

QString NetworkPermissionControl::getActiveInterface()
{
    QMutexLocker locker(&m_mutex);

    for (const auto& iface : m_networkInterfaces) {
        if (iface.isActive && iface.type != "loopback") {
            return iface.name;
        }
    }

    return "";
}

QList<NetworkPermissionControl::NetworkAccessRecord> NetworkPermissionControl::getNetworkAccessHistory(int daysBack)
{
    QMutexLocker locker(&m_mutex);

    QList<NetworkAccessRecord> filtered;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000LL);

    for (const auto& record : m_accessHistory) {
        if (record.timestamp >= cutoffTime) {
            filtered.append(record);
        }
    }

    return filtered;
}

QMap<QString, int> NetworkPermissionControl::getNetworkAccessStatistics(int daysBack)
{
    QMutexLocker locker(&m_mutex);

    QMap<QString, int> stats;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000LL);

    for (const auto& record : m_accessHistory) {
        if (record.timestamp >= cutoffTime) {
            stats[record.appId]++;
        }
    }

    return stats;
}

QMap<QString, qint64> NetworkPermissionControl::getNetworkUsageStatistics(int daysBack)
{
    QMutexLocker locker(&m_mutex);

    QMap<QString, qint64> stats;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000LL);

    for (const auto& record : m_networkUsage) {
        if (record.timestamp >= cutoffTime) {
            stats[record.appId] += (record.bytesSent + record.bytesReceived);
        }
    }

    return stats;
}

void NetworkPermissionControl::onAppPermissionChanged(const QString& appId, int categoryInt, int stateInt)
{
    if (categoryInt != PrivacyManager::Network) {
        return;  // Not a network permission change
    }

    qDebug() << "Network permission changed for" << appId << "- state:" << stateInt;
}

void NetworkPermissionControl::onGlobalSettingChanged(int settingType, bool enabled)
{
    if (settingType != PrivacyManager::Network) {
        return;  // Not a network setting change
    }

    emit globalNetworkStateChanged(enabled);
    qDebug() << "Global network setting changed:" << enabled;
}

void NetworkPermissionControl::updateNetworkUsage(const QString& appId, qint64 bytesSent, qint64 bytesReceived)
{
    QMutexLocker locker(&m_mutex);

    // Keep last 10000 records
    if (m_networkUsage.count() >= 10000) {
        m_networkUsage.removeFirst();
    }

    m_networkUsage.append({QDateTime::currentMSecsSinceEpoch(), appId, "", bytesSent, bytesReceived, getActiveInterface()});

    emit networkUsageUpdated(appId, bytesSent, bytesReceived);
}

void NetworkPermissionControl::logDNSQuery(const DNSQuery& query)
{
    QMutexLocker locker(&m_mutex);

    // Keep last 5000 DNS records
    if (m_dnsHistory.count() >= 5000) {
        m_dnsHistory.removeFirst();
    }

    m_dnsHistory.append(query);

    // Also log to PrivacyManager audit trail
    if (m_privacyManager) {
        m_privacyManager->logAccess(query.appId, PrivacyManager::Network,
                                   QString("dns: %1 query for %2 (%3)")
                                   .arg(query.queryType)
                                   .arg(query.domain)
                                   .arg(query.status));
    }

    emit dnsQueryLogged(query);
}

void NetworkPermissionControl::initializeNetworkInterfaces()
{
    QMutexLocker locker(&m_mutex);

    // Initialize with common network interfaces
    m_networkInterfaces.append({
        "lo", "loopback", true, "127.0.0.1", "00:00:00:00:00:00", 0, 0
    });

    m_networkInterfaces.append({
        "eth0", "ethernet", false, "", "", 0, 0
    });

    m_networkInterfaces.append({
        "wlan0", "wifi", false, "", "", 0, 0
    });

    qDebug() << "Network interfaces initialized:" << m_networkInterfaces.count();
}

void NetworkPermissionControl::logNetworkAccess(const NetworkAccessRecord& record)
{
    QMutexLocker locker(&m_mutex);

    // Keep last 2000 records
    if (m_accessHistory.count() >= 2000) {
        m_accessHistory.removeFirst();
    }

    m_accessHistory.append(record);

    // Also log to PrivacyManager audit trail
    if (m_privacyManager) {
        m_privacyManager->logAccess(record.appId, PrivacyManager::Network,
                                   QString("network: %1 (%2)")
                                   .arg(record.action)
                                   .arg(record.details));
    }
}

bool NetworkPermissionControl::validateIPAddress(const QString& ip)
{
    QHostAddress addr(ip);
    return !addr.isNull();
}

bool NetworkPermissionControl::isIPInRange(const QString& ip, const IPRange& range)
{
    QHostAddress ipAddr(ip);
    QHostAddress startAddr(range.startIP);
    QHostAddress endAddr(range.endIP);

    if (ipAddr.isNull() || startAddr.isNull() || endAddr.isNull()) {
        return false;
    }

    // Simple IP range comparison (works for IPv4)
    return (ipAddr.toIPv4Address() >= startAddr.toIPv4Address()) &&
           (ipAddr.toIPv4Address() <= endAddr.toIPv4Address());
}
