#include "microphone-control.h"

MicrophoneControl::MicrophoneControl(PrivacyManager* pm, QObject* parent)
    : QObject(parent), m_privacyManager(pm), m_globalEnabled(true), m_monitoringEnabled(false)
{
}

MicrophoneControl::~MicrophoneControl()
{
}

bool MicrophoneControl::requestMicrophoneAccess(const QString& appId, const QString& reason)
{
    if (!m_privacyManager) return false;
    return m_privacyManager->requestPermission(appId, PrivacyManager::Microphone, reason);
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
    if (!m_privacyManager) return;
    m_globalEnabled = enabled;
    m_privacyManager->setGlobalMicrophoneEnabled(enabled);
}

bool MicrophoneControl::isGlobalMicrophoneEnabled() const
{
    if (!m_privacyManager) return true;
    return m_privacyManager->isGlobalMicrophoneEnabled();
}

bool MicrophoneControl::isMicrophoneInUse() const
{
    if (!m_privacyManager) return false;
    return !m_privacyManager->getCurrentActivity().isEmpty();
}

QString MicrophoneControl::getCurrentMicrophoneUser() const
{
    auto activity = m_privacyManager->getCurrentActivity();
    for (const auto& record : activity) {
        if (record.category == PrivacyManager::Microphone && record.action == PrivacyManager::AuditUsed) {
            return record.appId;
        }
    }
    return "";
}

QList<QString> MicrophoneControl::getAppsWithMicrophoneAccess()
{
    if (!m_privacyManager) return QList<QString>();
    return m_privacyManager->getAppsWithPermission(PrivacyManager::Microphone);
}

int MicrophoneControl::getMicrophoneLevel() const
{
    // TODO: Integrate with PipeWire for actual audio level
    return 0;
}

void MicrophoneControl::enableAudioMonitoring(bool enable)
{
    m_monitoringEnabled = enable;
}
