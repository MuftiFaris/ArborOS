#include "update-checker.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>
#include <QVersionNumber>

namespace Arbor::UpdateManager {

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent),
      networkManager(std::make_unique<QNetworkAccessManager>(this)),
      updateServerUrl("https://updates.arboros.dev/api/latest"),
      checkIntervalMinutes(1440),
      checkInProgress(false),
      updateScheduled(false)
{
    currentVersion = "0.5.0"; // Version should come from system
}

UpdateChecker::~UpdateChecker() = default;

void UpdateChecker::setUpdateServer(const QString &url)
{
    updateServerUrl = url;
}

void UpdateChecker::setCheckInterval(int minutes)
{
    checkIntervalMinutes = minutes;
}

QString UpdateChecker::getCurrentVersion() const
{
    return currentVersion;
}

bool UpdateChecker::hasAvailableUpdates()
{
    return !latestUpdate.version.isEmpty();
}

UpdateInfo UpdateChecker::getLatestUpdate() const
{
    return latestUpdate;
}

QList<UpdateInfo> UpdateChecker::getPendingUpdates() const
{
    if (hasAvailableUpdates()) {
        return QList<UpdateInfo>() << latestUpdate;
    }
    return QList<UpdateInfo>();
}

void UpdateChecker::checkForUpdates()
{
    if (checkInProgress) {
        return;
    }
    
    checkInProgress = true;
    emit checkStarted();
    
    // Make network request to update server
    QNetworkRequest request(QUrl(updateServerUrl));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    
    auto reply = networkManager->get(request);
    
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &UpdateChecker::onNetworkError);
    connect(reply, &QNetworkReply::downloadProgress,
            this, &UpdateChecker::onDownloadProgress);
}

void UpdateChecker::scheduleAutoCheck()
{
    // In production, would set up timer for periodic checks
    qDebug() << "Auto-check scheduled every" << checkIntervalMinutes << "minutes";
}

void UpdateChecker::cancelPendingCheck()
{
    checkInProgress = false;
}

bool UpdateChecker::isUpdateInProgress() const
{
    return checkInProgress;
}

bool UpdateChecker::isUpdateScheduled() const
{
    return updateScheduled;
}

int UpdateChecker::getDownloadSize() const
{
    return latestUpdate.downloadSize;
}

void UpdateChecker::onNetworkReply(QNetworkReply *reply)
{
    if (!reply) {
        return;
    }
    
    if (reply->error() == QNetworkReply::NoError) {
        const auto data = reply->readAll();
        const auto jsonDoc = QJsonDocument::fromJson(data);
        
        if (jsonDoc.isObject()) {
            UpdateInfo info;
            if (parseUpdateResponse(jsonDoc.toJson(), info)) {
                if (compareVersions(currentVersion, info.version)) {
                    latestUpdate = info;
                    emit updateAvailable(info);
                    emit checkFinished(true);
                } else {
                    emit checkFinished(false);
                }
            }
        }
    } else {
        emit checkFailed(reply->errorString());
        emit checkFinished(false);
    }
    
    checkInProgress = false;
    reply->deleteLater();
}

void UpdateChecker::onNetworkError(QNetworkReply::NetworkError error)
{
    qDebug() << "Network error:" << static_cast<int>(error);
    emit checkFailed("Network error");
    checkInProgress = false;
}

void UpdateChecker::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        const int percent = static_cast<int>(bytesReceived * 100 / bytesTotal);
        emit progressUpdated(percent);
    }
}

bool UpdateChecker::parseUpdateResponse(const QString &response, UpdateInfo &info)
{
    // Parse JSON response from update server
    const auto jsonDoc = QJsonDocument::fromJson(response.toUtf8());
    if (!jsonDoc.isObject()) {
        return false;
    }
    
    const auto obj = jsonDoc.object();
    
    info.version = obj.value("version").toString();
    info.releaseNotes = obj.value("releaseNotes").toString();
    info.releaseDate = QDateTime::fromString(obj.value("releaseDate").toString());
    info.downloadSize = obj.value("downloadSize").toInt();
    info.isSecurityUpdate = obj.value("isSecurityUpdate").toBool();
    
    const auto depsArray = obj.value("dependencies").toArray();
    for (const auto &dep : depsArray) {
        info.dependencies.append(dep.toString());
    }
    
    return !info.version.isEmpty();
}

bool UpdateChecker::compareVersions(const QString &current, const QString &available) const
{
    // Compare semantic versions
    const auto currentVer = QVersionNumber::fromString(current);
    const auto availableVer = QVersionNumber::fromString(available);
    
    return availableVer > currentVer;
}

QString UpdateChecker::getVersionFile() const
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/arbor-version";
}

bool UpdateChecker::saveUpdateInfo(const UpdateInfo &info) const
{
    // Save update info to persistent storage
    QFile file(getVersionFile());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(info.version.toUtf8());
        file.close();
        return true;
    }
    return false;
}

UpdateInfo UpdateChecker::loadUpdateInfo() const
{
    UpdateInfo info;
    QFile file(getVersionFile());
    if (file.open(QIODevice::ReadOnly)) {
        info.version = QString::fromUtf8(file.readAll());
        file.close();
    }
    return info;
}

} // namespace Arbor::UpdateManager
