#ifndef MICROPHONE_CONTROL_H
#define MICROPHONE_CONTROL_H

#include "privacy-manager.h"
#include <QString>
#include <QObject>
#include <QList>

class MicrophoneControl : public QObject
{
    Q_OBJECT

public:
    explicit MicrophoneControl(PrivacyManager* pm, QObject* parent = nullptr);
    ~MicrophoneControl();

    // Permission management
    bool requestMicrophoneAccess(const QString& appId, const QString& reason = "");
    void setMicrophonePermission(const QString& appId, PrivacyManager::PermissionState state);
    PrivacyManager::PermissionState getMicrophonePermission(const QString& appId);

    // Global control
    void setGlobalMicrophoneEnabled(bool enabled);
    bool isGlobalMicrophoneEnabled() const;

    // Status
    bool isMicrophoneInUse() const;
    QString getCurrentMicrophoneUser() const;
    QList<QString> getAppsWithMicrophoneAccess();

    // Audio monitoring
    int getMicrophoneLevel() const;
    void enableAudioMonitoring(bool enable);

signals:
    void microphoneAccessRequested(const QString& appId, const QString& reason);
    void microphoneStatusChanged(bool inUse, const QString& appId);
    void audioLevelChanged(int level);
    void permissionChanged(const QString& appId, int state);

private:
    PrivacyManager* m_privacyManager;
    bool m_globalEnabled;
    bool m_monitoringEnabled;
};

#endif // MICROPHONE_CONTROL_H
