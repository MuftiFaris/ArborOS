#include "microphone-control.h"
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QMutexLocker>

MicrophoneControl::MicrophoneControl(PrivacyManager* privacyManager, QObject* parent)
    : QObject(parent), m_privacyManager(privacyManager), m_microphoneActive(false),
      m_currentAudioLevel(0.0f), m_defaultMicrophoneId("")
{
    if (!m_privacyManager) {
        qCritical() << "MicrophoneControl: PrivacyManager is null";
        return;
    }

    // Initialize audio devices
    initializeAudioDevices();

    // Connect to privacy manager signals
    connect(m_privacyManager, QOverload<const QString&, int, int>::of(&PrivacyManager::permissionDecided),
            this, &MicrophoneControl::onAppPermissionChanged);
    connect(m_privacyManager, QOverload<int, bool>::of(&PrivacyManager::globalSettingChanged),
            this, &MicrophoneControl::onGlobalSettingChanged);

    // Start monitoring microphone state periodically
    QTimer* monitorTimer = new QTimer(this);
    connect(monitorTimer, &QTimer::timeout, this, [this]() {
        checkAndBlockUnauthorizedAccess();
    });
    monitorTimer->start(500);  // Check every 500ms

    qDebug() << "MicrophoneControl initialized";
}

MicrophoneControl::~MicrophoneControl()
{
    // Cleanup
}

bool MicrophoneControl::requestMicrophoneAccess(const QString& appId, const QString& appName, const QString& reason)
{
    QMutexLocker locker(&m_mutex);

    // Check global setting first
    if (!isGlobalMicrophoneEnabled()) {
        qDebug() << "Microphone access denied for" << appId << "- global microphone disabled";
        emit microphoneAccessDecision(appId, false);
        logMicrophoneAccess({QDateTime::currentMSecsSinceEpoch(), appId, appName,
                            "denied_global_disabled", m_defaultMicrophoneId, 0.0f, 0});
        return false;
    }

    // Request from PrivacyManager
    bool granted = m_privacyManager->requestPermission(appId, PrivacyManager::Microphone,
                                                       reason.isEmpty() ? "microphone_access" : reason);

    if (granted) {
        logMicrophoneAccess({QDateTime::currentMSecsSinceEpoch(), appId, appName,
                            "granted", m_defaultMicrophoneId, 0.0f, 0});
        emit microphonePermissionRequested(appId, appName, reason);
    } else {
        logMicrophoneAccess({QDateTime::currentMSecsSinceEpoch(), appId, appName,
                            "denied", m_defaultMicrophoneId, 0.0f, 0});
        emit microphoneAccessBlocked(appId, appName);
    }

    emit microphoneAccessDecision(appId, granted);
    return granted;
}

void MicrophoneControl::setAppMicrophonePermission(const QString& appId, PrivacyManager::PermissionState state)
{
    if (!m_privacyManager) {
        qWarning() << "PrivacyManager not available";
        return;
    }

    m_privacyManager->setPermission(appId, PrivacyManager::Microphone, state);
    qDebug() << "Microphone permission for" << appId << "set to" << (int)state;
}

PrivacyManager::PermissionState MicrophoneControl::getAppMicrophonePermission(const QString& appId)
{
    if (!m_privacyManager) {
        return PrivacyManager::Denied;
    }

    return m_privacyManager->getPermission(appId, PrivacyManager::Microphone);
}

void MicrophoneControl::setGlobalMicrophoneEnabled(bool enabled)
{
    if (!m_privacyManager) {
        qWarning() << "PrivacyManager not available";
        return;
    }

    m_privacyManager->setGlobalMicrophoneEnabled(enabled);
    emit globalMicrophoneStateChanged(enabled);
    qDebug() << "Global microphone" << (enabled ? "enabled" : "disabled");
}

bool MicrophoneControl::isGlobalMicrophoneEnabled() const
{
    if (!m_privacyManager) {
        return false;
    }

    return m_privacyManager->isGlobalMicrophoneEnabled();
}

bool MicrophoneControl::isMicrophoneAvailable() const
{
    QMutexLocker locker(&m_mutex);
    return !m_audioDevices.isEmpty();
}

bool MicrophoneControl::isMicrophoneActive() const
{
    QMutexLocker locker(&m_mutex);
    return m_microphoneActive;
}

float MicrophoneControl::getCurrentAudioLevel() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentAudioLevel;
}

