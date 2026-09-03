#ifndef ARBOR_SOFTWARE_CENTER_FLATPAK_MANAGER_H
#define ARBOR_SOFTWARE_CENTER_FLATPAK_MANAGER_H

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QObject>
#include <memory>

namespace Arbor::SoftwareCenter {

/**
 * @class FlatpakManager
 * @brief Handles Flatpak package management operations
 *
 * Manages:
 * - Flatpak installation checks
 * - App installation/uninstallation
 * - App updates
 * - Flatpak remotes management
 * - Repository queries
 */
class FlatpakManager : public QObject {
    Q_OBJECT

public:
    explicit FlatpakManager(QObject *parent = nullptr);
    ~FlatpakManager();

    /// Query operations
    bool isFlatpakInstalled() const;
    bool isRemoteConfigured(const QString &remoteName) const;
    QStringList getInstalledApps() const;
    QString getAppVersion(const QString &appId) const;
    
    /// Installation operations
    bool installApp(const QString &appId, QString &errorMsg);
    bool uninstallApp(const QString &appId, QString &errorMsg);
    bool updateApp(const QString &appId, QString &errorMsg);
    bool updateAll(QString &errorMsg);
    
    /// Repository management
    bool addRemote(const QString &name, const QString &url, QString &errorMsg);
    bool removeRemote(const QString &name, QString &errorMsg);
    
    /// Utility methods
    QString getFlatpakVersion() const;
    bool verifyAppSignature(const QString &appId) const;

signals:
    void installProgress(int percent);
    void installStarted(const QString &appId);
    void installFinished(const QString &appId, bool success);
    void uninstallFinished(const QString &appId, bool success);
    void errorOccurred(const QString &message);

private:
    /// Process execution
    bool executeCommand(const QStringList &args, QString &output, QString &error) const;
    bool executeCommandAsync(const QStringList &args);
    
    /// Parsing and validation
    QStringList parseAppList(const QString &output) const;
    QString parseVersion(const QString &output) const;
    bool validateAppId(const QString &appId) const;
    
    /// Async process slots
    void onProcessFinished();
    void onProcessError(QProcess::ProcessError error);
    
    // Async process
    std::unique_ptr<QProcess> process;
    QString currentOperation;
};

} // namespace Arbor::SoftwareCenter

#endif // ARBOR_SOFTWARE_CENTER_FLATPAK_MANAGER_H
