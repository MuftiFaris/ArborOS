#include "privacy-manager.h"
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QTimer>
#include <QCoreApplication>
#include <QProcess>
#include <QMutex>
#include <QMutexLocker>
#include <algorithm>

// No static member needed - uses static local variable pattern in instance()
    : QObject(parent), m_settings(nullptr), m_auditDb(nullptr)
{
    // Setup paths
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationDataLocation);
    m_configPath = dataPath + "/privacy";
    m_auditDbPath = m_configPath + "/audit.db";

    // Create directories
    QDir dir;
    if (!dir.exists(m_configPath)) {
        dir.mkpath(m_configPath);
    }

    // Initialize settings
    m_settings = new QSettings(m_configPath + "/privacy.conf", QSettings::IniFormat, this);

    // Initialize audit database
    if (!initializeAuditDatabase()) {
        qWarning() << "Failed to initialize audit database";
    }

    // Setup maintenance timer (cleanup old audit records every 6 hours)
    QTimer* maintenanceTimer = new QTimer(this);
    connect(maintenanceTimer, &QTimer::timeout, this, &PrivacyManager::onAuditMaintenanceTimer, 
            Qt::QueuedConnection);
    maintenanceTimer->start(6 * 60 * 60 * 1000);  // 6 hours

    // Load initial policies
    m_settings->beginGroup("Permissions");
    QStringList apps = m_settings->childGroups();
    for (const QString& appId : apps) {
        m_settings->beginGroup(appId);
        QStringList categories = m_settings->childKeys();
        for (const QString& category : categories) {
            int categoryInt = stringToPermissionCategory(category);
            int stateInt = m_settings->value(category, (int)AskEveryTime).toInt();
            m_policies[appId][categoryInt] = (PermissionState)stateInt;
        }
        m_settings->endGroup();
    }
    m_settings->endGroup();

    qDebug() << "PrivacyManager initialized" << m_configPath;
}

PrivacyManager::~PrivacyManager()
{
    // Sync settings before cleanup
    if (m_settings) {
        m_settings->sync();
    }

    QMutexLocker dbLocker(&m_dbMutex);
    if (m_auditDb) {
        sqlite3_close_v2(m_auditDb);  // Safer than sqlite3_close
    }
}

PrivacyManager* PrivacyManager::instance()
{
    static PrivacyManager s_instance_obj;
    return &s_instance_obj;
}

