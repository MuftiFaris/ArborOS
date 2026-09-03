#include "update-window.h"
#include "../include/style-provider.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QGroupBox>
#include <QScrollArea>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QCalendarWidget>
#include <QCloseEvent>
#include <QDebug>

namespace Arbor::UpdateManager {

UpdateWindow::UpdateWindow(QWidget *parent)
    : QMainWindow(parent),
      updateChecker(std::make_unique<UpdateChecker>(this)),
      autoCheckEnabled(true),
      updateScheduled(false)
{
    setWindowTitle("Arbor Update Manager");
    setWindowIcon(QIcon(":/icons/update.svg"));
    setGeometry(100, 100, 1000, 700);
    
    setupUI();
    displayCurrentVersion();
    createMenuBar();
    
    // Apply design system theme
    Arbor::DesignSystem::StyleProvider::instance()->applyStyle(this);
    
    // Connect update checker signals
    connect(updateChecker.get(), &UpdateChecker::checkStarted,
            this, &UpdateWindow::onCheckStarted);
    connect(updateChecker.get(), &UpdateChecker::checkFinished,
            this, &UpdateWindow::onCheckFinished);
    connect(updateChecker.get(), &UpdateChecker::updateAvailable,
            this, &UpdateWindow::onUpdateAvailable);
    connect(updateChecker.get(), &UpdateChecker::checkFailed,
            this, &UpdateWindow::onCheckFailed);
    connect(updateChecker.get(), &UpdateChecker::progressUpdated,
            this, &UpdateWindow::onProgressUpdated);
    
    // Auto-check on startup
    checkForUpdates();
}

UpdateWindow::~UpdateWindow() = default;

void UpdateWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);
    
    createStatusPanel();
    createUpdateList();
    createDetailsPanel();
    createInstallTimeline();
    createStatusBar();
    
    mainLayout->addStretch();
}

void UpdateWindow::createMenuBar()
{
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // File menu
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    QAction *exitAction = fileMenu->addAction(tr("E&xit"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Edit menu
    QMenu *editMenu = menuBar->addMenu(tr("&Edit"));
    QAction *prefsAction = editMenu->addAction(tr("&Preferences"));
    connect(prefsAction, &QAction::triggered, [this]() {
        statusLabel->setText("Preferences not yet implemented");
    });
    
    // Help menu
    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));
    QAction *aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered, [this]() {
        statusLabel->setText("Arbor Update Manager v1.0");
    });
}

