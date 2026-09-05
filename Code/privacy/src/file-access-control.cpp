#include "file-access-control.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QFileInfo>

FileAccessControl::FileAccessControl(PrivacyManager* privacyManager, QObject* parent)
    : QObject(parent), m_privacyManager(privacyManager)
{
    if (!m_privacyManager) {
        qCritical() << "FileAccessControl: PrivacyManager is null";
        return;
    }

    // Initialize protected paths
    initializeProtectedPaths();

    // Set up sandbox base directory
    m_sandboxBaseDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.local/share/arbor-sandbox";
    QDir().mkpath(m_sandboxBaseDir);

    // Connect to privacy manager signals
    connect(m_privacyManager, QOverload<const QString&, int, int>::of(&PrivacyManager::permissionDecided),
            this, &FileAccessControl::onAppPermissionChanged);

    qDebug() << "FileAccessControl initialized with sandbox base:" << m_sandboxBaseDir;
}

FileAccessControl::~FileAccessControl()
{
    // Cleanup
}

bool FileAccessControl::requestFileAccess(const QString& appId, const QString& appName,
                                         const QString& filePath, AccessType accessType,
                                         const QString& reason)
{
    QMutexLocker locker(&m_mutex);

    // Normalize the path
    QString normalizedPath = normalizePath(filePath);

    // Check if it's a protected path (system areas)
    if (isSystemProtectedPath(normalizedPath)) {
        qDebug() << "File access denied for" << appId << "- protected system path:" << normalizedPath;
        logFileAccess({QDateTime::currentMSecsSinceEpoch(), appId, appName, normalizedPath,
                      (int)accessType, "denied", reason, false});
        emit fileAccessBlocked(appId, appName, normalizedPath);
        return false;
    }

    // Check against whitelist/blacklist
    if (!checkPathAgainstPolicy(appId, normalizedPath, accessType)) {
        qDebug() << "File access denied for" << appId << "- policy violation:" << normalizedPath;
        logFileAccess({QDateTime::currentMSecsSinceEpoch(), appId, appName, normalizedPath,
                      (int)accessType, "denied", reason, false});
        emit fileAccessBlocked(appId, appName, normalizedPath);
        return false;
    }

    // If requesting access to special directory, ask user
    if (isHomeSubdirectory(normalizedPath) && !isSandboxDirectory(normalizedPath)) {
        if (!m_specialApprovals[appId].contains(normalizedPath)) {
            requestSpecialDirectoryAccess(appId, appName, normalizedPath);
            logFileAccess({QDateTime::currentMSecsSinceEpoch(), appId, appName, normalizedPath,
                          (int)accessType, "requested", reason, false});
            return false;
        }
    }

    // Grant access
    logFileAccess({QDateTime::currentMSecsSinceEpoch(), appId, appName, normalizedPath,
                  (int)accessType, "granted", reason, true});
    emit fileAccessDecision(appId, normalizedPath, (int)accessType, true);

    return true;
}

void FileAccessControl::addToWhitelist(const QString& appId, const QString& directoryPath)
{
    QMutexLocker locker(&m_mutex);

    QString normalized = normalizePath(directoryPath);
    m_whitelists[appId].insert(normalized);

    qDebug() << "Added to whitelist for" << appId << ":" << normalized;
    emit accessPolicyChanged(appId);
}

void FileAccessControl::removeFromWhitelist(const QString& appId, const QString& directoryPath)
{
    QMutexLocker locker(&m_mutex);

    QString normalized = normalizePath(directoryPath);
    m_whitelists[appId].remove(normalized);

    qDebug() << "Removed from whitelist for" << appId << ":" << normalized;
    emit accessPolicyChanged(appId);
}

QList<QString> FileAccessControl::getWhitelist(const QString& appId)
{
    QMutexLocker locker(&m_mutex);
    return m_whitelists[appId].toList();
}

void FileAccessControl::addToBlacklist(const QString& appId, const QString& directoryPath)
{
    QMutexLocker locker(&m_mutex);

    QString normalized = normalizePath(directoryPath);
    m_blacklists[appId].insert(normalized);

    qDebug() << "Added to blacklist for" << appId << ":" << normalized;
    emit accessPolicyChanged(appId);
}

void FileAccessControl::removeFromBlacklist(const QString& appId, const QString& directoryPath)
{
    QMutexLocker locker(&m_mutex);

    QString normalized = normalizePath(directoryPath);
    m_blacklists[appId].remove(normalized);

    qDebug() << "Removed from blacklist for" << appId << ":" << normalized;
    emit accessPolicyChanged(appId);
}

