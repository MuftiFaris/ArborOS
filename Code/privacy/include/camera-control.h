#ifndef CAMERA_CONTROL_H
#define CAMERA_CONTROL_H

#include "privacy-manager.h"
#include <QString>
#include <QObject>
#include <QList>
#include <QMap>
#include <QDateTime>

class CameraControl : public QObject
{
    Q_OBJECT

public:
    struct CameraDevice {
        QString devicePath;  // e.g. "/dev/video0"
        QString name;        // e.g. "Integrated Camera"
        QString driver;      // e.g. "uvcvideo"
        bool inUse;
        QString activeAppId;
    };

    explicit CameraControl(PrivacyManager* pm, QObject* parent = nullptr);
    ~CameraControl();

    bool requestCameraAccess(const QString& appId, const QString& reason = "");
    void setCameraPermission(const QString& appId, PrivacyManager::PermissionState state);
    PrivacyManager::PermissionState getCameraPermission(const QString& appId);

    void setGlobalCameraEnabled(bool enabled);
    bool isGlobalCameraEnabled() const;

    bool isCameraInUse() const;
    QString getCurrentCameraUser() const;
    QList<QString> getAppsWithCameraAccess();
    QList<CameraDevice> getAvailableCameraDevices();
    QList<QString> getAvailableCameras();

    // LED simulation & status query
    bool isCameraLEDActive() const;
    void registerActiveCameraStream(const QString& appId, const QString& devicePath = "/dev/video0");
    void unregisterActiveCameraStream(const QString& appId);

public slots:
    void onPermissionChangedSlot(const QString& appId, int categoryInt, int stateInt);

signals:
    void cameraAccessRequested(const QString& appId, const QString& reason);
    void cameraStatusChanged(bool inUse, const QString& appId);
    void permissionChanged(const QString& appId, int state);
    void cameraLEDStatusChanged(bool active);
    void globalCameraToggled(bool enabled);

private:
    PrivacyManager* m_privacyManager;
    bool m_globalEnabled;
    bool m_ledActive;
    QMap<QString, QString> m_activeStreams; // appId -> devicePath

    void updateLEDState();
};

#endif // CAMERA_CONTROL_H
