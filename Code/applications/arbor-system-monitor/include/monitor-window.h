#ifndef ARBOR_SYSTEM_MONITOR_MONITOR_WINDOW_H
#define ARBOR_SYSTEM_MONITOR_MONITOR_WINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QTableWidget>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <memory>
#include "system-metrics.h"

namespace Arbor::SystemMonitor {

/**
 * @class MonitorWindow
 * @brief Real-time system performance monitoring UI
 *
 * Displays:
 * - Live CPU, memory, network graphs
 * - Top processes table
 * - System information panel
 * - Resource usage gauges
 */
class MonitorWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MonitorWindow(QWidget *parent = nullptr);
    ~MonitorWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    /// Update handlers
    void onMetricsUpdated(const SystemMetrics &metrics);
    void onProcessesUpdated(const QList<ProcessInfo> &processes);
    
    /// UI interactions
    void onRefreshClicked();
    void onProcessKillClicked();
    void onProcessDetailsClicked();
    void onTabChanged(int index);

private:
    /// UI setup
    void setupUI();
    void createMenuBar();
    void createOverviewTab();
    void createProcessesTab();
    void createSystemInfoTab();
    
    /// Graph creation
    void createCpuGraph();
    void createMemoryGraph();
    void createNetworkGraph();
    
    /// Update methods
    void updateOverviewMetrics(const SystemMetrics &metrics);
    void updateProcessTable(const QList<ProcessInfo> &processes);
    void updateSystemInfo(const SystemMetrics &metrics);
    void updateGraphs(const SystemMetrics &metrics);
    
    /// Helper methods
    QString formatBytes(int mb) const;
    QString formatSpeed(double mbps) const;
    QString formatTemperature(double celsius) const;
    
    // Tabs and main widgets
    QWidget *overviewTab;
    QWidget *processesTab;
    QWidget *systemInfoTab;
    
    // Overview widgets
    QLabel *cpuLabel;
    QProgressBar *cpuBar;
    QLabel *memoryLabel;
    QProgressBar *memoryBar;
    QLabel *networkLabel;
    
    QtCharts::QChartView *cpuChart;
    QtCharts::QChartView *memoryChart;
    QtCharts::QChartView *networkChart;
    
    QtCharts::QLineSeries *cpuSeries;
    QtCharts::QLineSeries *memorySeries;
    QtCharts::QLineSeries *downloadSeries;
    QtCharts::QLineSeries *uploadSeries;
    
    // Processes tab
    QTableWidget *processTable;
    
    // System info tab
    QLabel *kernelLabel;
    QLabel *uptimeLabel;
    QLabel *cpuCoresLabel;
    QLabel *cpuFreqLabel;
    QLabel *cpuTempLabel;
    QLabel *totalMemoryLabel;
    
    // Metrics collector
    std::unique_ptr<SystemMetricsCollector> metricsCollector;
    
    // State
    int graphPointCount;
};

} // namespace Arbor::SystemMonitor

#endif // ARBOR_SYSTEM_MONITOR_MONITOR_WINDOW_H