bool PrivacyManager::initializeAuditDatabase()
{
    int rc = sqlite3_open_v2(m_auditDbPath.toStdString().c_str(), &m_auditDb,
                            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | 
                            SQLITE_OPEN_FULLMUTEX,  // Thread-safe
                            nullptr);
    if (rc != SQLITE_OK) {
        qCritical() << "Cannot open audit database:" << sqlite3_errmsg(m_auditDb);
        return false;
    }

    // Enable foreign keys
    int fkrc = sqlite3_exec(m_auditDb, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    if (fkrc != SQLITE_OK) {
        qWarning() << "Failed to enable foreign keys:" << sqlite3_errmsg(m_auditDb);
    }

    if (!ensureAuditTableExists()) {
        return false;
    }

    qDebug() << "Audit database initialized:" << m_auditDbPath;
    return true;
}

bool PrivacyManager::ensureAuditTableExists()
{
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS permission_audits (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp INTEGER NOT NULL,
            app_id TEXT NOT NULL,
            permission_category INTEGER NOT NULL,
            action INTEGER NOT NULL,
            permission_state INTEGER,
            details TEXT CHECK(length(details) <= 1000),
            user_decision TEXT CHECK(length(user_decision) <= 100)
        );

        CREATE INDEX IF NOT EXISTS idx_audit_timestamp ON permission_audits(timestamp);
        CREATE INDEX IF NOT EXISTS idx_audit_app ON permission_audits(app_id);
        CREATE INDEX IF NOT EXISTS idx_audit_category ON permission_audits(permission_category);
        CREATE INDEX IF NOT EXISTS idx_audit_composite ON permission_audits(app_id, timestamp);
    )";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_auditDb, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qCritical() << "Cannot create audit table:" << errMsg;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

bool PrivacyManager::requestPermission(const QString& appId, PermissionCategory category,
                                      const QString& details)
{
    // Check global setting first
    switch (category) {
        case Microphone:
            if (!isGlobalMicrophoneEnabled()) {
                logAuditRecord({QDateTime::currentMSecsSinceEpoch(), appId, category,
                               AuditDenied, SystemDenied, details, "global_disabled"});
                return false;
            }
            break;
        case Camera:
            if (!isGlobalCameraEnabled()) {
                logAuditRecord({QDateTime::currentMSecsSinceEpoch(), appId, category,
                               AuditDenied, SystemDenied, details, "global_disabled"});
                return false;
            }
            break;
        case Network:
            if (!isGlobalNetworkEnabled()) {
                logAuditRecord({QDateTime::currentMSecsSinceEpoch(), appId, category,
                               AuditDenied, SystemDenied, details, "global_disabled"});
                return false;
            }
            break;
        default:
            break;
    }

    // Check stored policy
    PermissionState currentState = checkPermissionPolicy(appId, category);

    if (currentState == Denied || currentState == SystemDenied) {
        logAuditRecord({QDateTime::currentMSecsSinceEpoch(), appId, category,
                       AuditDenied, currentState, details, "policy_denied"});
        return false;
    }

    if (currentState == AllowedAlways) {
        logAuditRecord({QDateTime::currentMSecsSinceEpoch(), appId, category,
                       AuditGranted, AllowedAlways, details, "policy_allowed"});
        logAccess(appId, category, details);
        return true;
    }

    if (currentState == AllowedOnce) {
        logAuditRecord({QDateTime::currentMSecsSinceEpoch(), appId, category,
                       AuditGranted, AllowedOnce, details, "policy_allowed_once"});
        logAccess(appId, category, details);
        // NOTE: Reset to AskEveryTime happens in onPermissionDialogResponse after user confirms
        // Do NOT reset here - allow the app to use the permission first
        return true;
    }

    // AskEveryTime - emit signal for dialog
    logAuditRecord({QDateTime::currentMSecsSinceEpoch(), appId, category,
                   AuditRequested, currentState, details, ""});

    AppMetadata metadata = getAppMetadata(appId);
    emit permissionRequested(appId, (int)category, metadata.name, details);

    // In real UI, dialog will call onPermissionDialogResponse()
    // For now, default to deny
    return false;
}

void PrivacyManager::setPermission(const QString& appId, PermissionCategory category,
                                  PermissionState state)
{
    // Validate state
    if (state < Denied || state > SystemDenied) {
        qWarning() << "Invalid permission state:" << (int)state << "for" << appId;
        return;
    }

    // Validate appId
    if (appId.isEmpty() || appId.length() > 255) {
        qWarning() << "Invalid appId length";
        return;
    }

    // Validate category enum bounds
    if (category < Microphone || category > Audio) {
        qWarning() << "Invalid permission category:" << (int)category;
        return;
    }

    QMutexLocker policyLocker(&m_policyMutex);
    m_policies[appId][category] = state;

    // Protect QSettings access
    QMutexLocker settingsLocker(&m_settingsMutex);
    m_settings->beginGroup("Permissions");
    m_settings->beginGroup(appId);
    m_settings->setValue(permissionCategoryToString(category), (int)state);
    m_settings->endGroup();
    m_settings->endGroup();
    m_settings->sync();
    settingsLocker.unlock();
    policyLocker.unlock();

    logAuditRecord({QDateTime::currentMSecsSinceEpoch(), appId, category,
                   AuditChanged, state, "", ""});

    emit permissionDecided(appId, (int)category, (int)state);
}

PrivacyManager::PermissionState PrivacyManager::getPermission(const QString& appId,
                                                             PermissionCategory category)
{
    return checkPermissionPolicy(appId, category);
}

PrivacyManager::PermissionState PrivacyManager::checkPermissionPolicy(const QString& appId,
                                                                     PermissionCategory category)
{
    QMutexLocker locker(&m_policyMutex);  // CRITICAL: Protect m_policies access
    if (m_policies.contains(appId) && m_policies[appId].contains((int)category)) {
        return m_policies[appId][(int)category];
    }
    return AskEveryTime;  // Default: ask on first request
}

void PrivacyManager::denyAllPermissionsForApp(const QString& appId)
{
    // Calculate from actual enum max value to avoid hardcoding
    static const int PERMISSION_COUNT = static_cast<int>(Audio) + 1;
    for (int i = 0; i < PERMISSION_COUNT; ++i) {
        setPermission(appId, (PermissionCategory)i, Denied);
    }
}

void PrivacyManager::allowAllPermissionsForApp(const QString& appId)
{
    static const int PERMISSION_COUNT = static_cast<int>(Audio) + 1;
    for (int i = 0; i < PERMISSION_COUNT; ++i) {
        setPermission(appId, (PermissionCategory)i, AllowedAlways);
    }
}

void PrivacyManager::resetPermissionsForApp(const QString& appId)
{
    QMutexLocker policyLocker(&m_policyMutex);
    QMutexLocker settingsLocker(&m_settingsMutex);
    
    m_settings->beginGroup("Permissions");
    m_settings->remove(appId);
    m_settings->endGroup();
    m_settings->sync();

    if (m_policies.contains(appId)) {
        m_policies.remove(appId);
    }
}

QList<PrivacyManager::PermissionRecord> PrivacyManager::getAuditTrail(int daysBack)
{
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000);
    
    const char* sql = "SELECT timestamp, app_id, permission_category, action, permission_state, "
                     "details, user_decision FROM permission_audits WHERE timestamp >= ? "
                     "ORDER BY timestamp DESC LIMIT 1000";
    
    QMutexLocker locker(&m_dbMutex);
    QList<PermissionRecord> records;
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_auditDb, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Cannot prepare audit query:" << sqlite3_errmsg(m_auditDb);
        return records;
    }

    sqlite3_bind_int64(stmt, 1, cutoffTime);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        PermissionRecord record;
        record.timestamp = sqlite3_column_int64(stmt, 0);
        record.appId = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 1));
        record.category = (PermissionCategory)sqlite3_column_int(stmt, 2);
        record.action = (AuditAction)sqlite3_column_int(stmt, 3);
        record.state = (PermissionState)sqlite3_column_int(stmt, 4);
        record.details = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 5));
        record.userDecision = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 6));

        if (record.isValid()) {
            records.append(record);
        }
    }

    sqlite3_finalize(stmt);
    return records;
}

