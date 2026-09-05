#ifndef PRIVACY_MANAGER_H
#define PRIVACY_MANAGER_H

#include <QString>
#include <QObject>
#include <QMap>
#include <QList>
#include <QDateTime>
#include <QSettings>
#include <QMutex>
#include <sqlite3.h>

/**
 * PrivacyManager - Centralized privacy control system for ArborOS
 *
 * Manages all permission requests, policies, and audit logging.
 * Singleton pattern - access via PrivacyManager::instance()
 *
 * Permission Flow:
 * 1. App requests permission via requestPermission()
 * 2. PrivacyManager checks stored policy
 * 3. If needed, shows permission dialog (triggers signal)
 * 4. User decision stored in policy
 * 5. Access logged to audit trail
 * 6. Result returned to app
 */

class PrivacyManager : public QObject
{
    Q_OBJECT

public:
    // Permission categories
    enum PermissionCategory {
        Microphone = 0,
        Camera = 1,
        Files = 2,
        Network = 3,
        Location = 4,
        Clipboard = 5,
        Contacts = 6,
        Calendar = 7,
        Photos = 8,
        Bluetooth = 9,
        USB = 10,
        Printing = 11,
        Screenshots = 12,
        SystemMonitor = 13,
        Audio = 14  // system audio access
    };
    Q_ENUM(PermissionCategory)

    // Permission states
    enum PermissionState {
        Denied = 0,           // Permanently denied
        AllowedAlways = 1,    // Always allow
        AskEveryTime = 2,     // Ask on each request
        AllowedOnce = 3,      // Allow once, then ask again
        SystemDenied = 4      // Denied by system policy
    };
    Q_ENUM(PermissionState)

    // Audit action types
    enum AuditAction {
        AuditRequested = 0,
        AuditGranted = 1,
        AuditDenied = 2,
        AuditUsed = 3,
        AuditChanged = 4
    };
    Q_ENUM(AuditAction)

    // Permission record for audit trail
    struct PermissionRecord {
        qint64 timestamp;
        QString appId;
        PermissionCategory category;
        AuditAction action;
        PermissionState state;
        QString details;
        QString userDecision;  // what user selected in dialog

        QString categoryName() const;
        QString actionName() const;
        QString stateName() const;
        QString formattedTime() const;
        bool isValid() const;
    };

    // App metadata
    struct AppMetadata {
        QString id;
        QString name;
        QString execPath;
        QString icon;
        QList<PermissionCategory> requestedPermissions;
        qint64 lastAccessTime;
        int accessCount;
    };

    // Singleton access
    static PrivacyManager* instance();

    // Permission management
    bool requestPermission(const QString& appId, PermissionCategory category,
                          const QString& details = "");
    void setPermission(const QString& appId, PermissionCategory category,
                      PermissionState state);
    PermissionState getPermission(const QString& appId, PermissionCategory category);

    // Bulk permission operations
    void denyAllPermissionsForApp(const QString& appId);
    void allowAllPermissionsForApp(const QString& appId);
    void resetPermissionsForApp(const QString& appId);

    // Audit trail
    QList<PermissionRecord> getAuditTrail(int daysBack = 30);
    QList<PermissionRecord> getAuditTrailForApp(const QString& appId, int daysBack = 30);
    QList<PermissionRecord> getAuditTrailForCategory(PermissionCategory category, int daysBack = 30);
    void clearAuditTrail(int daysBack = 0);  // 0 = clear all

    // Privacy scoring
    int calculatePrivacyScore();  // 0-100
    QString getPrivacyRecommendations();

    // App management
    void registerApp(const AppMetadata& metadata);
    AppMetadata getAppMetadata(const QString& appId);
    QList<AppMetadata> getAllRegisteredApps();
    QList<QString> getAppsWithPermission(PermissionCategory category);

    // Real-time access tracking
    void logAccess(const QString& appId, PermissionCategory category, const QString& details = "");
    QList<PermissionRecord> getCurrentActivity();  // last 5 minutes
    bool isAppCurrentlyAccessing(const QString& appId, PermissionCategory category);

    // Global settings
    void setGlobalMicrophoneEnabled(bool enabled);
    bool isGlobalMicrophoneEnabled() const;
    void setGlobalCameraEnabled(bool enabled);
    bool isGlobalCameraEnabled() const;
    void setGlobalNetworkEnabled(bool enabled);
    bool isGlobalNetworkEnabled() const;

    // Policy export/import
    QString exportPolicy(const QString& appId);
    bool importPolicy(const QString& appId, const QString& policyJson);

    // D-Bus integration ready
    QString getDBusPath() const { return "/org/arboros/PrivacyManager"; }
    QString getDBusInterface() const { return "org.arboros.PrivacyManager"; }

public slots:
    void onPermissionDialogResponse(const QString& appId, PermissionCategory category,
                                   PermissionState response);
    void onSystemPolicyChange();
    void onAuditMaintenanceTimer();

signals:
    // Emitted when app requests permission
    void permissionRequested(const QString& appId, int categoryInt,
                            const QString& appName, const QString& details);

    // Emitted when permission is granted/denied
    void permissionDecided(const QString& appId, int categoryInt, int stateInt);

    // Emitted when app accesses permission
    void accessLogged(const QString& appId, int categoryInt, qint64 timestamp);

    // Emitted when global setting changes
    void globalSettingChanged(int settingType, bool enabled);

    // Emitted when privacy score changes
    void privacyScoreChanged(int newScore);

    // Emitted when audit trail reaches size limit (cleanup needed)
    void auditTrailNeedsMaintenance();

private:
    explicit PrivacyManager(QObject* parent = nullptr);
    ~PrivacyManager();
    PrivacyManager(const PrivacyManager&) = delete;
    PrivacyManager& operator=(const PrivacyManager&) = delete;

    // Singleton instance
    static PrivacyManager* s_instance;
    static class SingletonGuard {
    public:
        ~SingletonGuard();
    } s_guard;

    // Internal data
    QSettings* m_settings;
    sqlite3* m_auditDb;
    QMap<QString, AppMetadata> m_appRegistry;
    QMap<QString, QMap<int, PermissionState>> m_policies;
    QMutex m_dbMutex;  // Protects SQLite access
    QMutex m_policyMutex;  // Protects policy map
    QMutex m_settingsMutex;  // Protects QSettings access
    QMutex m_registryMutex;  // Protects app registry

    // Configuration paths
    QString m_configPath;
    QString m_auditDbPath;

    // Initialize audit database
    bool initializeAuditDatabase();
    bool ensureAuditTableExists();

    // Internal permission check
    PermissionState checkPermissionPolicy(const QString& appId, PermissionCategory category);

    // Audit logging
    void logAuditRecord(const PermissionRecord& record);
    QList<PermissionRecord> queryAuditTrail(const QString& whereClause = "", int limit = 1000);

    // Privacy calculation helpers
    int countUnnecessaryPermissions(const AppMetadata& app);
    int countDangerousAppPermissions(const QString& appId);

    // Helper methods
    static QString permissionCategoryToString(PermissionCategory category);
    static PermissionCategory stringToPermissionCategory(const QString& str);
};

#endif // PRIVACY_MANAGER_H