QString MicrophoneControl::getAppCurrentlyUsingMicrophone() const
{
    QMutexLocker locker(&m_mutex);
    return m_appUsingMicrophone;
}

QList<QString> MicrophoneControl::getAppsWithMicrophoneAccess()
{
    if (!m_privacyManager) {
        return QList<QString>();
    }

    return m_privacyManager->getAppsWithPermission(PrivacyManager::Microphone);
}

QList<MicrophoneControl::AudioDevice> MicrophoneControl::getAvailableMicrophones()
{
    QMutexLocker locker(&m_mutex);
    return m_audioDevices.values();
}

void MicrophoneControl::setDefaultMicrophone(const QString& deviceId)
{
    QMutexLocker locker(&m_mutex);

    if (m_audioDevices.contains(deviceId)) {
        m_defaultMicrophoneId = deviceId;
        qDebug() << "Default microphone set to" << m_audioDevices[deviceId].name;
    } else {
        qWarning() << "Audio device not found:" << deviceId;
    }
}

QString MicrophoneControl::getDefaultMicrophone() const
{
    QMutexLocker locker(&m_mutex);
    return m_defaultMicrophoneId;
}

QList<MicrophoneControl::MicrophoneAccessRecord> MicrophoneControl::getMicrophoneAccessHistory(int daysBack)
{
    QMutexLocker locker(&m_mutex);

    QList<MicrophoneAccessRecord> filtered;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (daysBack * 24 * 60 * 60 * 1000);

    for (const auto& record : m_accessHistory) {
        if (record.timestamp >= cutoffTime) {
            filtered.append(record);
        }
    }

    return filtered;
}

void MicrophoneControl::onAppPermissionChanged(const QString& appId, int categoryInt, int stateInt)
{
    if (categoryInt != PrivacyManager::Microphone) {
        return;  // Not a microphone permission change
    }

    PrivacyManager::PermissionState state = (PrivacyManager::PermissionState)stateInt;
    qDebug() << "Microphone permission changed for" << appId << "- new state:" << (int)state;
}

void MicrophoneControl::onGlobalSettingChanged(int settingType, bool enabled)
{
    if (settingType != PrivacyManager::Microphone) {
        return;  // Not a microphone setting change
    }

    emit globalMicrophoneStateChanged(enabled);
    qDebug() << "Global microphone setting changed:" << enabled;
}

void MicrophoneControl::onMicrophoneStateChanged(bool active)
{
    {
        QMutexLocker locker(&m_mutex);
        m_microphoneActive = active;
    }

    emit microphoneStateChanged(active);
}

void MicrophoneControl::updateAudioLevel(float level)
{
    if (level < 0.0f) level = 0.0f;
    if (level > 1.0f) level = 1.0f;

    {
        QMutexLocker locker(&m_mutex);
        if (m_currentAudioLevel != level) {
            m_currentAudioLevel = level;
        }
    }

    emit audioLevelChanged(level);
}

void MicrophoneControl::initializeAudioDevices()
{
    QMutexLocker locker(&m_mutex);

    // Initialize with built-in microphone (in real implementation, would enumerate PulseAudio/ALSA devices)
    AudioDevice builtin;
    builtin.id = "builtin-microphone";
    builtin.name = "Built-in Microphone";
    builtin.type = "builtin";
    builtin.isDefault = true;

    m_audioDevices[builtin.id] = builtin;
    m_defaultMicrophoneId = builtin.id;

    qDebug() << "Audio devices initialized - found" << m_audioDevices.count() << "microphone(s)";
}

void MicrophoneControl::logMicrophoneAccess(const MicrophoneAccessRecord& record)
{
    QMutexLocker locker(&m_mutex);

    // Keep last 1000 records in memory
    if (m_accessHistory.count() >= 1000) {
        m_accessHistory.removeFirst();
    }

    m_accessHistory.append(record);

    // Also log to PrivacyManager audit trail
    if (m_privacyManager) {
        m_privacyManager->logAccess(record.appId, PrivacyManager::Microphone,
                                   QString("microphone: %1 (%2ms, level: %3)")
                                   .arg(record.action)
                                   .arg(record.durationMs)
                                   .arg(record.averageLevel));
    }
}

void MicrophoneControl::checkAndBlockUnauthorizedAccess()
{
    // In real implementation, would check if any process is using microphone
    // and verify it has proper permissions
    // For now, this is a placeholder for the monitoring loop
}
