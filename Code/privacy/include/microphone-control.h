#ifndef MICROPHONE_CONTROL_H
#define MICROPHONE_CONTROL_H

#include <QString>
#include <QObject>
#include <QMap>
#include <QList>
#include <QMutex>
#include "privacy-manager.h"

/**
 * MicrophoneControl - Fine-grained microphone access management
 * 
 * Provides:
 * - Per-application microphone permission control
 * - Global microphone kill-switch
 * - Real-time microphone state monitoring
 * - Audio level visualization
 * - Audit trail of microphone access
 */

class MicrophoneControl : public QObject
{
    Q_OBJECT

public:
    explicit MicrophoneControl(PrivacyManager* privacyManager, QObject* parent = nullptr);
    ~MicrophoneControl();

    // Permission management
    bool requestMicrophoneAccess(const QString& appId, const QString& appName, const QString& reason = "");
    void setAppMicrophonePermission(const QString& appId, PrivacyManager::PermissionState state);
    PrivacyManager::PermissionState getAppMicrophonePermission(const QString& appId);

    // Global microphone control
    void setGlobalMicrophoneEnabled(bool enabled);
    bool isGlobalMicrophoneEnabled() const;
    bool isMicrophoneAvailable() const;

    // Real-time monitoring
    bool isMicrophoneActive() const;
    float getCurrentAudioLevel() const;  // 0.0 to 1.0
    QString getAppCurrentlyUsingMicrophone() const;
    QList<QString> getAppsWithMicrophoneAccess();

    // Audio device management
    struct AudioDevice {
        QString id;
        QString name;
        QString type;  // "builtin", "usb", "hdmi"
        bool isDefault;
    };

    QList<AudioDevice> getAvailableMicrophones();
    void setDefaultMicrophone(const QString& deviceId);
    QString getDefaultMicrophone() const;

    // Audit trail specific to microphone
    struct MicrophoneAccessRecord {
        qint64 timestamp;
        QString appId;
        QString appName;
        QString action;  // "requested", "granted", "denied", "used", "blocked"
        QString audioDevice;
        float averageLevel;
        qint64 durationMs;
    };

    QList<MicrophoneAccessRecord> getMicrophoneAccessHistory(int daysBack = 7);

public slots:
    void onAppPermissionChanged(const QString& appId, int categoryInt, int stateInt);
    void onGlobalSettingChanged(int settingType, bool enabled);
    void onMicrophoneStateChanged(bool active);
    void updateAudioLevel(float level);

signals:
    // Emitted when microphone permission requested
    void microphonePermissionRequested(const QString& appId, const QString& appName, const QString& reason);

    // Emitted when microphone state changes (active/inactive)
    void microphoneStateChanged(bool isActive);

    // Emitted when microphone access is granted/denied
    void microphoneAccessDecision(const QString& appId, bool granted);

    // Emitted when audio level changes (for visualization)
    void audioLevelChanged(float level);

    // Emitted when app attempts to use microphone without permission
    void microphoneAccessBlocked(const QString& appId, const QString& appName);

    // Emitted when global microphone setting changes
    void globalMicrophoneStateChanged(bool enabled);

private:
    PrivacyManager* m_privacyManager;
    QMutex m_mutex;
    
    bool m_microphoneActive;
    float m_currentAudioLevel;
    QString m_appUsingMicrophone;
    QString m_defaultMicrophoneId;
    QMap<QString, AudioDevice> m_audioDevices;
    QList<MicrophoneAccessRecord> m_accessHistory;
    
    // Helper methods
    void initializeAudioDevices();
    void monitorMicrophoneState();
    void logMicrophoneAccess(const MicrophoneAccessRecord& record);
    void checkAndBlockUnauthorizedAccess();
};

#endif // MICROPHONE_CONTROL_H
