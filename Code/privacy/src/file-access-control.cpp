#include "file-access-control.h"

FileAccessControl::FileAccessControl(PrivacyManager* pm) : m_privacyManager(pm) {}

bool FileAccessControl::requestFileAccess(const QString& appId, const QString& path, bool write)
{
    if (!m_privacyManager) return false;
    return m_privacyManager->requestPermission(appId, PrivacyManager::Files, 
        QString("File %1: %2").arg(write ? "write" : "read", path));
}

void FileAccessControl::setSandboxDirectory(const QString& appId, const QString& path)
{
    // TODO: Configure AppArmor/SELinux rules
}

QStringList FileAccessControl::getSandboxDirectories(const QString& appId)
{
    return {QString("%1/.local/share/%2").arg(QDir::homePath(), appId)};
}

bool FileAccessControl::canAppAccessPath(const QString& appId, const QString& path)
{
    auto perm = m_privacyManager->getPermission(appId, PrivacyManager::Files);
    return (perm == PrivacyManager::AllowedAlways || perm == PrivacyManager::AllowedOnce);
}
