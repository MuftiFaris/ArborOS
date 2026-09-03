#pragma once

#include <QString>
#include <QVariant>
#include <QMap>

/**
 * @brief System information queries
 * 
 * Provides read-only system info for display in Settings
 */
class SystemInfo {
public:
    // Hardware info
    static QString getOSVersion();
    static QString getOSName();
    static QString getKernelVersion();
    static QString getBuildInfo();

    // CPU info
    static QString getCPUModel();
    static int getCPUCoreCount();
    static int getCPUThreadCount();
    static float getCPUFrequency();

    // Memory info
    static qint64 getTotalMemory();
    static qint64 getAvailableMemory();
    static qint64 getUsedMemory();

    // Storage info
    static QMap<QString, qint64> getDriveInfo();
    static qint64 getDriveFreeSpace(const QString& path);

    // GPU info
    static QString getGPUModel();
    static QString getGPUDriver();

    // TPM/Security info
    static bool hasTPM();
    static bool isSecureBootEnabled();
    static bool isDiskEncrypted();

    // Uptime
    static qint64 getUptimeSeconds();
    static QString getUptimeFormatted();

private:
    static QVariant readProcFile(const QString& path);
    static QVariant readSysFile(const QString& path);
};
