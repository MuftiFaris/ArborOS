#include "camera-control.h"
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QMutexLocker>

CameraControl::CameraControl(PrivacyManager* privacyManager, QObject* parent)
    : QObject(parent), m_privacyManager(privacyManager), m_cameraActive(false),
      m_defaultCameraId(""), m_indicatorLEDActive(false)
{
    if (!m_privacyManager) {
        qCritical() << "CameraControl: PrivacyManager is null";
        return;
    }

    // Initialize camera devices
    initializeCameraDevices();

    // Connect to privacy manager signals
    connect(m_privacyManager, QOverload<const QString&, int, int>::of(&PrivacyManager::permissionDecided),
            this, &CameraControl::onAppPermissionChanged);
    connect(m_privacyManager, QOverload<int, bool>::of(&PrivacyManager::globalSettingChanged),
            this, &CameraControl::onGlobalSettingChanged);

    // Start monitoring camera state periodically
    QTimer* monitorTimer = new QTimer(this);
    connect(monitorTimer, &QTimer::timeout, this, [this]() {
        checkAndBlockUnauthorizedAccess();
    });
    monitorTimer->start(500);  // Check every 500ms

    qDebug() << "CameraControl initialized";
}

CameraControl::~CameraControl()
{
    // Deactivate indicator LED on shutdown
    deactivateIndicatorLED();
}

bool CameraControl::requestCameraAccess(const QString& appId, const QString& appName, 
                                       const QString& cameraId, const QString& reason)
{
    QMutexLocker locker(&m_mutex);

    // Check global setting first
    if (!isGlobalCameraEnabled()) {
        qDebug() << "Camera access denied for" << appId << "- global camera disabled";
        emit cameraAccessDecision(appId, false);
        logCameraAccess({QDateTime::currentMSecsSinceEpoch(), appId, appName,
                        cameraId.isEmpty() ? m_defaultCameraId : cameraId,
                        m_cameraDevices.value(m_defaultCameraId).name,
                        "denied_global_disabled", 0, 0});
        return false;
    }

    // Request from PrivacyManager
    bool granted = m_privacyManager->requestPermission(appId, PrivacyManager::Camera,
                                                       reason.isEmpty() ? "camera_access" : reason);

    if (granted) {
        logCameraAccess({QDateTime::currentMSecsSinceEpoch(), appId, appName,
                        cameraId.isEmpty() ? m_defaultCameraId : cameraId,
                        m_cameraDevices.value(m_defaultCameraId).name,
                        "granted", 0, QDateTime::currentMSecsSinceEpoch()});
        activateIndicatorLED();
    } else {
        logCameraAccess({QDateTime::currentMSecsSinceEpoch(), appId, appName,
                        cameraId.isEmpty() ? m_defaultCameraId : cameraId,
                        m_cameraDevices.value(m_defaultCameraId).name,
                        "denied", 0, 0});
        emit cameraAccessBlocked(appId, appName);
    }

    emit cameraAccessDecision(appId, granted);
    return granted;
}

void CameraControl::setAppCameraPermission(const QString& appId, PrivacyManager::PermissionState state)
{
    if (!m_privacyManager) {
        qWarning() << "PrivacyManager not available";
        return;
    }

    m_privacyManager->setPermission(appId, PrivacyManager::Camera, state);
    qDebug() << "Camera permission for" << appId << "set to" << (int)state;
}

PrivacyManager::PermissionState CameraControl::getAppCameraPermission(const QString& appId)
{
    if (!m_privacyManager) {
        return PrivacyManager::Denied;
    }

    return m_privacyManager->getPermission(appId, PrivacyManager::Camera);
}

void CameraControl::setGlobalCameraEnabled(bool enabled)
{
    if (!m_privacyManager) {
        qWarning() << "PrivacyManager not available";
        return;
    }

    m_privacyManager->setGlobalCameraEnabled(enabled);
    emit globalCameraStateChanged(enabled);
    
    if (!enabled) {
        deactivateIndicatorLED();
    }

    qDebug() << "Global camera" << (enabled ? "enabled" : "disabled");
}

bool CameraControl::isGlobalCameraEnabled() const
{
    if (!m_privacyManager) {
        return false;
    }

    return m_privacyManager->isGlobalCameraEnabled();
}

bool CameraControl::isCameraAvailable() const
{
    QMutexLocker locker(&m_mutex);
    return !m_cameraDevices.isEmpty();
}

bool CameraControl::isCameraActive() const
{
    QMutexLocker locker(&m_mutex);
    return m_cameraActive;
}

QString CameraControl::getAppCurrentlyUsingCamera() const
{
    QMutexLocker locker(&m_mutex);
    return m_cameraCurrentlyInUse;
}

QList<QString> CameraControl::getAppsWithCameraAccess()
{
    if (!m_privacyManager) {
        return QList<QString>();
    }

    return m_privacyManager->getAppsWithPermission(PrivacyManager::Camera);
}

QList<CameraControl::CameraDevice> CameraControl::getAvailableCameras()
{
    QMutexLocker locker(&m_mutex);
    return m_cameraDevices.values();
}

