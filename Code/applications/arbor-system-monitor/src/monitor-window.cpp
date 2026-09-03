#include "monitor-window.h"
#include "../include/style-provider.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QGroupBox>
#include <QPushButton>
#include <QHeaderView>
#include <QChartView>
#include <QChart>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QCloseEvent>

using namespace QtCharts;

namespace Arbor::SystemMonitor {

MonitorWindow::MonitorWindow(QWidget *parent)
    : QMainWindow(parent),
      metricsCollector(std::make_unique<SystemMetricsCollector>(this)),
      graphPointCount(0)
{
    setWindowTitle("Arbor System Monitor");
    setWindowIcon(QIcon(":/icons/monitor.svg"));
    setGeometry(100, 100, 1400, 900);
    
    setupUI();
    createMenuBar();
    
    // Apply design system theme
    Arbor::DesignSystem::StyleProvider::instance()->applyStyle(this);
    
    // Connect signals
    connect(metricsCollector.get(), &SystemMetricsCollector::metricsUpdated,
            this, &MonitorWindow::onMetricsUpdated);
    connect(metricsCollector.get(), &SystemMetricsCollector::processeUpdated,
            this, &MonitorWindow::onProcessesUpdated);
    
    // Start collection
    metricsCollector->setUpdateInterval(1000);
    metricsCollector->startCollecting();
}

MonitorWindow::~MonitorWindow() = default;

void MonitorWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);
    
    // Create tabs
    QTabWidget *tabWidget = new QTabWidget;
    
    createOverviewTab();
    createProcessesTab();
    createSystemInfoTab();
    
    tabWidget->addTab(overviewTab, "Overview");
    tabWidget->addTab(processesTab, "Processes");
    tabWidget->addTab(systemInfoTab, "System Info");
    
    connect(tabWidget, QOverload<int>::of(&QTabWidget::currentChanged),
            this, &MonitorWindow::onTabChanged);
    
    mainLayout->addWidget(tabWidget);
}

