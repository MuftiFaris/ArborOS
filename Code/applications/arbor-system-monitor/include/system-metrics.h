#ifndef ARBOR_SYSTEM_MONITOR_SYSTEM_METRICS_H
#define ARBOR_SYSTEM_MONITOR_SYSTEM_METRICS_H

#include <QString>
#include <QList>
#include <QObject>
#include <QTimer>
#include <memory>

namespace Arbor::SystemMonitor {

/**
 * @struct ProcessInfo
 * @brief Information about a running process
 */
struct ProcessInfo {
    int pid;                ///< Process ID
    QString name;           ///< Process name
    QString user;           ///< Owner user
    double cpuUsage;        ///< CPU usage percentage (0-100)
    int memoryUsage;        ///< Memory usage in MB
    QString status;         ///< Process status (Running, Sleeping, etc.)
};

/**
 * @struct SystemMetrics
 * @brief Current system performance metrics
 */
struct SystemMetrics {
    double cpuUsagePercent;      ///< Overall CPU usage (0-100)
    int totalMemoryMB;           ///< Total system memory
    int usedMemoryMB;            ///< Used memory
    int freeMemoryMB;            ///< Free memory
    double memoryUsagePercent;   ///< Memory usage percentage
    
    double netDownloadMBps;      ///< Network download speed
    double netUploadMBps;        ///< Network upload speed
    
    double cpuTemp;              ///< CPU temperature (°C)
    int cpuCoreCount;            ///< Number of CPU cores
    double cpuFreqGHz;           ///< Current CPU frequency
    
    int uptime;                  ///< System uptime in seconds
};

/**
 * @class SystemMetricsCollector
 * @brief Collects and monitors system performance metrics
 *
 * Periodically queries:
 * - CPU usage and frequency
 * - Memory utilization
 * - Network throughput
 * - Process list with resource usage
 * - System temperature
 */
class SystemMetricsCollector : public QObject {
    Q_OBJECT

public:
    explicit SystemMetricsCollector(QObject *parent = nullptr);
    ~SystemMetricsCollector();

    /// Configuration
    void setUpdateInterval(int msec);
    
    /// Data access
    SystemMetrics getCurrentMetrics() const;
    QList<ProcessInfo> getTopProcesses(int limit = 10) const;
    QList<ProcessInfo> getAllProcesses() const;
    ProcessInfo getProcessInfo(int pid) const;
    
    /// Operations
    void startCollecting();
    void stopCollecting();
    
    /// History
    QList<double> getCpuHistory() const { return cpuHistory; }
    QList<double> getMemoryHistory() const { return memoryHistory; }

signals:
    void metricsUpdated(const SystemMetrics &metrics);
    void processeUpdated(const QList<ProcessInfo> &processes);

private slots:
    void onCollectionTimer();

private:
    /// Metric collection (platform-specific)
    SystemMetrics collectMetrics();
    QList<ProcessInfo> collectProcesses();
    
    /// Parsing helpers
    double parseCpuUsage() const;
    void parseMemoryInfo();
    double parseNetworkStats();
    double parseCpuTemperature() const;
    
    /// File reading utilities
    QString readFile(const QString &path) const;
    
    // State
    std::unique_ptr<QTimer> collectionTimer;
    SystemMetrics currentMetrics;
    QList<ProcessInfo> currentProcesses;
    
    // History for graphs
    QList<double> cpuHistory;
    QList<double> memoryHistory;
    static constexpr int HISTORY_MAX = 300; // 5 minutes at 1 Hz
};

} // namespace Arbor::SystemMonitor

#endif // ARBOR_SYSTEM_MONITOR_SYSTEM_METRICS_H
