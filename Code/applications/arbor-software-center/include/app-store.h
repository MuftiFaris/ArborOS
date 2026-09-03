#ifndef ARBOR_SOFTWARE_CENTER_APP_STORE_H
#define ARBOR_SOFTWARE_CENTER_APP_STORE_H

#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QTabWidget>
#include <QString>
#include <QList>
#include <QProcess>
#include <memory>

namespace Arbor::SoftwareCenter {

/**
 * @struct AppInfo
 * @brief Represents a single application in the repository
 */
struct AppInfo {
    QString id;              ///< Unique app identifier
    QString name;            ///< Display name
    QString description;     ///< Short description
    QString category;        ///< Category (Games, Utilities, etc.)
    QString version;         ///< Current version
    QString installedVersion; ///< Installed version (empty if not installed)
    QString author;          ///< Author/maintainer name
    int downloadSize;        ///< Size in MB
    double rating;           ///< User rating (0-5)
    bool isInstalled;        ///< Installation status
    QString iconPath;        ///< Path to app icon
    QString type;            ///< "flatpak" or "native"
};

/**
 * @class AppStore
 * @brief Main window for Arbor Software Center application
 *
 * Features:
 * - Browse and search for applications
 * - View app details and ratings
 * - Install/uninstall applications via Flatpak
 * - Categorized browsing (Games, Utilities, Productivity, etc.)
 * - Update management for installed apps
 * - Installation progress tracking
 */
class AppStore : public QMainWindow {
    Q_OBJECT

public:
    explicit AppStore(QWidget *parent = nullptr);
    ~AppStore();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    /// Search/filter functionality
    void onSearchTextChanged(const QString &text);
    void onCategorySelected(int index);
    
    /// App selection and details
    void onAppSelected(QListWidgetItem *item);
    void onAppDoubleClicked(QListWidgetItem *item);
    
    /// Installation management
    void onInstallButtonClicked();
    void onUninstallButtonClicked();
    void onUpdateButtonClicked();
    
    /// Process management
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onProcessOutput();
    
    /// Tab switching
    void onTabChanged(int index);

private:
    /// UI setup
    void setupUI();
    void createMenuBar();
    void createSearchBar();
    void createCategoryTabs();
    void createAppList();
    void createDetailsPanel();
    void createStatusBar();
    
    /// Data management
    void loadApplications();
    void loadInstalledApps();
    void filterApps(const QString &query = "");
    void displayAppDetails(const AppInfo &app);
    void updateAppStatus(const AppInfo &app);
    
    /// Flatpak operations
    bool isFlatpakInstalled() const;
    bool isAppInstalled(const QString &appId) const;
    void installApp(const QString &appId);
    void uninstallApp(const QString &appId);
    void updateApp(const QString &appId);
    
    /// Helper methods
    QString formatSize(int sizeInMB) const;
    QString getRatingStars(double rating) const;
    
    // UI widgets
    QLineEdit *searchInput;
    QComboBox *categoryFilter;
    QListWidget *appListWidget;
    QLabel *appNameLabel;
    QLabel *appAuthorLabel;
    QLabel *appDescriptionLabel;
    QLabel *appRatingLabel;
    QLabel *appSizeLabel;
    QLabel *appCategoryLabel;
    QLabel *appTypeLabel;
    QPushButton *installButton;
    QPushButton *uninstallButton;
    QPushButton *updateButton;
    QProgressBar *installProgress;
    QLabel *statusLabel;
    QTabWidget *tabWidget;
    
    // Data
    QList<AppInfo> allApps;
    QList<AppInfo> filteredApps;
    AppInfo currentApp;
    std::unique_ptr<QProcess> flatpakProcess;
    
    // Settings
    QString currentCategory;
    bool showInstalledOnly;
};

} // namespace Arbor::SoftwareCenter

#endif // ARBOR_SOFTWARE_CENTER_APP_STORE_H