void MonitorWindow::createMenuBar()
{
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // File menu
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    QAction *exitAction = fileMenu->addAction(tr("E&xit"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // View menu
    QMenu *viewMenu = menuBar->addMenu(tr("&View"));
    QAction *refreshAction = viewMenu->addAction(tr("&Refresh"));
    connect(refreshAction, &QAction::triggered, this, &MonitorWindow::onRefreshClicked);
}

void MonitorWindow::createOverviewTab()
{
    overviewTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(overviewTab);
    
    // CPU gauge
    QGroupBox *cpuGroup = new QGroupBox("CPU Usage");
    QVBoxLayout *cpuLayout = new QVBoxLayout(cpuGroup);
    cpuLabel = new QLabel("CPU: 0%");
    cpuBar = new QProgressBar;
    cpuBar->setMaximum(100);
    cpuBar->setValue(0);
    cpuLayout->addWidget(cpuLabel);
    cpuLayout->addWidget(cpuBar);
    
    // Memory gauge
    QGroupBox *memGroup = new QGroupBox("Memory Usage");
    QVBoxLayout *memLayout = new QVBoxLayout(memGroup);
    memoryLabel = new QLabel("Memory: 0%");
    memoryBar = new QProgressBar;
    memoryBar->setMaximum(100);
    memoryBar->setValue(0);
    memLayout->addWidget(memoryLabel);
    memLayout->addWidget(memoryBar);
    
    // Network info
    QGroupBox *netGroup = new QGroupBox("Network");
    QVBoxLayout *netLayout = new QVBoxLayout(netGroup);
    networkLabel = new QLabel("Download: 0 MB/s | Upload: 0 MB/s");
    netLayout->addWidget(networkLabel);
    
    // Gauges row
    QHBoxLayout *gaugesLayout = new QHBoxLayout;
    gaugesLayout->addWidget(cpuGroup);
    gaugesLayout->addWidget(memGroup);
    
    layout->addLayout(gaugesLayout);
    layout->addWidget(netGroup);
    
    // Create graphs
    createCpuGraph();
    createMemoryGraph();
    createNetworkGraph();
    
    QHBoxLayout *chartsLayout = new QHBoxLayout;
    chartsLayout->addWidget(cpuChart);
    chartsLayout->addWidget(memoryChart);
    layout->addLayout(chartsLayout);
    
    layout->addWidget(networkChart);
}

void MonitorWindow::createProcessesTab()
{
    processesTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(processesTab);
    
    // Process table
    processTable = new QTableWidget;
    processTable->setColumnCount(5);
    processTable->setHorizontalHeaderLabels({"PID", "Name", "User", "CPU %", "Memory (MB)"});
    processTable->horizontalHeader()->setStretchLastSection(true);
    
    layout->addWidget(processTable);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    
    QPushButton *refreshButton = new QPushButton("Refresh");
    connect(refreshButton, &QPushButton::clicked, this, &MonitorWindow::onRefreshClicked);
    
    QPushButton *killButton = new QPushButton("Kill Process");
    connect(killButton, &QPushButton::clicked, this, &MonitorWindow::onProcessKillClicked);
    
    QPushButton *detailsButton = new QPushButton("Details");
    connect(detailsButton, &QPushButton::clicked, this, &MonitorWindow::onProcessDetailsClicked);
    
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(killButton);
    buttonLayout->addWidget(detailsButton);
    buttonLayout->addStretch();
    
    layout->addLayout(buttonLayout);
}

void MonitorWindow::createSystemInfoTab()
{
    systemInfoTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(systemInfoTab);
    
    kernelLabel = new QLabel("Kernel: Linux");
    uptimeLabel = new QLabel("Uptime: 1 day, 2 hours");
    cpuCoresLabel = new QLabel("CPU Cores: 4");
    cpuFreqLabel = new QLabel("CPU Frequency: 2.4 GHz");
    cpuTempLabel = new QLabel("CPU Temperature: 45°C");
    totalMemoryLabel = new QLabel("Total Memory: 8192 MB");
    
    layout->addWidget(kernelLabel);
    layout->addWidget(uptimeLabel);
    layout->addWidget(cpuCoresLabel);
    layout->addWidget(cpuFreqLabel);
    layout->addWidget(cpuTempLabel);
    layout->addWidget(totalMemoryLabel);
    layout->addStretch();
}

void MonitorWindow::createCpuGraph()
{
    auto *chart = new QChart();
    chart->setTitle("CPU Usage Over Time");
    
    cpuSeries = new QLineSeries();
    chart->addSeries(cpuSeries);
    
    auto *axisX = new QValueAxis();
    auto *axisY = new QValueAxis();
    
    axisX->setRange(0, 300);
    axisY->setRange(0, 100);
    
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    
    cpuSeries->attachAxis(axisX);
    cpuSeries->attachAxis(axisY);
    
    cpuChart = new QChartView(chart);
    cpuChart->setRenderHint(QPainter::Antialiasing);
}

void MonitorWindow::createMemoryGraph()
{
    auto *chart = new QChart();
    chart->setTitle("Memory Usage Over Time");
    
    memorySeries = new QLineSeries();
    chart->addSeries(memorySeries);
    
    auto *axisX = new QValueAxis();
    auto *axisY = new QValueAxis();
    
    axisX->setRange(0, 300);
    axisY->setRange(0, 100);
    
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    
    memorySeries->attachAxis(axisX);
    memorySeries->attachAxis(axisY);
    
    memoryChart = new QChartView(chart);
    memoryChart->setRenderHint(QPainter::Antialiasing);
}

void MonitorWindow::createNetworkGraph()
{
    auto *chart = new QChart();
    chart->setTitle("Network Throughput");
    
    downloadSeries = new QLineSeries();
    uploadSeries = new QLineSeries();
    chart->addSeries(downloadSeries);
    chart->addSeries(uploadSeries);
    
    auto *axisX = new QValueAxis();
    auto *axisY = new QValueAxis();
    
    axisX->setRange(0, 300);
    axisY->setRange(0, 50);
    
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    
    downloadSeries->attachAxis(axisX);
    downloadSeries->attachAxis(axisY);
    uploadSeries->attachAxis(axisX);
    uploadSeries->attachAxis(axisY);
    
    networkChart = new QChartView(chart);
    networkChart->setRenderHint(QPainter::Antialiasing);
}

void MonitorWindow::onMetricsUpdated(const SystemMetrics &metrics)
{
    updateOverviewMetrics(metrics);
    updateGraphs(metrics);
}

void MonitorWindow::onProcessesUpdated(const QList<ProcessInfo> &processes)
{
    updateProcessTable(processes);
}

void MonitorWindow::onRefreshClicked()
{
    auto metrics = metricsCollector->getCurrentMetrics();
    updateOverviewMetrics(metrics);
    updateSystemInfo(metrics);
    updateProcessTable(metricsCollector->getTopProcesses());
}

void MonitorWindow::onProcessKillClicked()
{
    // In production, would kill selected process
    statusBar()->showMessage("Kill process not implemented");
}

void MonitorWindow::onProcessDetailsClicked()
{
    statusBar()->showMessage("Process details not implemented");
}

void MonitorWindow::onTabChanged(int index)
{
    // Handle tab switching
}

void MonitorWindow::updateOverviewMetrics(const SystemMetrics &metrics)
{
    cpuLabel->setText(QString("CPU: %1%").arg(static_cast<int>(metrics.cpuUsagePercent)));
    cpuBar->setValue(static_cast<int>(metrics.cpuUsagePercent));
    
    memoryLabel->setText(QString("Memory: %1%").arg(static_cast<int>(metrics.memoryUsagePercent)));
    memoryBar->setValue(static_cast<int>(metrics.memoryUsagePercent));
    
    networkLabel->setText(QString("Download: %1 MB/s | Upload: %2 MB/s")
        .arg(metrics.netDownloadMBps, 0, 'f', 1)
        .arg(metrics.netUploadMBps, 0, 'f', 1));
}

void MonitorWindow::updateProcessTable(const QList<ProcessInfo> &processes)
{
    processTable->setRowCount(processes.size());
    
    for (int i = 0; i < processes.size(); ++i) {
        const auto &proc = processes[i];
        
        auto *pidItem = new QTableWidgetItem(QString::number(proc.pid));
        auto *nameItem = new QTableWidgetItem(proc.name);
        auto *userItem = new QTableWidgetItem(proc.user);
        auto *cpuItem = new QTableWidgetItem(QString::number(proc.cpuUsage, 'f', 1));
        auto *memItem = new QTableWidgetItem(QString::number(proc.memoryUsage));
        
        processTable->setItem(i, 0, pidItem);
        processTable->setItem(i, 1, nameItem);
        processTable->setItem(i, 2, userItem);
        processTable->setItem(i, 3, cpuItem);
        processTable->setItem(i, 4, memItem);
    }
}

void MonitorWindow::updateSystemInfo(const SystemMetrics &metrics)
{
    cpuCoresLabel->setText(QString("CPU Cores: %1").arg(metrics.cpuCoreCount));
    cpuFreqLabel->setText(QString("CPU Frequency: %1 GHz").arg(metrics.cpuFreqGHz, 0, 'f', 1));
    cpuTempLabel->setText(QString("CPU Temperature: %1°C").arg(metrics.cpuTemp, 0, 'f', 1));
    totalMemoryLabel->setText(QString("Total Memory: %1 MB").arg(metrics.totalMemoryMB));
}

void MonitorWindow::updateGraphs(const SystemMetrics &metrics)
{
    // Add data points to series
    cpuSeries->append(graphPointCount, metrics.cpuUsagePercent);
    memorySeries->append(graphPointCount, metrics.memoryUsagePercent);
    downloadSeries->append(graphPointCount, metrics.netDownloadMBps);
    uploadSeries->append(graphPointCount, metrics.netUploadMBps);
    
    graphPointCount++;
}

QString MonitorWindow::formatBytes(int mb) const
{
    if (mb < 1024) {
        return QString("%1 MB").arg(mb);
    }
    return QString("%1 GB").arg(mb / 1024.0, 0, 'f', 1);
}

QString MonitorWindow::formatSpeed(double mbps) const
{
    return QString("%1 MB/s").arg(mbps, 0, 'f', 1);
}

QString MonitorWindow::formatTemperature(double celsius) const
{
    return QString("%1°C").arg(celsius, 0, 'f', 1);
}

void MonitorWindow::closeEvent(QCloseEvent *event)
{
    metricsCollector->stopCollecting();
    event->accept();
}

} // namespace Arbor::SystemMonitor