QList<QString> FileAccessControl::getBlacklist(const QString& appId)
{
    QMutexLocker locker(&m_mutex);
    return m_blacklists[appId].toList();
}

QString FileAccessControl::getSandboxDirectory(const QString& appId)
{
    return m_sandboxBaseDir + "/" + appId;
}

QString FileAccessControl::createSandboxDirectory(const QString& appId)
{
    QMutexLocker locker(&m_mutex);

    QString sandboxPath = getSandboxDirectory(appId);
    QDir dir;

    if (dir.mkpath(sandboxPath)) {
        qDebug() << "Sandbox directory created for" << appId << ":" << sandboxPath;
        return sandboxPath;
    } else {
        qWarning() << "Failed to create sandbox directory for" << appId;
        return "";
    }
}

bool FileAccessControl::isSandboxDirectory(const QString& dirPath) const
{
    QString normalized = normalizePath(dirPath);
    return normalized.startsWith(normalizePath(m_sandboxBaseDir));
}

QList<QString> FileAccessControl::getAllSandboxDirectories()
{
    QMutexLocker locker(&m_mutex);

    QList<QString> sandboxDirs;
    QDir sandboxBase(m_sandboxBaseDir);

    for (const QString& appId : sandboxBase.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        sandboxDirs.append(m_sandboxBaseDir + "/" + appId);
    }

    return sandboxDirs;
}

bool FileAccessControl::isSystemProtectedPath(const QString& filePath) const
{
    QString normalized = normalizePath(filePath);

    for (const QString& protectedPath : m_protectedPaths) {
        if (isPathUnderDirectory(normalized, protectedPath)) {
            return true;
        }
    }

    return false;
}

QList<QString> FileAccessControl::getProtectedPaths() const
{
    return m_protectedPaths.toList();
}

bool FileAccessControl::isHomeDirectory(const QString& filePath) const
{
    QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return normalizePath(filePath) == normalizePath(home);
}

bool FileAccessControl::isHomeSubdirectory(const QString& filePath) const
{
    QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return isPathUnderDirectory(normalizePath(filePath), normalizePath(home));
}

void FileAccessControl::requestSpecialDirectoryAccess(const QString& appId, const QString& appName,
                                                     const QString& directoryPath)
{
    emit specialDirectoryAccessRequested(appId, appName, directoryPath);
}

void FileAccessControl::approveSpecialDirectoryAccess(const QString& appId, const QString& directoryPath)
{
    QMutexLocker locker(&m_mutex);

    QString normalized = normalizePath(directoryPath);
    m_specialApprovals[appId].insert(normalized);

    qDebug() << "Special directory access approved for" << appId << ":" << normalized;
}

void FileAccessControl::denySpecialDirectoryAccess(const QString& appId, const QString& directoryPath)
{
    QMutexLocker locker(&m_mutex);

    QString normalized = normalizePath(directoryPath);
    m_specialApprovals[appId].remove(normalized);

    qDebug() << "Special directory access denied for" << appId << ":" << normalized;
}

bool FileAccessControl::canAccessPath(const QString& appId, const QString& filePath, AccessType accessType)
{
    QMutexLocker locker(&m_mutex);

    QString normalizedPath = normalizePath(filePath);

    // Always deny system protected paths
    if (isSystemProtectedPath(normalizedPath)) {
        return false;
    }

    // Check blacklist first
    if (m_blacklists[appId].contains(normalizedPath)) {
        return false;
    }

    // If whitelist exists for this app, only allow whitelisted paths
    if (!m_whitelists[appId].isEmpty()) {
        for (const QString& whitelistedPath : m_whitelists[appId]) {
            if (isPathUnderDirectory(normalizedPath, whitelistedPath)) {
                return true;
            }
        }
        return false;
    }

    // If no whitelist, allow access to home/sandbox by default
    if (isSandboxDirectory(normalizedPath) || isHomeSubdirectory(normalizedPath)) {
        return true;
    }

    return false;
}

QList<FileAccessControl::FileAccessRecord> FileAccessControl::getFileAccessHistory(int daysBack)
{
    QMutexLocker locker(&m_mutex);

    QList<FileAccessRecord> filtered;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000LL);

    for (const auto& record : m_accessHistory) {
        if (record.timestamp >= cutoffTime) {
            filtered.append(record);
        }
    }

    return filtered;
}