QList<PrivacyManager::PermissionRecord> PrivacyManager::getAuditTrailForApp(const QString& appId,
                                                                            int daysBack)
{
    if (appId.isEmpty() || appId.length() > 255) {
        qWarning() << "Invalid appId length";
        return QList<PermissionRecord>();
    }

    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000);
    
    const char* sql = "SELECT timestamp, app_id, permission_category, action, permission_state, "
                     "details, user_decision FROM permission_audits WHERE app_id = ? AND timestamp >= ? "
                     "ORDER BY timestamp DESC LIMIT 1000";
    
    QMutexLocker locker(&m_dbMutex);
    QList<PermissionRecord> records;
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_auditDb, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Cannot prepare audit query:" << sqlite3_errmsg(m_auditDb);
        return records;
    }

    // FIX: Keep strings alive until finalize (use SQLITE_TRANSIENT not SQLITE_STATIC)
    std::string appIdStr = appId.toStdString();
    sqlite3_bind_text(stmt, 1, appIdStr.c_str(), appIdStr.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, cutoffTime);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        PermissionRecord record;
        record.timestamp = sqlite3_column_int64(stmt, 0);
        record.appId = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 1));
        record.category = (PermissionCategory)sqlite3_column_int(stmt, 2);
        record.action = (AuditAction)sqlite3_column_int(stmt, 3);
        record.state = (PermissionState)sqlite3_column_int(stmt, 4);
        record.details = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 5));
        record.userDecision = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 6));

        if (record.isValid()) {
            records.append(record);
        }
    }

    sqlite3_finalize(stmt);
    return records;
}

