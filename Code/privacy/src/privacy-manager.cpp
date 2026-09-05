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
#include <algorithm>

// Singleton instance
PrivacyManager* PrivacyManager::s_instance = nullptr;

PrivacyManager::PrivacyManager(QObject* parent)
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
    connect(maintenanceTimer, &QTimer::timeout, this, &PrivacyManager::onAuditMaintenanceTimer);
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
    if (m_auditDb) {
        sqlite3_close(m_auditDb);
    }
}

PrivacyManager* PrivacyManager::instance()
{
    if (!s_instance) {
        s_instance = new PrivacyManager(QCoreApplication::instance());
    }
    return s_instance;
}

bool PrivacyManager::initializeAuditDatabase()
{
    int rc = sqlite3_open(m_auditDbPath.toStdString().c_str(), &m_auditDb);
    if (rc != SQLITE_OK) {
        qCritical() << "Cannot open audit database:" << sqlite3_errmsg(m_auditDb);
        return false;
    }

    // Enable foreign keys
    sqlite3_exec(m_auditDb, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

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
            details TEXT,
            user_decision TEXT,
            FOREIGN KEY(app_id) REFERENCES apps(id)
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
        // Reset to AskEveryTime after use
        setPermission(appId, category, AskEveryTime);
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
    m_policies[appId][category] = state;

    // Save to settings
    m_settings->beginGroup("Permissions");
    m_settings->beginGroup(appId);
    m_settings->setValue(permissionCategoryToString(category), (int)state);
    m_settings->endGroup();
    m_settings->endGroup();
    m_settings->sync();

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
    if (m_policies.contains(appId) && m_policies[appId].contains((int)category)) {
        return m_policies[appId][(int)category];
    }
    return AskEveryTime;  // Default: ask on first request
}

void PrivacyManager::denyAllPermissionsForApp(const QString& appId)
{
    for (int i = Microphone; i <= Audio; ++i) {
        setPermission(appId, (PermissionCategory)i, Denied);
    }
}

void PrivacyManager::allowAllPermissionsForApp(const QString& appId)
{
    for (int i = Microphone; i <= Audio; ++i) {
        setPermission(appId, (PermissionCategory)i, AllowedAlways);
    }
}

void PrivacyManager::resetPermissionsForApp(const QString& appId)
{
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
    QString whereClause = QString("WHERE timestamp >= %1 ORDER BY timestamp DESC").arg(cutoffTime);
    return queryAuditTrail(whereClause);
}

QList<PrivacyManager::PermissionRecord> PrivacyManager::getAuditTrailForApp(const QString& appId,
                                                                            int daysBack)
{
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000);
    QString whereClause = QString("WHERE app_id = '%1' AND timestamp >= %2 ORDER BY timestamp DESC")
                         .arg(appId, QString::number(cutoffTime));
    return queryAuditTrail(whereClause);
}

QList<PrivacyManager::PermissionRecord> PrivacyManager::getAuditTrailForCategory(
    PermissionCategory category, int daysBack)
{
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000);
    QString whereClause = QString("WHERE permission_category = %1 AND timestamp >= %2 ORDER BY timestamp DESC")
                         .arg((int)category, QString::number(cutoffTime));
    return queryAuditTrail(whereClause);
}

void PrivacyManager::clearAuditTrail(int daysBack)
{
    if (daysBack == 0) {
        sqlite3_exec(m_auditDb, "DELETE FROM permission_audits;", nullptr, nullptr, nullptr);
    } else {
        qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000);
        QString sql = QString("DELETE FROM permission_audits WHERE timestamp < %1").arg(cutoffTime);
        sqlite3_exec(m_auditDb, sql.toStdString().c_str(), nullptr, nullptr, nullptr);
    }
}

QList<PrivacyManager::PermissionRecord> PrivacyManager::queryAuditTrail(const QString& whereClause,
                                                                       int limit)
{
    QList<PermissionRecord> records;

    QString sql = QString("SELECT timestamp, app_id, permission_category, action, permission_state, "
                         "details, user_decision FROM permission_audits %1 LIMIT %2")
                 .arg(whereClause, QString::number(limit));

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(m_auditDb, sql.toStdString().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Cannot prepare audit query:" << sqlite3_errmsg(m_auditDb);
        return records;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        PermissionRecord record;
        record.timestamp = sqlite3_column_int64(stmt, 0);
        record.appId = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 1));
        record.category = (PermissionCategory)sqlite3_column_int(stmt, 2);
        record.action = (AuditAction)sqlite3_column_int(stmt, 3);
        record.state = (PermissionState)sqlite3_column_int(stmt, 4);
        record.details = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 5));
        record.userDecision = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 6));

        records.append(record);
    }

    sqlite3_finalize(stmt);
    return records;
}

