#ifndef ARBOR_UPDATE_MANAGER_UPDATE_CHECKER_H
#define ARBOR_UPDATE_MANAGER_UPDATE_CHECKER_H

#include <QString>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDateTime>
#include <memory>

namespace Arbor::UpdateManager {

/**
 * @struct UpdateInfo
 * @brief Information about available system updates
 */
struct UpdateInfo {
    QString version;           ///< Target version
    QString releaseNotes;      ///< Release notes
    QDateTime releaseDate;     ///< Release date
    int downloadSize;          ///< Size in MB
    bool isSecurityUpdate;     ///< Security patch indicator
    QStringList dependencies;  ///< Required updates
};

/**
 * @class UpdateChecker
 * @brief Checks for and manages system updates
 *
 * Performs:
 * - Version checking against update server
 * - Delta download calculation
 * - Dependency resolution
 * - Security patch detection
 */
class UpdateChecker : public QObject {
    Q_OBJECT

public:
    explicit UpdateChecker(QObject *parent = nullptr);
    ~UpdateChecker();

    /// Configuration
    void setUpdateServer(const QString &url);
    void setCheckInterval(int minutes);
    
    /// Query operations
    QString getCurrentVersion() const;
    bool hasAvailableUpdates();
    UpdateInfo getLatestUpdate() const;
    QList<UpdateInfo> getPendingUpdates() const;
    
    /// Check operations
    void checkForUpdates();
    void scheduleAutoCheck();
    void cancelPendingCheck();
    
    /// Update info
    bool isUpdateInProgress() const;
    bool isUpdateScheduled() const;
    int getDownloadSize() const;

signals:
    void checkStarted();
    void checkFinished(bool updatesAvailable);
    void updateAvailable(const UpdateInfo &info);
    void checkFailed(const QString &error);
    void progressUpdated(int percent);

private slots:
    void onNetworkReply(QNetworkReply *reply);
    void onNetworkError(QNetworkReply::NetworkError error);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    /// Update detection
    bool parseUpdateResponse(const QString &response, UpdateInfo &info);
    bool compareVersions(const QString &current, const QString &available) const;
    
    /// File operations
    QString getVersionFile() const;
    bool saveUpdateInfo(const UpdateInfo &info) const;
    UpdateInfo loadUpdateInfo() const;
    
    // Network
    std::unique_ptr<QNetworkAccessManager> networkManager;
    QString updateServerUrl;
    int checkIntervalMinutes;
    
    // State
    QString currentVersion;
    UpdateInfo latestUpdate;
    bool checkInProgress;
    bool updateScheduled;
};

} // namespace Arbor::UpdateManager

#endif // ARBOR_UPDATE_MANAGER_UPDATE_CHECKER_H
