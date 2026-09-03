#ifndef ARBOR_UPDATE_MANAGER_UPDATE_WINDOW_H
#define ARBOR_UPDATE_MANAGER_UPDATE_WINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QListWidget>
#include <QString>
#include <memory>
#include "update-checker.h"

namespace Arbor::UpdateManager {

/**
 * @class UpdateWindow
 * @brief Main UI for system update management
 *
 * Displays:
 * - Current system version
 * - Available updates
 * - Download and install timeline
 * - Installation progress
 * - Release notes
 */
class UpdateWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit UpdateWindow(QWidget *parent = nullptr);
    ~UpdateWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    /// Update actions
    void onCheckUpdatesClicked();
    void onInstallUpdatesClicked();
    void onScheduleUpdateClicked();
    void onCancelClicked();
    
    /// Update checker signals
    void onCheckStarted();
    void onCheckFinished(bool updatesAvailable);
    void onUpdateAvailable(const UpdateInfo &info);
    void onCheckFailed(const QString &error);
    void onProgressUpdated(int percent);
    
    /// UI updates
    void onUpdateListItemSelected();
    void onAutoCheckToggled(bool checked);

private:
    /// UI setup
    void setupUI();
    void createMenuBar();
    void createStatusPanel();
    void createUpdateList();
    void createDetailsPanel();
    void createInstallTimeline();
    void createStatusBar();
    
    /// Data update
    void displayCurrentVersion();
    void displayAvailableUpdates();
    void displayUpdateDetails(const UpdateInfo &info);
    void displayInstallationTimeline();
    
    /// Update operations
    void checkForUpdates();
    void installUpdates();
    void scheduleUpdate(const QDateTime &time);
    void cancelUpdate();
    
    /// Helper methods
    QString formatSize(int sizeInMB) const;
    QString getTimeEstimate(int sizeInMB) const;
    
    // UI widgets
    QLabel *currentVersionLabel;
    QLabel *systemStatusLabel;
    QPushButton *checkUpdatesButton;
    QPushButton *installButton;
    QPushButton *scheduleButton;
    QPushButton *cancelButton;
    
    QListWidget *updateListWidget;
    QLabel *updateNameLabel;
    QLabel *updateSizeLabel;
    QLabel *updateDateLabel;
    QLabel *updateSecurityLabel;
    QTextEdit *releaseNotesEdit;
    
    QLabel *timelineLabel;
    QProgressBar *installProgress;
    QLabel *estimatedTimeLabel;
    
    // Update checker
    std::unique_ptr<UpdateChecker> updateChecker;
    
    // State
    UpdateInfo selectedUpdate;
    bool autoCheckEnabled;
    bool updateScheduled;
};

} // namespace Arbor::UpdateManager

#endif // ARBOR_UPDATE_MANAGER_UPDATE_WINDOW_H
