#include "camera-control.h"
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

CameraControl::CameraControl(PrivacyManager* pm, QObject* parent)
    : QObject(parent),
      m_privacyManager(pm),
      m_globalEnabled(true),
      m_ledActive(false)
{
    if (m_privacyManager) {
        m_globalEnabled = m_privacyManager->isGlobalCameraEnabled();
        connect(m_privacyManager, &PrivacyManager::permissionDecided,
                this, &CameraControl::onPermissionChangedSlot);
    }
}

CameraControl::~CameraControl()
{
}

bool CameraControl::requestCameraAccess(const QString& appId, const QString& reason)
{
    if (!m_privacyManager) return false;

    if (!m_globalEnabled) {
        emit cameraStatusChanged(false, appId);
        return false;
    }

    emit cameraAccessRequested(appId, reason);
    bool granted = m_privacyManager->requestPermission(appId, PrivacyManager::Camera, reason);
    if (granted) {
        registerActiveCameraStream(appId, "/dev/video0");
    }
    return granted;
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
    m_globalEnabled = enabled;
    if (m_privacyManager) {
        m_privacyManager->setGlobalCameraEnabled(enabled);
    }

    if (!enabled) {
        m_activeStreams.clear();
        updateLEDState();
        emit cameraStatusChanged(false, "");
    }

    emit globalCameraToggled(enabled);
}

bool CameraControl::isGlobalCameraEnabled() const
{
    if (!m_privacyManager) return m_globalEnabled;
    return m_privacyManager->isGlobalCameraEnabled();
}

bool CameraControl::isCameraInUse() const
{
    if (!m_globalEnabled) return false;

    if (!m_activeStreams.isEmpty()) return true;

    if (m_privacyManager) {
        auto activity = m_privacyManager->getCurrentActivity();
        for (const auto& record : activity) {
            if (record.category == PrivacyManager::Camera &&
                record.action == PrivacyManager::AuditUsed) {
                return true;
            }
        }
    }
    return false;
}

QString CameraControl::getCurrentCameraUser() const
{
    if (!m_globalEnabled) return "";

    if (!m_activeStreams.isEmpty()) {
        return m_activeStreams.keys().first();
    }

    if (m_privacyManager) {
        auto activity = m_privacyManager->getCurrentActivity();
        for (const auto& record : activity) {
            if (record.category == PrivacyManager::Camera &&
                record.action == PrivacyManager::AuditUsed) {
                return record.appId;
            }
        }
    }
    return "";
}

QList<QString> CameraControl::getAppsWithCameraAccess()
{
    if (!m_privacyManager) return QList<QString>();
    return m_privacyManager->getAppsWithPermission(PrivacyManager::Camera);
}

QList<CameraControl::CameraDevice> CameraControl::getAvailableCameraDevices()
{
    QList<CameraDevice> devices;

    // Enumerate V4L2 devices in /sys/class/video4linux or /dev/video*
    QDir videoDir("/sys/class/video4linux");
    if (videoDir.exists()) {
        QStringList entries = videoDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString& entry : entries) {
            if (entry.startsWith("video")) {
                CameraDevice dev;
                dev.devicePath = "/dev/" + entry;

                // Read device name from /sys/class/video4linux/<entry>/name
                QFile nameFile(videoDir.filePath(entry + "/name"));
                if (nameFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    dev.name = QString::fromUtf8(nameFile.readAll()).trimmed();
                } else {
                    dev.name = "Webcam " + entry;
                }

                dev.driver = "uvcvideo";
                dev.inUse = (isCameraInUse() && dev.devicePath == "/dev/video0");
                dev.activeAppId = dev.inUse ? getCurrentCameraUser() : "";

                devices.append(dev);
            }
        }
    }

    if (devices.isEmpty()) {
        // Fallback default camera entry if running in VM/test environment
        devices.append({"/dev/video0", "Integrated Webcam (USB)", "uvcvideo", isCameraInUse(), getCurrentCameraUser()});
    }

    return devices;
}

QList<QString> CameraControl::getAvailableCameras()
{
    QList<QString> names;
    auto devs = getAvailableCameraDevices();
    for (const auto& dev : devs) {
        names.append(dev.name + " (" + dev.devicePath + ")");
    }
    return names;
}

bool CameraControl::isCameraLEDActive() const
{
    return m_ledActive;
}

void CameraControl::registerActiveCameraStream(const QString& appId, const QString& devicePath)
{
    if (!m_globalEnabled) return;
    m_activeStreams[appId] = devicePath;
    updateLEDState();
    emit cameraStatusChanged(true, appId);
}

void CameraControl::unregisterActiveCameraStream(const QString& appId)
{
    if (m_activeStreams.contains(appId)) {
        m_activeStreams.remove(appId);
        updateLEDState();
        emit cameraStatusChanged(!m_activeStreams.isEmpty(), getCurrentCameraUser());
    }
}

void CameraControl::updateLEDState()
{
    bool shouldBeActive = isCameraInUse();
    if (m_ledActive != shouldBeActive) {
        m_ledActive = shouldBeActive;
        emit cameraLEDStatusChanged(m_ledActive);
    }
}

void CameraControl::onPermissionChangedSlot(const QString& appId, int categoryInt, int stateInt)
{
    if (categoryInt == (int)PrivacyManager::Camera) {
        if (stateInt == (int)PrivacyManager::Denied || stateInt == (int)PrivacyManager::SystemDenied) {
            unregisterActiveCameraStream(appId);
        }
        emit permissionChanged(appId, stateInt);
    }
}
