#ifndef CAMERA_CONTROL_H
#define CAMERA_CONTROL_H

#include "privacy-manager.h"
#include <QString>
#include <QObject>
#include <QList>

class CameraControl : public QObject
{
    Q_OBJECT

public:
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
    QList<QString> getAvailableCameras();

signals:
    void cameraAccessRequested(const QString& appId, const QString& reason);
    void cameraStatusChanged(bool inUse, const QString& appId);
    void permissionChanged(const QString& appId, int state);
    void cameraLEDStatusChanged(bool active);

private:
    PrivacyManager* m_privacyManager;
    bool m_globalEnabled;
};

#endif // CAMERA_CONTROL_H
