#include "system-info.h"
#include <QFile>
#include <QProcess>
#include <QDebug>
#include <QSysInfo>

QString SystemInfo::getOSVersion() {
    QFile versionFile("/etc/arbor-os-release");

    if (versionFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(versionFile.readAll());
        versionFile.close();

        for (const QString& line : content.split('\n')) {
            if (line.startsWith("VERSION=")) {
                return line.mid(8).trimmed().remove('"');
            }
        }
    }

    return "Unknown";
}

QString SystemInfo::getOSName() {
    return "Arbor OS";
}

QString SystemInfo::getKernelVersion() {
    return QSysInfo::kernelVersion();
}

QString SystemInfo::getBuildInfo() {
    QFile osRelease("/etc/os-release");

    if (osRelease.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(osRelease.readAll());
        osRelease.close();

        for (const QString& line : content.split('\n')) {
            if (line.startsWith("BUILD_ID=")) {
                return line.mid(9).trimmed().remove('"');
            }
        }
    }

    return "Unknown";
}

QString SystemInfo::getCPUModel() {
    QFile cpuInfo("/proc/cpuinfo");

    if (cpuInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(cpuInfo.readAll());
        cpuInfo.close();

        for (const QString& line : content.split('\n')) {
            if (line.startsWith("model name")) {
                return line.split(':').last().trimmed();
            }
        }
    }

    return "Unknown";
}

int SystemInfo::getCPUCoreCount() {
    QFile cpuInfo("/proc/cpuinfo");

    if (cpuInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(cpuInfo.readAll());
        cpuInfo.close();

        int count = 0;

        for (const QString& line : content.split('\n')) {
            if (line.startsWith("processor")) {
                count++;
            }
        }

        return count;
    }

    return 1;
}

int SystemInfo::getCPUThreadCount() {
    // On most systems, threads = cores (no hyperthreading info in /proc/cpuinfo)
    return getCPUCoreCount();
}

float SystemInfo::getCPUFrequency() {
    QFile cpuInfo("/proc/cpuinfo");

    if (cpuInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(cpuInfo.readAll());
        cpuInfo.close();

        for (const QString& line : content.split('\n')) {
            if (line.startsWith("cpu MHz")) {
                return line.split(':').last().trimmed().toFloat();
            }
        }
    }

    return 0.0f;
}

qint64 SystemInfo::getTotalMemory() {
    QFile memInfo("/proc/meminfo");

    if (memInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(memInfo.readAll());
        memInfo.close();

        for (const QString& line : content.split('\n')) {
            if (line.startsWith("MemTotal")) {
                return line.split(':').last().trimmed().split(' ').first().toLongLong() * 1024;
            }
        }
    }

    return 0;
}

qint64 SystemInfo::getAvailableMemory() {
    QFile memInfo("/proc/meminfo");

    if (memInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(memInfo.readAll());
        memInfo.close();

        for (const QString& line : content.split('\n')) {
            if (line.startsWith("MemAvailable")) {
                return line.split(':').last().trimmed().split(' ').first().toLongLong() * 1024;
            }
        }
    }

    return 0;
}

qint64 SystemInfo::getUsedMemory() {
    return getTotalMemory() - getAvailableMemory();
}

QMap<QString, qint64> SystemInfo::getDriveInfo() {
    QMap<QString, qint64> drives;

    // Parse /proc/partitions or use lsblk
    QProcess lsblk;
    lsblk.start("lsblk", QStringList() << "-b" << "-n" << "-o" << "NAME,SIZE");

    if (lsblk.waitForFinished()) {
        QString output = QString::fromUtf8(lsblk.readAllStandardOutput());

        for (const QString& line : output.split('\n')) {
            if (line.isEmpty()) continue;

            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

            if (parts.size() >= 2) {
                drives["/dev/" + parts[0]] = parts[1].toLongLong();
            }
        }
    }

    return drives;
}

qint64 SystemInfo::getDriveFreeSpace(const QString& path) {
    QProcess df;
    df.start("df", QStringList() << "-B1" << path);

    if (df.waitForFinished()) {
        QString output = QString::fromUtf8(df.readAllStandardOutput());
        QStringList lines = output.split('\n');

        if (lines.size() >= 2) {
            QStringList parts = lines[1].split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

            if (parts.size() >= 4) {
                return parts[3].toLongLong();
            }
        }
    }

    return 0;
}

QString SystemInfo::getGPUModel() {
    QProcess lspci;
    lspci.start("lspci", QStringList() << "-n");

    if (lspci.waitForFinished()) {
        QString output = QString::fromUtf8(lspci.readAllStandardOutput());

        for (const QString& line : output.split('\n')) {
            if (line.contains("VGA compatible controller", Qt::CaseInsensitive)) {
                return line;
            }
        }
    }

    return "Unknown";
}

QString SystemInfo::getGPUDriver() {
    QFile drmDevices("/sys/class/drm");

    if (drmDevices.exists()) {
        return "DRM (Open Source)";
    }

    // Check for NVIDIA proprietary driver
    QProcess nvidia;
    nvidia.start("nvidia-smi");

    if (nvidia.waitForFinished(1000)) {
        return "NVIDIA proprietary driver";
    }

    return "Unknown";
}

bool SystemInfo::hasTPM() {
    QFile tpmDevice("/dev/tpm0");
    return tpmDevice.exists();
}

bool SystemInfo::isSecureBootEnabled() {
    QFile secureBootFile("/sys/firmware/efi/efivars/SecureBoot-8be4df61-93ca-11d2-aa0d-00e098032b8c");
    return secureBootFile.exists();
}

bool SystemInfo::isDiskEncrypted() {
    QProcess dmsetup;
    dmsetup.start("dmsetup", QStringList() << "table");

    if (dmsetup.waitForFinished()) {
        QString output = QString::fromUtf8(dmsetup.readAllStandardOutput());
        return output.contains("crypt", Qt::CaseInsensitive);
    }

    return false;
}

qint64 SystemInfo::getUptimeSeconds() {
    QFile uptime("/proc/uptime");

    if (uptime.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(uptime.readAll());
        uptime.close();

        QStringList parts = content.split(' ');

        if (!parts.isEmpty()) {
            return (qint64)parts[0].toDouble();
        }
    }

    return 0;
}

QString SystemInfo::getUptimeFormatted() {
    qint64 seconds = getUptimeSeconds();

    int days = seconds / 86400;
    int hours = (seconds % 86400) / 3600;
    int minutes = (seconds % 3600) / 60;

    return QString("%1d %2h %3m").arg(days).arg(hours).arg(minutes);
}

QVariant SystemInfo::readProcFile(const QString& path) {
    QFile file(path);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(file.readAll());
        file.close();
        return content;
    }

    return QVariant();
}

QVariant SystemInfo::readSysFile(const QString& path) {
    QFile file(path);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(file.readAll());
        file.close();
        return content;
    }

    return QVariant();
}
