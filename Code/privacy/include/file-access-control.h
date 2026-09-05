#ifndef FILE_ACCESS_CONTROL_H
#define FILE_ACCESS_CONTROL_H

#include "privacy-manager.h"
#include <QString>
#include <QStringList>

class FileAccessControl
{
public:
    FileAccessControl(PrivacyManager* pm);
    
    bool requestFileAccess(const QString& appId, const QString& path, bool write = false);
    void setSandboxDirectory(const QString& appId, const QString& path);
    QStringList getSandboxDirectories(const QString& appId);
    
    bool canAppAccessPath(const QString& appId, const QString& path);
    
private:
    PrivacyManager* m_privacyManager;
};

#endif
