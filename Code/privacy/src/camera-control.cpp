#include "camera-control.h"

CameraControl::CameraControl(PrivacyManager* pm, QObject* parent)
    : QObject(parent), m_privacyManager(pm), m_globalEnabled(true)
{
}

CameraControl::~CameraControl()
{
}

bool CameraControl::requestCameraAccess(const QString& appId, const QString& reason)
{
    if (!m_privacyManager) return false;
    return m_privacyManager->requestPermission(appId, PrivacyManager::Camera, reason);
}

void CameraControl::setCameraPermission(const QString& appId, PrivacyManager::PermissionState state)
{
    if (!m_privacyManager) return;
    m_privacyManager->setPermission(appId, PrivacyManager::Camera, state);
    emit permissionChanged(appId, (int)state);
}

PrivacyManager::PermissionState CameraControl::getCameraPermission(const QString& appId)
{
    if (!m_privacyManager) return PrivacyManager::Denied;
    return m_privacyManager->getPermission(appId, PrivacyManager::Camera);
}

void CameraControl::setGlobalCameraEnabled(bool enabled)
{
    if (!m_privacyManager) return;
    m_globalEnabled = enabled;
    m_privacyManager->setGlobalCameraEnabled(enabled);
    emit cameraLEDStatusChanged(enabled);
}

bool CameraControl::isGlobalCameraEnabled() const
{
    if (!m_privacyManager) return true;
    return m_privacyManager->isGlobalCameraEnabled();
}

bool CameraControl::isCameraInUse() const
{
    if (!m_privacyManager) return false;
    auto activity = m_privacyManager->getCurrentActivity();
    for (const auto& record : activity) {
        if (record.category == PrivacyManager::Camera && record.action == PrivacyManager::AuditUsed) {
            return true;
        }
    }
    return false;
}

QString CameraControl::getCurrentCameraUser() const
{
    if (!m_privacyManager) return "";
    auto activity = m_privacyManager->getCurrentActivity();
    for (const auto& record : activity) {
        if (record.category == PrivacyManager::Camera && record.action == PrivacyManager::AuditUsed) {
            return record.appId;
        }
    }
    return "";
}

QList<QString> CameraControl::getAppsWithCameraAccess()
{
    if (!m_privacyManager) return QList<QString>();
    return m_privacyManager->getAppsWithPermission(PrivacyManager::Camera);
}

QList<QString> CameraControl::getAvailableCameras()
{
    // TODO: Enumerate /dev/video* or use libv4l2
    return {"default"};
}
