#include "system-metrics.h"
#include <QFile>
#include <QProcess>
#include <QStringList>
#include <QDebug>
#include <QTimer>

namespace Arbor::SystemMonitor {

SystemMetricsCollector::SystemMetricsCollector(QObject *parent)
    : QObject(parent),
      collectionTimer(std::make_unique<QTimer>(this)),
      graphPointCount(0)
{
    connect(collectionTimer.get(), &QTimer::timeout, this, &SystemMetricsCollector::onCollectionTimer);
    
    // Initialize metrics
    currentMetrics = {};
}

SystemMetricsCollector::~SystemMetricsCollector() = default;

void SystemMetricsCollector::setUpdateInterval(int msec)
{
    collectionTimer->setInterval(msec);
}

SystemMetrics SystemMetricsCollector::getCurrentMetrics() const
{
    return currentMetrics;
}

QList<ProcessInfo> SystemMetricsCollector::getTopProcesses(int limit) const
{
    auto processes = currentProcesses;
    
    // Sort by CPU usage descending
    std::sort(processes.begin(), processes.end(),
        [](const ProcessInfo &a, const ProcessInfo &b) {
            return a.cpuUsage > b.cpuUsage;
        });
    
    if (processes.size() > limit) {
        processes = processes.mid(0, limit);
    }
    
    return processes;
}

QList<ProcessInfo> SystemMetricsCollector::getAllProcesses() const
{
    return currentProcesses;
}

ProcessInfo SystemMetricsCollector::getProcessInfo(int pid) const
{
    for (const auto &proc : currentProcesses) {
        if (proc.pid == pid) {
            return proc;
        }
    }
    return ProcessInfo{};
}

void SystemMetricsCollector::startCollecting()
{
    collectionTimer->start(1000); // Default: collect every 1 second
}

void SystemMetricsCollector::stopCollecting()
{
    collectionTimer->stop();
}

void SystemMetricsCollector::onCollectionTimer()
{
    currentMetrics = collectMetrics();
    currentProcesses = collectProcesses();
    
    // Store history for graphs
    cpuHistory.append(currentMetrics.cpuUsagePercent);
    if (cpuHistory.size() > HISTORY_MAX) {
        cpuHistory.removeFirst();
    }
    
    memoryHistory.append(currentMetrics.memoryUsagePercent);
    if (memoryHistory.size() > HISTORY_MAX) {
        memoryHistory.removeFirst();
    }
    
    emit metricsUpdated(currentMetrics);
    emit processeUpdated(currentProcesses);
}

SystemMetrics SystemMetricsCollector::collectMetrics()
{
    SystemMetrics metrics{};
    
    // Parse /proc/stat for CPU usage
    metrics.cpuUsagePercent = parseCpuUsage();
    
    // Parse /proc/meminfo for memory
    parseMemoryInfo();
    metrics.usedMemoryMB = currentMetrics.usedMemoryMB;
    metrics.freeMemoryMB = currentMetrics.freeMemoryMB;
    metrics.totalMemoryMB = currentMetrics.totalMemoryMB;
    metrics.memoryUsagePercent = currentMetrics.memoryUsagePercent;
    
    // Parse network stats
    metrics.netDownloadMBps = parseNetworkStats();
    metrics.netUploadMBps = 0; // Would parse from /proc/net/dev
    
    // Parse CPU info
    metrics.cpuTemp = parseCpuTemperature();
    metrics.cpuCoreCount = 4; // Would detect from /proc/cpuinfo
    metrics.cpuFreqGHz = 2.4; // Would parse from /sys/devices/system/cpu
    
    // System uptime
    metrics.uptime = 86400; // Would parse from /proc/uptime
    
    return metrics;
}

QList<ProcessInfo> SystemMetricsCollector::collectProcesses()
{
    QList<ProcessInfo> processes;
    
    // In production, would read /proc/[pid]/stat for all processes
    // This is a stub implementation
    
    QProcess ps;
    ps.start("ps", QStringList() << "aux");
    
    if (ps.waitForFinished()) {
        const auto output = QString::fromUtf8(ps.readAllStandardOutput());
        const auto lines = output.split('\n', Qt::SkipEmptyParts);
        
        for (int i = 1; i < lines.size() && i < 20; ++i) {
            const auto parts = lines[i].split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() > 10) {
                ProcessInfo info;
                info.user = parts[0];
                info.pid = parts[1].toInt();
                info.cpuUsage = parts[2].toDouble();
                info.memoryUsage = static_cast<int>(parts[3].toDouble() * currentMetrics.totalMemoryMB / 100.0);
                info.name = parts[10];
                info.status = "Running";
                
                processes.append(info);
            }
        }
    }
    
    return processes;
}

double SystemMetricsCollector::parseCpuUsage() const
{
    // Read /proc/stat to calculate CPU usage
    const auto content = readFile("/proc/stat");
    if (content.isEmpty()) {
        return 0.0;
    }
    
    const auto lines = content.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) {
        return 0.0;
    }
    
    const auto parts = lines[0].split(' ', Qt::SkipEmptyParts);
    if (parts.size() < 5) {
        return 0.0;
    }
    
    // Simple stub: return random value for demo
    return 25.0 + (rand() % 30);
}

void SystemMetricsCollector::parseMemoryInfo()
{
    const auto content = readFile("/proc/meminfo");
    const auto lines = content.split('\n', Qt::SkipEmptyParts);
    
    int totalKB = 0, freeKB = 0;
    
    for (const auto &line : lines) {
        if (line.startsWith("MemTotal:")) {
            totalKB = line.split(QRegularExpression("\\s+"))[1].toInt();
        } else if (line.startsWith("MemFree:")) {
            freeKB = line.split(QRegularExpression("\\s+"))[1].toInt();
        }
    }
    
    currentMetrics.totalMemoryMB = totalKB / 1024;
    currentMetrics.freeMemoryMB = freeKB / 1024;
    currentMetrics.usedMemoryMB = currentMetrics.totalMemoryMB - currentMetrics.freeMemoryMB;
    
    if (currentMetrics.totalMemoryMB > 0) {
        currentMetrics.memoryUsagePercent = 
            (static_cast<double>(currentMetrics.usedMemoryMB) / currentMetrics.totalMemoryMB) * 100.0;
    }
}

double SystemMetricsCollector::parseNetworkStats()
{
    // Parse /proc/net/dev for network throughput
    return 5.5; // Stub: 5.5 MB/s
}

double SystemMetricsCollector::parseCpuTemperature() const
{
    const auto content = readFile("/sys/class/thermal/thermal_zone0/temp");
    if (content.isEmpty()) {
        return 45.0;
    }
    
    bool ok;
    const int tempMilli = content.trimmed().toInt(&ok);
    if (ok) {
        return tempMilli / 1000.0;
    }
    
    return 45.0;
}

QString SystemMetricsCollector::readFile(const QString &path) const
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        const auto content = QString::fromUtf8(file.readAll());
        file.close();
        return content;
    }
    return QString();
}

} // namespace Arbor::SystemMonitor