void PrivacyManager::logAuditRecord(const PermissionRecord& record)
{
    const char* sql = "INSERT INTO permission_audits "
                     "(timestamp, app_id, permission_category, action, permission_state, details, user_decision) "
                     "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(m_auditDb, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Cannot prepare insert:" << sqlite3_errmsg(m_auditDb);
        return;
    }

    sqlite3_bind_int64(stmt, 1, record.timestamp);
    sqlite3_bind_text(stmt, 2, record.appId.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, (int)record.category);
    sqlite3_bind_int(stmt, 4, (int)record.action);
    sqlite3_bind_int(stmt, 5, (int)record.state);
    sqlite3_bind_text(stmt, 6, record.details.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, record.userDecision.toStdString().c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        qWarning() << "Cannot insert audit record:" << sqlite3_errmsg(m_auditDb);
    }

    sqlite3_finalize(stmt);
}

int PrivacyManager::calculatePrivacyScore()
{
    int score = 100;

    // Deduct for each app with unnecessary permissions
    for (const auto& appId : m_policies.keys()) {
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
    m_appRegistry[metadata.id] = metadata;
}

PrivacyManager::AppMetadata PrivacyManager::getAppMetadata(const QString& appId)
{
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
    if (m_appRegistry.contains(appId)) {
        m_appRegistry[appId].lastAccessTime = QDateTime::currentMSecsSinceEpoch();
        m_appRegistry[appId].accessCount++;
    }

    emit accessLogged(appId, (int)category, QDateTime::currentMSecsSinceEpoch());
}

QList<PrivacyManager::PermissionRecord> PrivacyManager::getCurrentActivity()
{
    qint64 fiveMinutesAgo = QDateTime::currentMSecsSinceEpoch() - (5 * 60 * 1000);
    QString whereClause = QString("WHERE timestamp >= %1 AND action = %2 ORDER BY timestamp DESC")
                         .arg(QString::number(fiveMinutesAgo), QString::number((int)AuditUsed));
    return queryAuditTrail(whereClause, 50);
}

bool PrivacyManager::isAppCurrentlyAccessing(const QString& appId, PermissionCategory category)
{
    qint64 oneSecondAgo = QDateTime::currentMSecsSinceEpoch() - 1000;
    QString whereClause = QString("WHERE app_id = '%1' AND permission_category = %2 "
                                 "AND timestamp >= %3 AND action = %4")
                         .arg(appId, QString::number((int)category),
                              QString::number(oneSecondAgo), QString::number((int)AuditUsed));
    QList<PermissionRecord> records = queryAuditTrail(whereClause, 1);
    return !records.isEmpty();
}

void PrivacyManager::setGlobalMicrophoneEnabled(bool enabled)
{
    m_settings->setValue("GlobalSettings/MicrophoneEnabled", enabled);
    m_settings->sync();
    emit globalSettingChanged(Microphone, enabled);
}

bool PrivacyManager::isGlobalMicrophoneEnabled() const
{
    return m_settings->value("GlobalSettings/MicrophoneEnabled", true).toBool();
}

void PrivacyManager::setGlobalCameraEnabled(bool enabled)
{
    m_settings->setValue("GlobalSettings/CameraEnabled", enabled);
    m_settings->sync();
    emit globalSettingChanged(Camera, enabled);
}

bool PrivacyManager::isGlobalCameraEnabled() const
{
    return m_settings->value("GlobalSettings/CameraEnabled", true).toBool();
}

void PrivacyManager::setGlobalNetworkEnabled(bool enabled)
{
    m_settings->setValue("GlobalSettings/NetworkEnabled", enabled);
    m_settings->sync();
    emit globalSettingChanged(Network, enabled);
}

bool PrivacyManager::isGlobalNetworkEnabled() const
{
    return m_settings->value("GlobalSettings/NetworkEnabled", true).toBool();
}

QString PrivacyManager::exportPolicy(const QString& appId)
{
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
    // Keep only last 90 days of audit trail
    clearAuditTrail(90);
    qDebug() << "Audit maintenance completed";
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