QList<PrivacyManager::PermissionRecord> PrivacyManager::getAuditTrailForCategory(
    PermissionCategory category, int daysBack)
{
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000);
    
    const char* sql = "SELECT timestamp, app_id, permission_category, action, permission_state, "
                     "details, user_decision FROM permission_audits WHERE permission_category = ? AND timestamp >= ? "
                     "ORDER BY timestamp DESC LIMIT 1000";
    
    QMutexLocker locker(&m_dbMutex);
    QList<PermissionRecord> records;
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_auditDb, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Cannot prepare audit query:" << sqlite3_errmsg(m_auditDb);
        return records;
    }

    sqlite3_bind_int(stmt, 1, (int)category);
    sqlite3_bind_int64(stmt, 2, cutoffTime);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        PermissionRecord record;
        record.timestamp = sqlite3_column_int64(stmt, 0);
        record.appId = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 1));
        record.category = (PermissionCategory)sqlite3_column_int(stmt, 2);
        record.action = (AuditAction)sqlite3_column_int(stmt, 3);
        record.state = (PermissionState)sqlite3_column_int(stmt, 4);
        record.details = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 5));
        record.userDecision = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 6));

        if (record.isValid()) {
            records.append(record);
        }
    }

    sqlite3_finalize(stmt);
    return records;
}

void PrivacyManager::clearAuditTrail(int daysBack)
{
    QMutexLocker locker(&m_dbMutex);
    
    // Validate daysBack parameter
    if (daysBack < 0) {
        qWarning() << "clearAuditTrail: daysBack must be non-negative, got" << daysBack;
        return;
    }

    if (!m_auditDb) {
        qWarning() << "Audit database not available";
        return;
    }

    if (daysBack == 0) {
        const char* sql = "DELETE FROM permission_audits;";
        int rc = sqlite3_exec(m_auditDb, sql, nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK) {
            qWarning() << "Failed to clear audit trail:" << sqlite3_errmsg(m_auditDb);
        } else {
            qDebug() << "Audit trail cleared (all records)";
            // Log this action itself (system action, not app-specific)
            logAuditRecord({QDateTime::currentMSecsSinceEpoch(), "system", Files,
                           AuditChanged, Denied, "clearAuditTrail(0)", "system_maintenance"});
        }
    } else {
        qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (qint64(daysBack) * 24 * 60 * 60 * 1000);
        const char* sql = "DELETE FROM permission_audits WHERE timestamp < ?";
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(m_auditDb, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            qWarning() << "Cannot prepare clear query:" << sqlite3_errmsg(m_auditDb);
            return;
        }
        
        sqlite3_bind_int64(stmt, 1, cutoffTime);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            qWarning() << "Failed to clear old audit records:" << sqlite3_errmsg(m_auditDb);
        } else {
            qDebug() << "Old audit records cleared (older than" << daysBack << "days)";
            // Log this action
            logAuditRecord({QDateTime::currentMSecsSinceEpoch(), "system", Files,
                           AuditChanged, Denied, QString("clearAuditTrail(%1)").arg(daysBack), 
                           "system_maintenance"});
        }
        sqlite3_finalize(stmt);
    }
}

QList<PrivacyManager::PermissionRecord> PrivacyManager::queryAuditTrail(const QString& whereClause,
                                                                       int limit)
{
    // DEPRECATED - DO NOT USE. This function is removed to prevent SQL injection.
    // Use specific query functions instead: getAuditTrail(), getAuditTrailForApp(), etc.
    Q_UNUSED(whereClause);
    Q_UNUSED(limit);
    qCritical() << "ERROR: queryAuditTrail() is deprecated and should never be called";
    return QList<PermissionRecord>();
}

