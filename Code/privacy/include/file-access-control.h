#ifndef FILE_ACCESS_CONTROL_H
#define FILE_ACCESS_CONTROL_H

#include <QString>
#include <QObject>
#include <QMap>
#include <QList>
#include <QMutex>
#include <QSet>
#include "privacy-manager.h"

/**
 * FileAccessControl - Fine-grained file system access management
 * 
 * Provides:
 * - Sandbox directory concept (isolated app storage)
 * - Protected system directories (deny by default)
 * - Per-application whitelist/blacklist
 * - Access request dialogs with path preview
 * - Audit trail of file access attempts
 */

class FileAccessControl : public QObject
{
    Q_OBJECT

public:
    explicit FileAccessControl(PrivacyManager* privacyManager, QObject* parent = nullptr);
    ~FileAccessControl();

    // Access categories
    enum AccessType {
        Read = 0,
        Write = 1,
        Execute = 2,
        Delete = 3
    };

    // Request file access with permission check
    bool requestFileAccess(const QString& appId, const QString& appName, 
                          const QString& filePath, AccessType accessType, 
                          const QString& reason = "");
    
    // Whitelist/blacklist management
    void addToWhitelist(const QString& appId, const QString& directoryPath);
    void removeFromWhitelist(const QString& appId, const QString& directoryPath);
    QList<QString> getWhitelist(const QString& appId);

    void addToBlacklist(const QString& appId, const QString& directoryPath);
    void removeFromBlacklist(const QString& appId, const QString& directoryPath);
    QList<QString> getBlacklist(const QString& appId);

    // Sandbox directory management (isolated per-app storage)
    QString getSandboxDirectory(const QString& appId);
    QString createSandboxDirectory(const QString& appId);
    bool isSandboxDirectory(const QString& dirPath) const;
    QList<QString> getAllSandboxDirectories();

    // System protection (always denied)
    bool isSystemProtectedPath(const QString& filePath) const;
    QList<QString> getProtectedPaths() const;

    // Home directory access control
    bool isHomeDirectory(const QString& filePath) const;
    bool isHomeSubdirectory(const QString& filePath) const;

    // User approval for special directories
    void requestSpecialDirectoryAccess(const QString& appId, const QString& appName, 
                                      const QString& directoryPath);
    void approveSpecialDirectoryAccess(const QString& appId, const QString& directoryPath);
    void denySpecialDirectoryAccess(const QString& appId, const QString& directoryPath);

    // Access verification
    bool canAccessPath(const QString& appId, const QString& filePath, AccessType accessType);

    // Audit trail
    struct FileAccessRecord {
        qint64 timestamp;
        QString appId;
        QString appName;
        QString filePath;
        int accessType;  // Read, Write, Execute, Delete
        QString action;  // "requested", "granted", "denied", "attempted"
        QString reason;
        bool allowed;
    };

    QList<FileAccessRecord> getFileAccessHistory(int daysBack = 7);
    int getTotalAccessAttempts(const QString& appId, int daysBack = 7);

    // Statistics
    int getAccessDenialCount(const QString& appId, int daysBack = 7);
    QMap<QString, int> getAccessStatistics(int daysBack = 7);

public slots:
    void onAppPermissionChanged(const QString& appId, int categoryInt, int stateInt);

signals:
    // Emitted when file access is requested
    void fileAccessRequested(const QString& appId, const QString& appName, 
                            const QString& filePath, int accessType);

    // Emitted when file access is granted/denied
    void fileAccessDecision(const QString& appId, const QString& filePath, 
                           int accessType, bool allowed);

    // Emitted when app attempts denied access
    void fileAccessBlocked(const QString& appId, const QString& appName, 
                          const QString& filePath);

    // Emitted when special directory access is requested
    void specialDirectoryAccessRequested(const QString& appId, const QString& appName, 
                                        const QString& directoryPath);

    // Emitted when whitelist/blacklist changes
    void accessPolicyChanged(const QString& appId);

private:
    PrivacyManager* m_privacyManager;
    QMutex m_mutex;

    // Per-app whitelists/blacklists
    QMap<QString, QSet<QString>> m_whitelists;
    QMap<QString, QSet<QString>> m_blacklists;
    QMap<QString, QSet<QString>> m_specialApprovals;

    // Access history
    QList<FileAccessRecord> m_accessHistory;

    // System protected paths (never allow access)
    QSet<QString> m_protectedPaths;

    // Sandbox base directory
    QString m_sandboxBaseDir;

    // Helper methods
    void initializeProtectedPaths();
    void logFileAccess(const FileAccessRecord& record);
    bool checkPathAgainstPolicy(const QString& appId, const QString& filePath, AccessType accessType);
    bool isPathUnderDirectory(const QString& filePath, const QString& directoryPath) const;
    QString normalizePath(const QString& filePath) const;
    QString expandUserPath(const QString& filePath) const;
};

#endif // FILE_ACCESS_CONTROL_H