void CameraControl::setDefaultCamera(const QString& deviceId)
{
    QMutexLocker locker(&m_mutex);

    if (m_cameraDevices.contains(deviceId)) {
        m_defaultCameraId = deviceId;
        qDebug() << "Default camera set to" << m_cameraDevices[deviceId].name;
    } else {
        qWarning() << "Camera device not found:" << deviceId;
    }
}

QString CameraControl::getDefaultCamera() const
{
    QMutexLocker locker(&m_mutex);
    return m_defaultCameraId;
}

QList<CameraControl::CameraAccessRecord> CameraControl::getCameraAccessHistory(int daysBack)
{
    QMutexLocker locker(&m_mutex);

    QList<CameraAccessRecord> filtered;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000);

    for (const auto& record : m_accessHistory) {
        if (record.timestamp >= cutoffTime) {
            filtered.append(record);
        }
    }

    return filtered;
}

qint64 CameraControl::getLastAccessTime(const QString& appId)
{
    QMutexLocker locker(&m_mutex);

    if (m_lastAccessPerApp.contains(appId)) {
        return m_lastAccessPerApp[appId].lastAccessTime;
    }

    return 0;
}

int CameraControl::getAccessCount(const QString& appId)
{
    QMutexLocker locker(&m_mutex);

    int count = 0;
    for (const auto& record : m_accessHistory) {
        if (record.appId == appId && record.action == "used") {
            count++;
        }
    }

    return count;
}

bool CameraControl::isIndicatorLEDActive() const
{
    QMutexLocker locker(&m_mutex);
    return m_indicatorLEDActive;
}

void CameraControl::activateIndicatorLED()
{
    {
        QMutexLocker locker(&m_mutex);
        if (m_indicatorLEDActive) {
            return;  // Already active
        }
        m_indicatorLEDActive = true;
    }

    emit indicatorLEDStateChanged(true);
    qDebug() << "Camera indicator LED activated";
}

void CameraControl::deactivateIndicatorLED()
{
    {
        QMutexLocker locker(&m_mutex);
        if (!m_indicatorLEDActive) {
            return;  // Already inactive
        }
        m_indicatorLEDActive = false;
    }

    emit indicatorLEDStateChanged(false);
    qDebug() << "Camera indicator LED deactivated";
}

void CameraControl::onAppPermissionChanged(const QString& appId, int categoryInt, int stateInt)
{
    if (categoryInt != PrivacyManager::Camera) {
        return;  // Not a camera permission change
    }

    PrivacyManager::PermissionState state = (PrivacyManager::PermissionState)stateInt;
    qDebug() << "Camera permission changed for" << appId << "- new state:" << (int)state;
}

void CameraControl::onGlobalSettingChanged(int settingType, bool enabled)
{
    if (settingType != PrivacyManager::Camera) {
        return;  // Not a camera setting change
    }

    emit globalCameraStateChanged(enabled);
    qDebug() << "Global camera setting changed:" << enabled;
}

void CameraControl::onCameraStateChanged(bool active, const QString& cameraId)
{
    {
        QMutexLocker locker(&m_mutex);
        m_cameraActive = active;
        if (active) {
            m_cameraCurrentlyInUse = cameraId;
        } else {
            m_cameraCurrentlyInUse = "";
        }
    }

    emit cameraStateChanged(active, cameraId);

    if (active) {
        activateIndicatorLED();
    } else {
        deactivateIndicatorLED();
    }
}

void CameraControl::initializeCameraDevices()
{
    QMutexLocker locker(&m_mutex);

    // Initialize with built-in camera (in real implementation, would enumerate V4L2 devices)
    CameraDevice builtin;
    builtin.id = "builtin-camera";
    builtin.name = "Built-in Camera";
    builtin.type = "builtin";
    builtin.resolution = "1920x1080";
    builtin.isDefault = true;
    builtin.hasPhysicalLED = false;  // Most laptops don't have hardware LED
    builtin.hasHardwareShutter = false;

    m_cameraDevices[builtin.id] = builtin;
    m_defaultCameraId = builtin.id;

    qDebug() << "Camera devices initialized - found" << m_cameraDevices.count() << "camera(s)";
}

void CameraControl::logCameraAccess(const CameraAccessRecord& record)
{
    QMutexLocker locker(&m_mutex);

    // Keep last 1000 records in memory
    if (m_accessHistory.count() >= 1000) {
        m_accessHistory.removeFirst();
    }

    m_accessHistory.append(record);

    // Update last access time
    m_lastAccessPerApp[record.appId] = record;

    // Also log to PrivacyManager audit trail
    if (m_privacyManager) {
        m_privacyManager->logAccess(record.appId, PrivacyManager::Camera,
                                   QString("camera: %1 on %2 (%3ms)")
                                   .arg(record.action)
                                   .arg(record.cameraName)
                                   .arg(record.durationMs));
    }
}

void CameraControl::checkAndBlockUnauthorizedAccess()
{
    // In real implementation, would check if any process is using camera
    // and verify it has proper permissions
    // For now, this is a placeholder for the monitoring loop
}