void PrivacyManager::logAuditRecord(const PermissionRecord& record)
{
    // Pre-validate record before insert
    if (!record.isValid()) {
        qWarning() << "Cannot log invalid audit record for app" << record.appId;
        return;
    }

    QMutexLocker locker(&m_dbMutex);
    
    if (!m_auditDb) {
        qWarning() << "Audit database not available, cannot log record";
        return;
    }
    
    const char* sql = "INSERT INTO permission_audits "
                     "(timestamp, app_id, permission_category, action, permission_state, details, user_decision) "
                     "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_auditDb, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Cannot prepare insert:" << sqlite3_errmsg(m_auditDb);
        return;
    }

    // Convert strings to C++ strings that live until finalize
    std::string appIdStr = record.appId.toStdString();
    std::string detailsStr = record.details.toUtf8().constData();
    std::string decisionStr = record.userDecision.toUtf8().constData();

    sqlite3_bind_int64(stmt, 1, record.timestamp);
    sqlite3_bind_text(stmt, 2, appIdStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, (int)record.category);
    sqlite3_bind_int(stmt, 4, (int)record.action);
    sqlite3_bind_int(stmt, 5, (int)record.state);
    sqlite3_bind_text(stmt, 6, detailsStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, decisionStr.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        qWarning() << "Cannot insert audit record:" << sqlite3_errmsg(m_auditDb);
    }

    sqlite3_finalize(stmt);
}

int PrivacyManager::calculatePrivacyScore()
{
    // Copy policies under lock, release before calculations to avoid deadlock
    QList<QString> appIds;
    {
        QMutexLocker locker(&m_policyMutex);
        appIds = m_policies.keys();
    }  // Release m_policyMutex here
    
    int score = 100;

    // Deduct for each app with unnecessary permissions (no lock held - helper methods can acquire)
    for (const auto& appId : appIds) {
        AppMetadata metadata = getAppMetadata(appId);
        int unnecessary = countUnnecessaryPermissions(metadata);
        score -= (unnecessary * 2);  // -2 per unnecessary permission

        int dangerous = countDangerousAppPermissions(appId);
        score -= (dangerous * 5);  // -5 per dangerous permission
    }

    // Bonus for global restrictions
    if (!isGlobalMicrophoneEnabled()) score += 5;
    if (!isGlobalCameraEnabled()) score += 5;

    // Ensure score is in valid range
    return qMax(0, qMin(100, score));
}

QString PrivacyManager::getPrivacyRecommendations()
{
    QMutexLocker locker(&m_policyMutex);  // Protect m_policies access
    
    QString recommendations;

    // Check for apps with excessive permissions
    for (const auto& appId : m_policies.keys()) {
        int allowedCount = 0;
        for (int i = Microphone; i <= Audio; ++i) {
            PermissionState state = m_policies[appId].value(i, AskEveryTime);
            if (state == AllowedAlways) {
                allowedCount++;
            }
        }

        if (allowedCount > 5) {
            recommendations += QString("Consider restricting permissions for %1\n").arg(appId);
        }
    }

    if (recommendations.isEmpty()) {
        recommendations = "Your privacy settings look good!";
    }

    return recommendations;
}

void PrivacyManager::registerApp(const AppMetadata& metadata)
{
    QMutexLocker locker(&m_registryMutex);
    m_appRegistry[metadata.id] = metadata;
}

PrivacyManager::AppMetadata PrivacyManager::getAppMetadata(const QString& appId)
{
    QMutexLocker locker(&m_registryMutex);
    if (m_appRegistry.contains(appId)) {
        return m_appRegistry[appId];
    }
    return {appId, appId, "", "", {}, 0, 0};
}

QList<PrivacyManager::AppMetadata> PrivacyManager::getAllRegisteredApps()
{
    return m_appRegistry.values();
}

QList<QString> PrivacyManager::getAppsWithPermission(PermissionCategory category)
{
    QMutexLocker locker(&m_policyMutex);  // Protect m_policies read
    
    QList<QString> apps;
    for (const auto& appId : m_policies.keys()) {
        if (m_policies[appId].contains((int)category)) {
            PermissionState state = m_policies[appId][(int)category];
            if (state == AllowedAlways || state == AllowedOnce) {
                apps.append(appId);
            }
        }
    }
    return apps;
}