int FileAccessControl::getTotalAccessAttempts(const QString& appId, int daysBack)
{
    QMutexLocker locker(&m_mutex);

    int count = 0;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000LL);

    for (const auto& record : m_accessHistory) {
        if (record.timestamp >= cutoffTime && record.appId == appId) {
            count++;
        }
    }

    return count;
}

int FileAccessControl::getAccessDenialCount(const QString& appId, int daysBack)
{
    QMutexLocker locker(&m_mutex);

    int count = 0;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000LL);

    for (const auto& record : m_accessHistory) {
        if (record.timestamp >= cutoffTime && record.appId == appId && !record.allowed) {
            count++;
        }
    }

    return count;
}

QMap<QString, int> FileAccessControl::getAccessStatistics(int daysBack)
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

void FileAccessControl::onAppPermissionChanged(const QString& appId, int categoryInt, int stateInt)
{
    if (categoryInt != PrivacyManager::FileAccess) {
        return;  // Not a file access permission change
    }

    qDebug() << "File access permission changed for" << appId << "- state:" << stateInt;
}

void FileAccessControl::initializeProtectedPaths()
{
    QMutexLocker locker(&m_mutex);

    // System directories that should never be accessible by user apps
    m_protectedPaths.insert(normalizePath("/etc"));
    m_protectedPaths.insert(normalizePath("/root"));
    m_protectedPaths.insert(normalizePath("/boot"));
    m_protectedPaths.insert(normalizePath("/sys"));
    m_protectedPaths.insert(normalizePath("/proc"));
    m_protectedPaths.insert(normalizePath("/dev"));
    m_protectedPaths.insert(normalizePath("/lib"));
    m_protectedPaths.insert(normalizePath("/lib64"));
    m_protectedPaths.insert(normalizePath("/bin"));
    m_protectedPaths.insert(normalizePath("/sbin"));
    m_protectedPaths.insert(normalizePath("/usr/bin"));
    m_protectedPaths.insert(normalizePath("/usr/sbin"));

    qDebug() << "Protected paths initialized:" << m_protectedPaths.count();
}

void FileAccessControl::logFileAccess(const FileAccessRecord& record)
{
    QMutexLocker locker(&m_mutex);

    // Keep last 5000 records in memory
    if (m_accessHistory.count() >= 5000) {
        m_accessHistory.removeFirst();
    }

    m_accessHistory.append(record);

    // Also log to PrivacyManager audit trail
    if (m_privacyManager) {
        QString accessTypeStr;
        switch (record.accessType) {
            case Read: accessTypeStr = "read"; break;
            case Write: accessTypeStr = "write"; break;
            case Execute: accessTypeStr = "execute"; break;
            case Delete: accessTypeStr = "delete"; break;
            default: accessTypeStr = "unknown";
        }

        m_privacyManager->logAccess(record.appId, PrivacyManager::FileAccess,
                                   QString("file: %1 %2 on %3 (%4)")
                                   .arg(record.action)
                                   .arg(accessTypeStr)
                                   .arg(record.filePath)
                                   .arg(record.reason));
    }
}

bool FileAccessControl::checkPathAgainstPolicy(const QString& appId, const QString& filePath,
                                             AccessType accessType)
{
    // Check blacklist first
    if (m_blacklists[appId].contains(filePath)) {
        return false;
    }

    // If whitelist exists, must be whitelisted
    if (!m_whitelists[appId].isEmpty()) {
        for (const QString& whitelistedPath : m_whitelists[appId]) {
            if (isPathUnderDirectory(filePath, whitelistedPath)) {
                return true;
            }
        }
        return false;
    }

    return true;
}

bool FileAccessControl::isPathUnderDirectory(const QString& filePath, const QString& directoryPath) const
{
    QString normFile = normalizePath(filePath);
    QString normDir = normalizePath(directoryPath);

    return normFile.startsWith(normDir + "/") || normFile == normDir;
}

QString FileAccessControl::normalizePath(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    QString normalized = fileInfo.absoluteFilePath();

    // Use forward slashes consistently
    normalized.replace("\\", "/");

    // Remove trailing slashes
    while (normalized.endsWith("/") && normalized.length() > 1) {
        normalized.chop(1);
    }

    return normalized;
}

QString FileAccessControl::expandUserPath(const QString& filePath) const
{
    if (filePath.startsWith("~")) {
        QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        return home + filePath.mid(1);
    }

    return filePath;
}