void UpdateWindow::createStatusPanel()
{
    QGroupBox *statusGroup = new QGroupBox(tr("System Status"), this);
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
    
    currentVersionLabel = new QLabel;
    currentVersionLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    
    systemStatusLabel = new QLabel;
    systemStatusLabel->setWordWrap(true);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    
    checkUpdatesButton = new QPushButton(tr("Check for Updates"));
    connect(checkUpdatesButton, &QPushButton::clicked, this, &UpdateWindow::onCheckUpdatesClicked);
    
    cancelButton = new QPushButton(tr("Cancel"));
    cancelButton->setEnabled(false);
    connect(cancelButton, &QPushButton::clicked, this, &UpdateWindow::onCancelClicked);
    
    buttonLayout->addWidget(checkUpdatesButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    
    QCheckBox *autoCheckBox = new QCheckBox(tr("Check automatically"));
    autoCheckBox->setChecked(autoCheckEnabled);
    connect(autoCheckBox, &QCheckBox::toggled, this, &UpdateWindow::onAutoCheckToggled);
    
    statusLayout->addWidget(currentVersionLabel);
    statusLayout->addWidget(systemStatusLabel);
    statusLayout->addLayout(buttonLayout);
    statusLayout->addWidget(autoCheckBox);
    
    QWidget *centralWidget = this->centralWidget();
    if (centralWidget && centralWidget->layout()) {
        centralWidget->layout()->insertWidget(0, statusGroup);
    }
}

void UpdateWindow::createUpdateList()
{
    QGroupBox *updateGroup = new QGroupBox(tr("Available Updates"), this);
    QVBoxLayout *updateLayout = new QVBoxLayout(updateGroup);
    
    updateListWidget = new QListWidget;
    connect(updateListWidget, &QListWidget::itemSelectionChanged,
            this, &UpdateWindow::onUpdateListItemSelected);
    
    updateLayout->addWidget(updateListWidget);
    
    QWidget *centralWidget = this->centralWidget();
    if (centralWidget && centralWidget->layout()) {
        centralWidget->layout()->insertWidget(1, updateGroup);
    }
}

void UpdateWindow::createDetailsPanel()
{
    QGroupBox *detailsGroup = new QGroupBox(tr("Update Details"), this);
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsGroup);
    
    updateNameLabel = new QLabel;
    updateNameLabel->setStyleSheet("font-weight: bold;");
    
    updateSizeLabel = new QLabel;
    updateDateLabel = new QLabel;
    updateSecurityLabel = new QLabel;
    
    releaseNotesEdit = new QTextEdit;
    releaseNotesEdit->setReadOnly(true);
    releaseNotesEdit->setMaximumHeight(150);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    installButton = new QPushButton(tr("Install Now"));
    scheduleButton = new QPushButton(tr("Schedule"));
    
    connect(installButton, &QPushButton::clicked, this, &UpdateWindow::onInstallUpdatesClicked);
    connect(scheduleButton, &QPushButton::clicked, this, &UpdateWindow::onScheduleUpdateClicked);
    
    buttonLayout->addWidget(installButton);
    buttonLayout->addWidget(scheduleButton);
    buttonLayout->addStretch();
    
    detailsLayout->addWidget(updateNameLabel);
    detailsLayout->addWidget(updateSizeLabel);
    detailsLayout->addWidget(updateDateLabel);
    detailsLayout->addWidget(updateSecurityLabel);
    detailsLayout->addWidget(new QLabel(tr("Release Notes:")));
    detailsLayout->addWidget(releaseNotesEdit);
    detailsLayout->addLayout(buttonLayout);
    
    QWidget *centralWidget = this->centralWidget();
    if (centralWidget && centralWidget->layout()) {
        centralWidget->layout()->insertWidget(2, detailsGroup);
    }
}

void UpdateWindow::createInstallTimeline()
{
    QGroupBox *timelineGroup = new QGroupBox(tr("Installation Timeline"), this);
    QVBoxLayout *timelineLayout = new QVBoxLayout(timelineGroup);
    
    timelineLabel = new QLabel;
    timelineLabel->setWordWrap(true);
    
    installProgress = new QProgressBar;
    installProgress->setMaximum(100);
    installProgress->setValue(0);
    installProgress->setVisible(false);
    
    estimatedTimeLabel = new QLabel;
    
    timelineLayout->addWidget(timelineLabel);
    timelineLayout->addWidget(installProgress);
    timelineLayout->addWidget(estimatedTimeLabel);
    
    QWidget *centralWidget = this->centralWidget();
    if (centralWidget && centralWidget->layout()) {
        centralWidget->layout()->insertWidget(3, timelineGroup);
    }
}

void UpdateWindow::createStatusBar()
{
    statusLabel = new QLabel(tr("Ready"));
    statusBar()->addWidget(statusLabel, 1);
}

void UpdateWindow::displayCurrentVersion()
{
    const auto version = updateChecker->getCurrentVersion();
    currentVersionLabel->setText(QString("Current Version: %1").arg(version));
}

void UpdateWindow::displayAvailableUpdates()
{
    updateListWidget->clear();
    
    auto updates = updateChecker->getPendingUpdates();
    for (const auto &update : updates) {
        const auto text = QString("%1 (%2 MB)%3")
            .arg(update.version)
            .arg(update.downloadSize)
            .arg(update.isSecurityUpdate ? " [SECURITY]" : "");
        updateListWidget->addItem(text);
    }
}

void UpdateWindow::displayUpdateDetails(const UpdateInfo &info)
{
    selectedUpdate = info;
    
    updateNameLabel->setText(QString("Version %1").arg(info.version));
    updateSizeLabel->setText(QString("Size: %1 MB").arg(info.downloadSize));
    updateDateLabel->setText(QString("Released: %1").arg(info.releaseDate.toString()));
    
    if (info.isSecurityUpdate) {
        updateSecurityLabel->setText("🔒 Security Update");
        updateSecurityLabel->setStyleSheet("color: red; font-weight: bold;");
    } else {
        updateSecurityLabel->setText("Standard Update");
    }
    
    releaseNotesEdit->setPlainText(info.releaseNotes);
    estimatedTimeLabel->setText(QString("Estimated time: %1").arg(getTimeEstimate(info.downloadSize)));
}