void PrivacyManager::logAccess(const QString& appId, PermissionCategory category,
                              const QString& details)
{
    logAuditRecord({QDateTime::currentMSecsSinceEpoch(), appId, category,
                   AuditUsed, AllowedAlways, details, ""});

    // Update access count in metadata
    QMutexLocker locker(&m_registryMutex);
    if (m_appRegistry.contains(appId)) {
        m_appRegistry[appId].lastAccessTime = QDateTime::currentMSecsSinceEpoch();
        m_appRegistry[appId].accessCount++;
    }
    locker.unlock();

    emit accessLogged(appId, (int)category, QDateTime::currentMSecsSinceEpoch());
}

QList<PrivacyManager::PermissionRecord> PrivacyManager::getCurrentActivity()
{
    qint64 fiveMinutesAgo = QDateTime::currentMSecsSinceEpoch() - (5 * 60 * 1000);
    
    const char* sql = "SELECT timestamp, app_id, permission_category, action, permission_state, "
                     "details, user_decision FROM permission_audits WHERE timestamp >= ? AND action = ? "
                     "ORDER BY timestamp DESC LIMIT 50";
    
    QMutexLocker locker(&m_dbMutex);
    QList<PermissionRecord> records;
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_auditDb, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Cannot prepare activity query:" << sqlite3_errmsg(m_auditDb);
        return records;
    }

    sqlite3_bind_int64(stmt, 1, fiveMinutesAgo);
    sqlite3_bind_int(stmt, 2, (int)AuditUsed);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        PermissionRecord record;
        record.timestamp = sqlite3_column_int64(stmt, 0);
        record.appId = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 1));
        record.category = (PermissionCategory)sqlite3_column_int(stmt, 2);
        record.action = (AuditAction)sqlite3_column_int(stmt, 3);
        record.state = (PermissionState)sqlite3_column_int(stmt, 4);
        record.details = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 5));
        record.userDecision = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 6));

        if (record.isValid()) {
            records.append(record);
        }
    }

    sqlite3_finalize(stmt);
    return records;
}

bool PrivacyManager::isAppCurrentlyAccessing(const QString& appId, PermissionCategory category)
{
    if (appId.isEmpty() || appId.length() > 255) {
        return false;
    }

    qint64 oneSecondAgo = QDateTime::currentMSecsSinceEpoch() - 1000;
    
    const char* sql = "SELECT COUNT(*) FROM permission_audits WHERE app_id = ? AND permission_category = ? "
                     "AND timestamp >= ? AND action = ?";
    
    QMutexLocker locker(&m_dbMutex);
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_auditDb, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Cannot prepare access check query:" << sqlite3_errmsg(m_auditDb);
        return false;
    }

    // FIX: Keep strings alive until finalize (use SQLITE_TRANSIENT not SQLITE_STATIC)
    std::string appIdStr = appId.toStdString();
    sqlite3_bind_text(stmt, 1, appIdStr.c_str(), appIdStr.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, (int)category);
    sqlite3_bind_int64(stmt, 3, oneSecondAgo);
    sqlite3_bind_int(stmt, 4, (int)AuditUsed);

    bool isAccessing = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        isAccessing = (count > 0);
    }

    sqlite3_finalize(stmt);
    return isAccessing;
}

void PrivacyManager::setGlobalMicrophoneEnabled(bool enabled)
{
    QMutexLocker locker(&m_settingsMutex);
    m_settings->setValue("GlobalSettings/MicrophoneEnabled", enabled);
    m_settings->sync();
    locker.unlock();
    emit globalSettingChanged(Microphone, enabled);
}

bool PrivacyManager::isGlobalMicrophoneEnabled() const
{
    // Default: enabled (true) - users can opt out
    QMutexLocker locker(&m_settingsMutex);
    return m_settings->value("GlobalSettings/MicrophoneEnabled", true).toBool();
}

