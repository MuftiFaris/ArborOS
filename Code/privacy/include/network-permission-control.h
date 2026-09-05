#ifndef NETWORK_PERMISSION_CONTROL_H
#define NETWORK_PERMISSION_CONTROL_H

#include <QString>
#include <QObject>
#include <QMap>
#include <QList>
#include <QMutex>
#include <QSet>
#include "privacy-manager.h"

/**
 * NetworkPermissionControl - Fine-grained network access management
 * 
 * Provides:
 * - Per-application network permission control
 * - DNS query logging (what domains accessed per app)
 * - IP address filtering (block/allow ranges)
 * - VPN integration and detection
 * - Per-app network usage tracking (bytes sent/received)
 * - Network interface monitoring
 */

class NetworkPermissionControl : public QObject
{
    Q_OBJECT

public:
    explicit NetworkPermissionControl(PrivacyManager* privacyManager, QObject* parent = nullptr);
    ~NetworkPermissionControl();

    // Permission management
    bool requestNetworkAccess(const QString& appId, const QString& appName, 
                             const QString& reason = "");
    void setAppNetworkPermission(const QString& appId, PrivacyManager::PermissionState state);
    PrivacyManager::PermissionState getAppNetworkPermission(const QString& appId);

    // Global network control
    void setGlobalNetworkEnabled(bool enabled);
    bool isGlobalNetworkEnabled() const;

    // DNS query logging
    struct DNSQuery {
        qint64 timestamp;
        QString appId;
        QString appName;
        QString domain;
        QString queryType;  // "A", "AAAA", "MX", "NS", "CNAME", etc.
        QString resolvedIPs;  // comma-separated
        QString status;  // "resolved", "failed", "blocked"
    };

    QList<DNSQuery> getDNSQueries(int daysBack = 7);
    QList<DNSQuery> getDNSQueriesByApp(const QString& appId, int daysBack = 7);
    QList<QString> getDomainsAccessedByApp(const QString& appId, int daysBack = 7);
    int getDNSQueryCount(const QString& appId, int daysBack = 7);

    // IP filtering
    struct IPRange {
        QString id;
        QString description;
        QString startIP;
        QString endIP;
        bool isBlacklist;  // true = deny, false = allow
    };

    void addIPFilter(const IPRange& range);
    void removeIPFilter(const QString& filterId);
    QList<IPRange> getIPFilters();
    bool isIPBlocked(const QString& ipAddress);
    bool isIPAllowed(const QString& ipAddress);

    // Network usage tracking
    struct NetworkUsageRecord {
        qint64 timestamp;
        QString appId;
        QString appName;
        qint64 bytesSent;
        qint64 bytesReceived;
        QString interface;  // "eth0", "wlan0", "vpn0", etc.
    };

    QList<NetworkUsageRecord> getNetworkUsage(int daysBack = 7);
    QList<NetworkUsageRecord> getNetworkUsageByApp(const QString& appId, int daysBack = 7);
    qint64 getTotalBytesSent(const QString& appId, int daysBack = 7);
    qint64 getTotalBytesReceived(const QString& appId, int daysBack = 7);
    qint64 getTotalNetworkUsage(const QString& appId, int daysBack = 7);

    // VPN management
    struct VPNConnection {
        QString id;
        QString name;
        QString protocol;  // "OpenVPN", "WireGuard", "IKEv2", etc.
        QString status;  // "connected", "disconnected", "connecting"
        QString localIP;
        QString remoteGateway;
        qint64 connectionTime;
    };

    QList<VPNConnection> getAvailableVPNs();
    bool isVPNConnected() const;
    QString getActiveVPNId() const;
    void setAppsRequireVPN(const QList<QString>& appIds);
    QList<QString> getAppsRequiringVPN();
    bool canAppAccessNetworkWithoutVPN(const QString& appId);

    // Network interface monitoring
    struct NetworkInterface {
        QString name;  // "eth0", "wlan0", etc.
        QString type;  // "ethernet", "wifi", "vpn", "loopback"
        bool isActive;
        QString ipAddress;
        QString macAddress;
        qint64 bytesIn;
        qint64 bytesOut;
    };

    QList<NetworkInterface> getNetworkInterfaces();
    QString getActiveInterface();

    // Audit trail
    struct NetworkAccessRecord {
        qint64 timestamp;
        QString appId;
        QString appName;
        QString action;  // "requested", "granted", "denied", "attempted", "blocked"
        QString details;
        bool allowed;
    };

    QList<NetworkAccessRecord> getNetworkAccessHistory(int daysBack = 7);

    // Statistics
    QMap<QString, int> getNetworkAccessStatistics(int daysBack = 7);
    QMap<QString, qint64> getNetworkUsageStatistics(int daysBack = 7);

public slots:
    void onAppPermissionChanged(const QString& appId, int categoryInt, int stateInt);
    void onGlobalSettingChanged(int settingType, bool enabled);
    void updateNetworkUsage(const QString& appId, qint64 bytesSent, qint64 bytesReceived);
    void logDNSQuery(const DNSQuery& query);

signals:
    // Emitted when network permission requested
    void networkPermissionRequested(const QString& appId, const QString& appName);

    // Emitted when network access is granted/denied
    void networkAccessDecision(const QString& appId, bool granted);

    // Emitted when app attempts network access without permission
    void networkAccessBlocked(const QString& appId, const QString& appName);

    // Emitted when global network setting changes
    void globalNetworkStateChanged(bool enabled);

    // Emitted when DNS query is logged
    void dnsQueryLogged(const DNSQuery& query);

    // Emitted when IP is blocked
    void ipBlocked(const QString& appId, const QString& ipAddress);

    // Emitted when network usage changes
    void networkUsageUpdated(const QString& appId, qint64 bytesSent, qint64 bytesReceived);

    // Emitted when VPN status changes
    void vpnStatusChanged(const QString& vpnId, bool connected);

private:
    PrivacyManager* m_privacyManager;
    QMutex m_mutex;

    // DNS query history
    QList<DNSQuery> m_dnsHistory;

    // IP filtering rules
    QMap<QString, IPRange> m_ipFilters;

    // Network usage tracking
    QList<NetworkUsageRecord> m_networkUsage;

    // VPN configuration
    QList<VPNConnection> m_vpnConnections;
    QString m_activeVPNId;
    QSet<QString> m_appsRequiringVPN;

    // Network interfaces
    QList<NetworkInterface> m_networkInterfaces;

    // Access history
    QList<NetworkAccessRecord> m_accessHistory;

    // Helper methods
    void initializeNetworkInterfaces();
    void logNetworkAccess(const NetworkAccessRecord& record);
    bool validateIPAddress(const QString& ip);
    bool isIPInRange(const QString& ip, const IPRange& range);
};

#endif // NETWORK_PERMISSION_CONTROL_H
