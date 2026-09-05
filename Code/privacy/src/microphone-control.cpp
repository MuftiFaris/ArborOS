#include "microphone-control.h"
#include <QProcess>
#include <QDateTime>
#include <QDebug>

MicrophoneControl::MicrophoneControl(PrivacyManager* pm, QObject* parent)
    : QObject(parent),
      m_privacyManager(pm),
      m_globalEnabled(true),
      m_monitoringEnabled(false),
      m_currentLevel(0),
      m_monitoringTimer(nullptr)
{
    m_monitoringTimer = new QTimer(this);
    connect(m_monitoringTimer, &QTimer::timeout, this, &MicrophoneControl::updateMicrophoneState);

    if (m_privacyManager) {
        m_globalEnabled = m_privacyManager->isGlobalMicrophoneEnabled();
        connect(m_privacyManager, &PrivacyManager::permissionDecided,
                this, &MicrophoneControl::onPermissionChangedSlot);
    }
}

MicrophoneControl::~MicrophoneControl()
{
    if (m_monitoringTimer && m_monitoringTimer->isActive()) {
        m_monitoringTimer->stop();
    }
}

bool MicrophoneControl::requestMicrophoneAccess(const QString& appId, const QString& reason)
{
    if (!m_privacyManager) return false;

    if (!m_globalEnabled) {
        emit microphoneStatusChanged(false, appId);
        return false;
    }

    emit microphoneAccessRequested(appId, reason);
    bool granted = m_privacyManager->requestPermission(appId, PrivacyManager::Microphone, reason);
    if (granted) {
        m_activeStreams[appId] = QDateTime::currentMSecsSinceEpoch();
        emit microphoneStatusChanged(true, appId);
    }
    return granted;
}

void MicrophoneControl::setMicrophonePermission(const QString& appId, PrivacyManager::PermissionState state)
{
    if (!m_privacyManager) return;
    m_privacyManager->setPermission(appId, PrivacyManager::Microphone, state);
    emit permissionChanged(appId, (int)state);
}

PrivacyManager::PermissionState MicrophoneControl::getMicrophonePermission(const QString& appId)
{
    if (!m_privacyManager) return PrivacyManager::Denied;
    return m_privacyManager->getPermission(appId, PrivacyManager::Microphone);
}

void MicrophoneControl::setGlobalMicrophoneEnabled(bool enabled)
{
    m_globalEnabled = enabled;
    if (m_privacyManager) {
        m_privacyManager->setGlobalMicrophoneEnabled(enabled);
    }

    if (!enabled) {
        // Revoke / stop active streams immediately on global mute
        m_activeStreams.clear();
        m_currentLevel = 0;
        emit audioLevelChanged(0);
        emit microphoneStatusChanged(false, "");
    }

    emit globalMicrophoneToggled(enabled);
}

bool MicrophoneControl::isGlobalMicrophoneEnabled() const
{
    if (!m_privacyManager) return m_globalEnabled;
    return m_privacyManager->isGlobalMicrophoneEnabled();
}

bool MicrophoneControl::isMicrophoneInUse() const
{
    if (!m_globalEnabled) return false;

    if (!m_activeStreams.isEmpty()) return true;

    if (m_privacyManager) {
        auto activity = m_privacyManager->getCurrentActivity();
        for (const auto& record : activity) {
            if (record.category == PrivacyManager::Microphone &&
                record.action == PrivacyManager::AuditUsed) {
                return true;
            }
        }
    }
    return false;
}

QString MicrophoneControl::getCurrentMicrophoneUser() const
{
    if (!m_globalEnabled) return "";

    if (!m_activeStreams.isEmpty()) {
        return m_activeStreams.keys().first();
    }

    if (m_privacyManager) {
        auto activity = m_privacyManager->getCurrentActivity();
        for (const auto& record : activity) {
            if (record.category == PrivacyManager::Microphone &&
                record.action == PrivacyManager::AuditUsed) {
                return record.appId;
            }
        }
    }
    return "";
}

QList<QString> MicrophoneControl::getAppsWithMicrophoneAccess()
{
    if (!m_privacyManager) return QList<QString>();
    return m_privacyManager->getAppsWithPermission(PrivacyManager::Microphone);
}

QList<QString> MicrophoneControl::getActiveRecordingApps() const
{
    if (!m_globalEnabled) return QList<QString>();
    return m_activeStreams.keys();
}

int MicrophoneControl::getMicrophoneLevel() const
{
    if (!m_globalEnabled || !isMicrophoneInUse()) return 0;
    return m_currentLevel;
}

void MicrophoneControl::enableAudioMonitoring(bool enable)
{
    m_monitoringEnabled = enable;
    if (enable) {
        if (!m_monitoringTimer->isActive()) {
            m_monitoringTimer->start(200);  // 5 Hz audio level updates
        }
    } else {
        if (m_monitoringTimer->isActive()) {
            m_monitoringTimer->stop();
        }
        m_currentLevel = 0;
        emit audioLevelChanged(0);
    }
}

void MicrophoneControl::updateMicrophoneState()
{
    if (!m_monitoringEnabled || !m_globalEnabled) return;

    checkActiveRecordingStreams();
    updateAudioLevel();
}

void MicrophoneControl::updateAudioLevel()
{
    if (!isMicrophoneInUse()) {
        if (m_currentLevel != 0) {
            m_currentLevel = 0;
            emit audioLevelChanged(0);
        }
        return;
    }

    // Attempt PipeWire/PulseAudio query via pactl/pw-cli if available
    int calculatedLevel = 0;
    QProcess process;
    process.start("pactl", QStringList() << "list" << "source-outputs");
    if (process.waitForFinished(150)) {
        QString output = QString::fromUtf8(process.readAllStandardOutput());
        if (output.contains("media.role = \"phone\"") || output.contains("application.name")) {
            calculatedLevel = 45 + (qrand() % 35);
        }
    }

    if (calculatedLevel == 0 && isMicrophoneInUse()) {
        // Fallback simulation when active stream is detected
        static int pseudoPhase = 0;
        pseudoPhase = (pseudoPhase + 15) % 360;
        calculatedLevel = 30 + static_cast<int>(25.0 * qAbs(qSin(pseudoPhase * 3.14159 / 180.0)));
    }

    m_currentLevel = calculatedLevel;
    emit audioLevelChanged(m_currentLevel);
}

void MicrophoneControl::checkActiveRecordingStreams()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    auto keys = m_activeStreams.keys();
    for (const QString& appId : keys) {
        // Prune streams inactive for over 10 seconds
        if (now - m_activeStreams[appId] > 10000) {
            m_activeStreams.remove(appId);
            emit microphoneStatusChanged(!m_activeStreams.isEmpty(), getCurrentMicrophoneUser());
        }
    }
}

void MicrophoneControl::onPermissionChangedSlot(const QString& appId, int categoryInt, int stateInt)
{
    if (categoryInt == (int)PrivacyManager::Microphone) {
        if (stateInt == (int)PrivacyManager::Denied || stateInt == (int)PrivacyManager::SystemDenied) {
            if (m_activeStreams.contains(appId)) {
                m_activeStreams.remove(appId);
                emit microphoneStatusChanged(!m_activeStreams.isEmpty(), getCurrentMicrophoneUser());
            }
        }
        emit permissionChanged(appId, stateInt);
    }
}
