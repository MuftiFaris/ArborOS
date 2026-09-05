#ifndef CAMERA_CONTROL_H
#define CAMERA_CONTROL_H

#include <QString>
#include <QObject>
#include <QMap>
#include <QList>
#include <QMutex>
#include "privacy-manager.h"

/**
 * CameraControl - Fine-grained camera/webcam access management
 * 
 * Provides:
 * - Per-application camera permission control
 * - Per-device camera control (multiple cameras)
 * - Global camera kill-switch
 * - Visual indicator (LED simulation if hardware LED unavailable)
 * - Last access timestamp tracking
 * - Audit trail of camera access
 */

class CameraControl : public QObject
{
    Q_OBJECT

public:
    explicit CameraControl(PrivacyManager* privacyManager, QObject* parent = nullptr);
    ~CameraControl();

    // Permission management
    bool requestCameraAccess(const QString& appId, const QString& appName, const QString& cameraId = "", const QString& reason = "");
    void setAppCameraPermission(const QString& appId, PrivacyManager::PermissionState state);
    PrivacyManager::PermissionState getAppCameraPermission(const QString& appId);

    // Global camera control
    void setGlobalCameraEnabled(bool enabled);
    bool isGlobalCameraEnabled() const;
    bool isCameraAvailable() const;

    // Real-time monitoring
    bool isCameraActive() const;
    QString getAppCurrentlyUsingCamera() const;
    QList<QString> getAppsWithCameraAccess();

    // Camera device management
    struct CameraDevice {
        QString id;
        QString name;
        QString type;  // "builtin", "usb", "external"
        QString resolution;  // "1920x1080", etc.
        bool isDefault;
        bool hasPhysicalLED;
        bool hasHardwareShutter;
    };

    QList<CameraDevice> getAvailableCameras();
    void setDefaultCamera(const QString& deviceId);
    QString getDefaultCamera() const;

    // Access tracking
    struct CameraAccessRecord {
        qint64 timestamp;
        QString appId;
        QString appName;
        QString cameraId;
        QString cameraName;
        QString action;  // "requested", "granted", "denied", "used", "blocked"
        qint64 durationMs;
        qint64 lastAccessTime;
    };

    QList<CameraAccessRecord> getCameraAccessHistory(int daysBack = 7);
    qint64 getLastAccessTime(const QString& appId);
    int getAccessCount(const QString& appId);

    // Visual indicators
    bool isIndicatorLEDActive() const;
    void activateIndicatorLED();
    void deactivateIndicatorLED();

public slots:
    void onAppPermissionChanged(const QString& appId, int categoryInt, int stateInt);
    void onGlobalSettingChanged(int settingType, bool enabled);
    void onCameraStateChanged(bool active, const QString& cameraId);

signals:
    // Emitted when camera permission requested
    void cameraPermissionRequested(const QString& appId, const QString& appName, const QString& cameraId, const QString& reason);

    // Emitted when camera state changes (active/inactive)
    void cameraStateChanged(bool isActive, const QString& cameraId);

    // Emitted when camera access is granted/denied
    void cameraAccessDecision(const QString& appId, bool granted);

    // Emitted when app attempts to use camera without permission
    void cameraAccessBlocked(const QString& appId, const QString& appName);

    // Emitted when global camera setting changes
    void globalCameraStateChanged(bool enabled);

    // Emitted when indicator LED state changes
    void indicatorLEDStateChanged(bool active);

private:
    PrivacyManager* m_privacyManager;
    QMutex m_mutex;
    
    bool m_cameraActive;
    QString m_cameraCurrentlyInUse;
    QString m_defaultCameraId;
    bool m_indicatorLEDActive;
    QMap<QString, CameraDevice> m_cameraDevices;
    QMap<QString, CameraAccessRecord> m_lastAccessPerApp;
    QList<CameraAccessRecord> m_accessHistory;
    
    // Helper methods
    void initializeCameraDevices();
    void monitorCameraState();
    void logCameraAccess(const CameraAccessRecord& record);
    void checkAndBlockUnauthorizedAccess();
};

#endif // CAMERA_CONTROL_H