void PrivacyManager::setGlobalCameraEnabled(bool enabled)
{
    QMutexLocker locker(&m_settingsMutex);
    m_settings->setValue("GlobalSettings/CameraEnabled", enabled);
    m_settings->sync();
    locker.unlock();
    emit globalSettingChanged(Camera, enabled);
}

bool PrivacyManager::isGlobalCameraEnabled() const
{
    // Default: enabled (true) - users can opt out
    QMutexLocker locker(&m_settingsMutex);
    return m_settings->value("GlobalSettings/CameraEnabled", true).toBool();
}

void PrivacyManager::setGlobalNetworkEnabled(bool enabled)
{
    QMutexLocker locker(&m_settingsMutex);
    m_settings->setValue("GlobalSettings/NetworkEnabled", enabled);
    m_settings->sync();
    locker.unlock();
    emit globalSettingChanged(Network, enabled);
}

bool PrivacyManager::isGlobalNetworkEnabled() const
{
    // Default: enabled (true) - users can opt out
    QMutexLocker locker(&m_settingsMutex);
    return m_settings->value("GlobalSettings/NetworkEnabled", true).toBool();
}

QString PrivacyManager::exportPolicy(const QString& appId)
{
    QMutexLocker locker(&m_policyMutex);  // Protect m_policies read
    
    QJsonObject policyObj;
    policyObj["appId"] = appId;
    policyObj["exportTime"] = QDateTime::currentDateTime().toString();

    QJsonArray permissions;
    if (m_policies.contains(appId)) {
        for (auto it = m_policies[appId].begin(); it != m_policies[appId].end(); ++it) {
            QJsonObject perm;
            perm["category"] = permissionCategoryToString((PermissionCategory)it.key());
            perm["state"] = it.value();
            permissions.append(perm);
        }
    }

    policyObj["permissions"] = permissions;

    QJsonDocument doc(policyObj);
    return QString::fromUtf8(doc.toJson());
}

bool PrivacyManager::importPolicy(const QString& appId, const QString& policyJson)
{
    QJsonDocument doc = QJsonDocument::fromJson(policyJson.toUtf8());
    if (!doc.isObject()) {
        return false;
    }

    QJsonObject obj = doc.object();
    QJsonArray permissions = obj["permissions"].toArray();

    for (const auto& permValue : permissions) {
        QJsonObject perm = permValue.toObject();
        QString category = perm["category"].toString();
        int state = perm["state"].toInt();

        PermissionCategory cat = stringToPermissionCategory(category);
        setPermission(appId, cat, (PermissionState)state);
    }

    return true;
}

void PrivacyManager::onPermissionDialogResponse(const QString& appId, PermissionCategory category,
                                               PermissionState response)
{
    setPermission(appId, category, response);

    // If user selected AllowedOnce, reset to AskEveryTime for next time
    if (response == AllowedOnce) {
        QTimer::singleShot(100, this, [this, appId, category]() {
            setPermission(appId, category, AskEveryTime);
        });
    }

    if (response != Denied && response != SystemDenied) {
        logAccess(appId, category, "Dialog response");
    }
}

void PrivacyManager::onSystemPolicyChange()
{
    // Called when system-wide policy changes (e.g., from polkit)
    // For now, just reload settings
    m_settings->sync();
}

void PrivacyManager::onAuditMaintenanceTimer()
{
    QMutexLocker locker(&m_dbMutex);
    
    if (!m_auditDb) {
        qWarning() << "Audit DB not available, skipping maintenance";
        return;
    }

    // Keep only last 90 days of audit trail
    // Use qint64 for calculation to avoid overflow
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (qint64(90) * 24 * 60 * 60 * 1000);
    
    const char* sql = "DELETE FROM permission_audits WHERE timestamp < ?";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(m_auditDb, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qCritical() << "Cannot prepare maintenance query:" << sqlite3_errmsg(m_auditDb);
        return;
    }

    sqlite3_bind_int64(stmt, 1, cutoffTime);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        qCritical() << "Failed to clean old records:" << sqlite3_errmsg(m_auditDb);
    } else {
        qDebug() << "Audit maintenance completed";
    }

    sqlite3_finalize(stmt);
}