void UpdateWindow::displayInstallationTimeline()
{
    QString timeline = "Installation Steps:\n";
    timeline += "1. Download update package\n";
    timeline += "2. Verify integrity\n";
    timeline += "3. Install components\n";
    timeline += "4. System restart (if required)\n";
    
    timelineLabel->setText(timeline);
}

void UpdateWindow::onCheckUpdatesClicked()
{
    checkForUpdates();
}

void UpdateWindow::onInstallUpdatesClicked()
{
    if (selectedUpdate.version.isEmpty()) {
        statusLabel->setText("No update selected");
        return;
    }
    
    installUpdates();
}

void UpdateWindow::onScheduleUpdateClicked()
{
    // Open dialog to select installation time
    QDateTime suggestedTime = QDateTime::currentDateTime().addSecs(86400); // Tomorrow
    scheduleUpdate(suggestedTime);
}

void UpdateWindow::onCancelClicked()
{
    cancelUpdate();
}

void UpdateWindow::onCheckStarted()
{
    checkUpdatesButton->setEnabled(false);
    cancelButton->setEnabled(true);
    systemStatusLabel->setText("Checking for updates...");
}

void UpdateWindow::onCheckFinished(bool updatesAvailable)
{
    checkUpdatesButton->setEnabled(true);
    cancelButton->setEnabled(false);
    
    if (updatesAvailable) {
        systemStatusLabel->setText("Updates available");
        displayAvailableUpdates();
    } else {
        systemStatusLabel->setText("Your system is up to date");
    }
}

void UpdateWindow::onUpdateAvailable(const UpdateInfo &info)
{
    displayUpdateDetails(info);
}

void UpdateWindow::onCheckFailed(const QString &error)
{
    checkUpdatesButton->setEnabled(true);
    cancelButton->setEnabled(false);
    systemStatusLabel->setText(QString("Check failed: %1").arg(error));
}

void UpdateWindow::onProgressUpdated(int percent)
{
    installProgress->setValue(percent);
}

void UpdateWindow::onUpdateListItemSelected()
{
    const auto idx = updateListWidget->row(updateListWidget->currentItem());
    auto updates = updateChecker->getPendingUpdates();
    if (idx >= 0 && idx < updates.size()) {
        displayUpdateDetails(updates[idx]);
    }
}

void UpdateWindow::onAutoCheckToggled(bool checked)
{
    autoCheckEnabled = checked;
}

void UpdateWindow::checkForUpdates()
{
    updateChecker->checkForUpdates();
}

void UpdateWindow::installUpdates()
{
    installProgress->setVisible(true);
    installButton->setEnabled(false);
    systemStatusLabel->setText("Installing update...");
    
    // Simulate installation (in production, would call system update mechanism)
    for (int i = 0; i <= 100; i += 5) {
        installProgress->setValue(i);
        QApplication::processEvents();
    }
    
    systemStatusLabel->setText("Update installed successfully");
    displayCurrentVersion();
}

void UpdateWindow::scheduleUpdate(const QDateTime &time)
{
    updateScheduled = true;
    systemStatusLabel->setText(QString("Update scheduled for %1").arg(time.toString()));
}

void UpdateWindow::cancelUpdate()
{
    updateScheduled = false;
    systemStatusLabel->setText("Update cancelled");
    installProgress->setVisible(false);
    installButton->setEnabled(true);
}

QString UpdateWindow::formatSize(int sizeInMB) const
{
    if (sizeInMB < 1024) {
        return QString("%1 MB").arg(sizeInMB);
    } else {
        return QString("%1 GB").arg(sizeInMB / 1024.0, 0, 'f', 1);
    }
}

QString UpdateWindow::getTimeEstimate(int sizeInMB) const
{
    // Estimate based on typical network speed (assume 5 MB/s)
    const int seconds = sizeInMB * 1000 / 5000;
    
    if (seconds < 60) {
        return QString("%1 seconds").arg(seconds);
    } else if (seconds < 3600) {
        return QString("%1 minutes").arg(seconds / 60);
    } else {
        return QString("%1 hours").arg(seconds / 3600);
    }
}

void UpdateWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}

} // namespace Arbor::UpdateManager
