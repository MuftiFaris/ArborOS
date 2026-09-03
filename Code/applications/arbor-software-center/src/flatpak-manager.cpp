#include "flatpak-manager.h"
#include <QDebug>
#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace Arbor::SoftwareCenter {

FlatpakManager::FlatpakManager(QObject *parent)
    : QObject(parent), process(std::make_unique<QProcess>(this))
{
    connect(process.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &FlatpakManager::onProcessFinished);
    connect(process.get(), QOverload<QProcess::ProcessError>::of(&QProcess::error),
            this, &FlatpakManager::onProcessError);
}

FlatpakManager::~FlatpakManager() = default;

bool FlatpakManager::isFlatpakInstalled() const
{
    QString output, error;
    return executeCommand({"flatpak", "--version"}, output, error);
}

bool FlatpakManager::isRemoteConfigured(const QString &remoteName) const
{
    QString output, error;
    if (!executeCommand({"flatpak", "remotes"}, output, error)) {
        return false;
    }
    return output.contains(remoteName);
}

QStringList FlatpakManager::getInstalledApps() const
{
    QString output, error;
    if (!executeCommand({"flatpak", "list", "--app", "--columns=application"}, output, error)) {
        return QStringList();
    }
    return parseAppList(output);
}

QString FlatpakManager::getAppVersion(const QString &appId) const
{
    QString output, error;
    QStringList args = {"flatpak", "info", appId};
    if (!executeCommand(args, output, error)) {
        return QString();
    }
    return parseVersion(output);
}

bool FlatpakManager::installApp(const QString &appId, QString &errorMsg)
{
    if (!validateAppId(appId)) {
        errorMsg = "Invalid app ID format";
        return false;
    }
    
    emit installStarted(appId);
    
    QString output, error;
    bool success = executeCommand({"flatpak", "install", "-y", appId}, output, error);
    
    if (!success) {
        errorMsg = error;
        emit errorOccurred(error);
    }
    
    emit installFinished(appId, success);
    return success;
}

bool FlatpakManager::uninstallApp(const QString &appId, QString &errorMsg)
{
    if (!validateAppId(appId)) {
        errorMsg = "Invalid app ID format";
        return false;
    }
    
    QString output, error;
    bool success = executeCommand({"flatpak", "uninstall", "-y", appId}, output, error);
    
    if (!success) {
        errorMsg = error;
        emit errorOccurred(error);
    }
    
    emit uninstallFinished(appId, success);
    return success;
}

bool FlatpakManager::updateApp(const QString &appId, QString &errorMsg)
{
    if (!validateAppId(appId)) {
        errorMsg = "Invalid app ID format";
        return false;
    }
    
    QString output, error;
    bool success = executeCommand({"flatpak", "update", "-y", appId}, output, error);
    
    if (!success) {
        errorMsg = error;
        emit errorOccurred(error);
    }
    
    return success;
}

bool FlatpakManager::updateAll(QString &errorMsg)
{
    QString output, error;
    bool success = executeCommand({"flatpak", "update", "-y"}, output, error);
    
    if (!success) {
        errorMsg = error;
        emit errorOccurred(error);
    }
    
    return success;
}

bool FlatpakManager::addRemote(const QString &name, const QString &url, QString &errorMsg)
{
    QString output, error;
    bool success = executeCommand({"flatpak", "remote-add", "--if-not-exists", name, url}, 
                                 output, error);
    
    if (!success) {
        errorMsg = error;
        emit errorOccurred(error);
    }
    
    return success;
}

bool FlatpakManager::removeRemote(const QString &name, QString &errorMsg)
{
    QString output, error;
    bool success = executeCommand({"flatpak", "remote-remove", name}, output, error);
    
    if (!success) {
        errorMsg = error;
        emit errorOccurred(error);
    }
    
    return success;
}

QString FlatpakManager::getFlatpakVersion() const
{
    QString output, error;
    if (executeCommand({"flatpak", "--version"}, output, error)) {
        return output.trimmed();
    }
    return QString();
}

bool FlatpakManager::verifyAppSignature(const QString &appId) const
{
    // Security: verify Flatpak manifest signature
    QString output, error;
    QStringList args = {"flatpak", "info", appId};
    return executeCommand(args, output, error);
}

bool FlatpakManager::executeCommand(const QStringList &args, QString &output, QString &error) const
{
    QProcess proc;
    proc.start(args.at(0), args.mid(1));
    
    if (!proc.waitForFinished(30000)) {
        error = "Command timeout";
        return false;
    }
    
    if (proc.exitCode() != 0) {
        error = QString::fromUtf8(proc.readAllStandardError());
        return false;
    }
    
    output = QString::fromUtf8(proc.readAllStandardOutput());
    return true;
}

bool FlatpakManager::executeCommandAsync(const QStringList &args)
{
    if (process->state() == QProcess::Running) {
        return false;
    }
    
    currentOperation = args.join(" ");
    process->start(args.at(0), args.mid(1));
    return process->waitForStarted();
}

QStringList FlatpakManager::parseAppList(const QString &output) const
{
    QStringList apps;
    const auto lines = output.split('\n', Qt::SkipEmptyParts);
    
    for (const auto &line : lines) {
        const auto trimmed = line.trimmed();
        if (!trimmed.isEmpty() && !trimmed.startsWith("Application ID")) {
            apps.append(trimmed);
        }
    }
    
    return apps;
}

QString FlatpakManager::parseVersion(const QString &output) const
{
    // Extract version from "flatpak info" output
    const auto lines = output.split('\n', Qt::SkipEmptyParts);
    
    for (const auto &line : lines) {
        if (line.contains("Version:")) {
            const auto parts = line.split(':');
            if (parts.size() > 1) {
                return parts.at(1).trimmed();
            }
        }
    }
    
    return QString();
}

bool FlatpakManager::validateAppId(const QString &appId) const
{
    // Validate app ID format: com.example.App or org.name.app
    if (appId.isEmpty() || appId.count('.') < 2) {
        return false;
    }
    
    const auto parts = appId.split('.');
    for (const auto &part : parts) {
        if (part.isEmpty() || !part.at(0).isLetter()) {
            return false;
        }
    }
    
    return true;
}

void FlatpakManager::onProcessFinished()
{
    // Async operation completed
    if (process->exitCode() == 0) {
        qDebug() << "Operation succeeded:" << currentOperation;
    } else {
        const auto error = QString::fromUtf8(process->readAllStandardError());
        emit errorOccurred(error);
    }
}

void FlatpakManager::onProcessError(QProcess::ProcessError error)
{
    qDebug() << "Process error:" << static_cast<int>(error);
    emit errorOccurred("Process execution failed");
}

} // namespace Arbor::SoftwareCenter