QString PrivacyManager::permissionCategoryToString(PermissionCategory category)
{
    switch (category) {
        case Microphone: return "Microphone";
        case Camera: return "Camera";
        case Files: return "Files";
        case Network: return "Network";
        case Location: return "Location";
        case Clipboard: return "Clipboard";
        case Contacts: return "Contacts";
        case Calendar: return "Calendar";
        case Photos: return "Photos";
        case Bluetooth: return "Bluetooth";
        case USB: return "USB";
        case Printing: return "Printing";
        case Screenshots: return "Screenshots";
        case SystemMonitor: return "SystemMonitor";
        case Audio: return "Audio";
        default: return "Unknown";
    }
}

PrivacyManager::PermissionCategory PrivacyManager::stringToPermissionCategory(const QString& str)
{
    if (str == "Microphone") return Microphone;
    if (str == "Camera") return Camera;
    if (str == "Files") return Files;
    if (str == "Network") return Network;
    if (str == "Location") return Location;
    if (str == "Clipboard") return Clipboard;
    if (str == "Contacts") return Contacts;
    if (str == "Calendar") return Calendar;
    if (str == "Photos") return Photos;
    if (str == "Bluetooth") return Bluetooth;
    if (str == "USB") return USB;
    if (str == "Printing") return Printing;
    if (str == "Screenshots") return Screenshots;
    if (str == "SystemMonitor") return SystemMonitor;
    if (str == "Audio") return Audio;
    return Microphone;  // Default
}

QString PrivacyManager::PermissionRecord::categoryName() const
{
    return PrivacyManager::permissionCategoryToString(category);
}

QString PrivacyManager::PermissionRecord::actionName() const
{
    switch (action) {
        case AuditRequested: return "Requested";
        case AuditGranted: return "Granted";
        case AuditDenied: return "Denied";
        case AuditUsed: return "Used";
        case AuditChanged: return "Changed";
        default: return "Unknown";
    }
}

QString PrivacyManager::PermissionRecord::stateName() const
{
    switch (state) {
        case Denied: return "Denied";
        case AllowedAlways: return "Allowed Always";
        case AskEveryTime: return "Ask Every Time";
        case AllowedOnce: return "Allowed Once";
        case SystemDenied: return "System Denied";
        default: return "Unknown";
    }
}

QString PrivacyManager::PermissionRecord::formattedTime() const
{
    return QDateTime::fromMSecsSinceEpoch(timestamp).toString("yyyy-MM-dd hh:mm:ss");
}

bool PrivacyManager::PermissionRecord::isValid() const
{
    // Validate record before adding to results
    if (appId.isEmpty() || appId.length() > 255) return false;
    if (timestamp <= 0) return false;
    // Validate UTF-8 byte size, not character count
    if (details.toUtf8().size() > 1000) return false;
    if (userDecision.toUtf8().size() > 100) return false;
    if (action < AuditRequested || action > AuditChanged) return false;
    if (state < Denied || state > SystemDenied) return false;
    return true;
}

int PrivacyManager::countUnnecessaryPermissions(const AppMetadata& app)
{
    // This is a heuristic - certain apps don't need certain permissions
    // Terminal should never need camera, most apps don't need location, etc.
    int count = 0;

    if (app.id.contains("terminal", Qt::CaseInsensitive)) {
        if (getPermission(app.id, Camera) != Denied) count++;
        if (getPermission(app.id, Photos) != Denied) count++;
    }

    if (app.id.contains("files", Qt::CaseInsensitive)) {
        if (getPermission(app.id, Microphone) != Denied) count++;
        if (getPermission(app.id, Camera) != Denied) count++;
    }

    return count;
}

int PrivacyManager::countDangerousAppPermissions(const QString& appId)
{
    int count = 0;
    if (getPermission(appId, Microphone) == AllowedAlways) count++;
    if (getPermission(appId, Camera) == AllowedAlways) count++;
    if (getPermission(appId, Location) == AllowedAlways) count++;
    return count;
}
