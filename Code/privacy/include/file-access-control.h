#ifndef FILE_ACCESS_CONTROL_H
#define FILE_ACCESS_CONTROL_H

#include "privacy-manager.h"
#include <QString>
#include <QStringList>
#include <QSet>
#include <QMap>

class FileAccessControl
{
public:
    enum PathAccessLevel {
        AccessDenied = 0,
        AccessReadOnly = 1,
        AccessReadWrite = 2,
        AccessAskUser = 3
    };

    struct SandboxRule {
        QString appId;
        QString defaultDataDir;
        QStringList whitelistedPaths;
        QStringList blacklistedPaths;
        bool allowHomeAccess;
    };

    explicit FileAccessControl(PrivacyManager* pm);
    ~FileAccessControl();

    bool requestFileAccess(const QString& appId, const QString& path, bool write = false);
    void setSandboxDirectory(const QString& appId, const QString& path);
    QStringList getSandboxDirectories(const QString& appId);

    // Path evaluation
    bool canAppAccessPath(const QString& appId, const QString& path, bool write = false);
    PathAccessLevel evaluatePathAccess(const QString& appId, const QString& path);
    bool isProtectedSystemPath(const QString& path) const;

    // Whitelist / Blacklist management
    void addPathToWhitelist(const QString& appId, const QString& path);
    void removePathFromWhitelist(const QString& appId, const QString& path);
    QStringList getAppWhitelist(const QString& appId) const;

    // Profile export
    QString generateAppArmorProfile(const QString& appId) const;

private:
    PrivacyManager* m_privacyManager;
    QMap<QString, SandboxRule> m_appRules;
    QStringList m_systemRestrictedPaths;

    void initializeSystemRestrictedPaths();
    QString canonicalizePath(const QString& path) const;
};

#endif // FILE_ACCESS_CONTROL_H
