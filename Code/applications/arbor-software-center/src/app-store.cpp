#include "app-store.h"
#include "../include/style-provider.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QClipboard>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>
#include <QSplitter>
#include <QScrollArea>
#include <QLabel>
#include <QPixmap>
#include <QCloseEvent>

namespace Arbor::SoftwareCenter {

AppStore::AppStore(QWidget *parent)
    : QMainWindow(parent),
      searchInput(nullptr),
      categoryFilter(nullptr),
      appListWidget(nullptr),
      flatpakProcess(std::make_unique<QProcess>(this)),
      currentCategory("All"),
      showInstalledOnly(false)
{
    setWindowTitle("Arbor Software Center");
    setWindowIcon(QIcon(":/icons/app-store.svg"));
    setGeometry(100, 100, 1200, 700);
    
    setupUI();
    loadApplications();
    loadInstalledApps();
    
    // Apply design system theme
    Arbor::DesignSystem::StyleProvider::instance()->applyStyle(this);
    
    connect(flatpakProcess.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AppStore::onProcessFinished);
    connect(flatpakProcess.get(), &QProcess::readyReadStandardOutput,
            this, &AppStore::onProcessOutput);
    connect(flatpakProcess.get(), QOverload<QProcess::ProcessError>::of(&QProcess::error),
            this, &AppStore::onProcessError);
}

AppStore::~AppStore() = default;

void AppStore::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);
    
    createSearchBar();
    createCategoryTabs();
    createAppList();
    createDetailsPanel();
    createStatusBar();
    createMenuBar();
    
    // Main content area: splitter with app list and details
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(appListWidget);
    
    // Details panel
    QWidget *detailsPanel = new QWidget;
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsPanel);
    detailsLayout->setSpacing(12);
    detailsLayout->addWidget(appNameLabel);
    detailsLayout->addWidget(appAuthorLabel);
    detailsLayout->addWidget(appDescriptionLabel);
    detailsLayout->addWidget(appRatingLabel);
    detailsLayout->addWidget(appSizeLabel);
    detailsLayout->addWidget(appCategoryLabel);
    detailsLayout->addWidget(appTypeLabel);
    
    // Button row
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(installButton);
    buttonLayout->addWidget(uninstallButton);
    buttonLayout->addWidget(updateButton);
    
    detailsLayout->addLayout(buttonLayout);
    detailsLayout->addWidget(installProgress);
    detailsLayout->addStretch();
    
    splitter->addWidget(detailsPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(splitter);
}

void AppStore::createMenuBar()
{
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // File menu
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    QAction *refreshAction = fileMenu->addAction(tr("&Refresh"));
    connect(refreshAction, &QAction::triggered, this, &AppStore::loadApplications);
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction(tr("E&xit"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Edit menu
    QMenu *editMenu = menuBar->addMenu(tr("&Edit"));
    QAction *prefsAction = editMenu->addAction(tr("&Preferences"));
    connect(prefsAction, &QAction::triggered, [this]() {
        statusLabel->setText("Preferences not yet implemented");
    });
    
    // View menu
    QMenu *viewMenu = menuBar->addMenu(tr("&View"));
    QAction *installedAction = viewMenu->addAction(tr("Show &Installed Only"));
    installedAction->setCheckable(true);
    connect(installedAction, &QAction::triggered, [this](bool checked) {
        showInstalledOnly = checked;
        filterApps(searchInput->text());
    });
    
    // Help menu
    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));
    QAction *aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered, [this]() {
        statusLabel->setText("Arbor Software Center v1.0 - Application Store");
    });
}

void AppStore::createSearchBar()
{
    QHBoxLayout *searchLayout = new QHBoxLayout;
    
    QLabel *searchLabel = new QLabel(tr("Search:"));
    searchInput = new QLineEdit;
    searchInput->setPlaceholderText(tr("Search applications..."));
    searchInput->setMinimumHeight(36);
    
    connect(searchInput, &QLineEdit::textChanged, this, &AppStore::onSearchTextChanged);
    
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchInput);
    
    QWidget *container = new QWidget;
    container->setLayout(searchLayout);
    
    QMainWindow::centralWidget()->layout()->insertWidget(0, container);
}

void AppStore::createCategoryTabs()
{
    QHBoxLayout *categoryLayout = new QHBoxLayout;
    
    QLabel *categoryLabel = new QLabel(tr("Category:"));
    categoryFilter = new QComboBox;
    categoryFilter->addItems(QStringList() << "All" << "Games" << "Utilities" << "Productivity"
                             << "Graphics" << "Development" << "Network" << "Office");
    
    connect(categoryFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AppStore::onCategorySelected);
    
    categoryLayout->addWidget(categoryLabel);
    categoryLayout->addWidget(categoryFilter);
    categoryLayout->addStretch();
    
    QWidget *container = new QWidget;
    container->setLayout(categoryLayout);
    
    QMainWindow::centralWidget()->layout()->insertWidget(1, container);
}

void AppStore::createAppList()
{
    appListWidget = new QListWidget;
    appListWidget->setMinimumWidth(300);
    
    connect(appListWidget, &QListWidget::itemSelectionChanged, [this]() {
        if (appListWidget->currentItem()) {
            const auto idx = appListWidget->row(appListWidget->currentItem());
            if (idx >= 0 && idx < filteredApps.size()) {
                displayAppDetails(filteredApps[idx]);
            }
        }
    });
    
    connect(appListWidget, &QListWidget::itemDoubleClicked, this, &AppStore::onAppDoubleClicked);
}

void AppStore::createDetailsPanel()
{
    appNameLabel = new QLabel;
    appNameLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    
    appAuthorLabel = new QLabel;
    appDescriptionLabel = new QLabel;
    appDescriptionLabel->setWordWrap(true);
    appRatingLabel = new QLabel;
    appSizeLabel = new QLabel;
    appCategoryLabel = new QLabel;
    appTypeLabel = new QLabel;
    
    installButton = new QPushButton(tr("Install"));
    uninstallButton = new QPushButton(tr("Uninstall"));
    updateButton = new QPushButton(tr("Update"));
    
    installProgress = new QProgressBar;
    installProgress->setMaximum(100);
    installProgress->setValue(0);
    installProgress->setVisible(false);
    
    connect(installButton, &QPushButton::clicked, this, &AppStore::onInstallButtonClicked);
    connect(uninstallButton, &QPushButton::clicked, this, &AppStore::onUninstallButtonClicked);
    connect(updateButton, &QPushButton::clicked, this, &AppStore::onUpdateButtonClicked);
}

void AppStore::createStatusBar()
{
    statusLabel = new QLabel(tr("Ready"));
    statusBar()->addWidget(statusLabel, 1);
}

void AppStore::onSearchTextChanged(const QString &text)
{
    filterApps(text);
}

void AppStore::onCategorySelected(int index)
{
    if (categoryFilter) {
        currentCategory = categoryFilter->currentText();
        filterApps(searchInput->text());
    }
}

void AppStore::onAppSelected(QListWidgetItem *item)
{
    if (item) {
        const auto idx = appListWidget->row(item);
        if (idx >= 0 && idx < filteredApps.size()) {
            displayAppDetails(filteredApps[idx]);
        }
    }
}

void AppStore::onAppDoubleClicked(QListWidgetItem *item)
{
    onAppSelected(item);
    if (currentApp.isInstalled) {
        onUninstallButtonClicked();
    } else {
        onInstallButtonClicked();
    }
}

void AppStore::onInstallButtonClicked()
{
    if (currentApp.id.isEmpty()) {
        statusLabel->setText("No app selected");
        return;
    }
    
    installApp(currentApp.id);
}

void AppStore::onUninstallButtonClicked()
{
    if (currentApp.id.isEmpty()) {
        statusLabel->setText("No app selected");
        return;
    }
    
    uninstallApp(currentApp.id);
}

void AppStore::onUpdateButtonClicked()
{
    if (currentApp.id.isEmpty()) {
        statusLabel->setText("No app selected");
        return;
    }
    
    updateApp(currentApp.id);
}

void AppStore::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    installProgress->setVisible(false);
    
    if (exitCode == 0) {
        statusLabel->setText("Operation completed successfully");
        loadInstalledApps();
        updateAppStatus(currentApp);
    } else {
        statusLabel->setText("Operation failed");
    }
}

void AppStore::onProcessError(QProcess::ProcessError error)
{
    installProgress->setVisible(false);
    statusLabel->setText(QString("Process error: %1").arg(static_cast<int>(error)));
}

void AppStore::onProcessOutput()
{
    const auto output = flatpakProcess->readAllStandardOutput();
    qDebug() << "Process output:" << output;
}

void AppStore::onTabChanged(int index)
{
    // Tab switching logic if needed
}

void AppStore::loadApplications()
{
    allApps.clear();
    
    // Sample app repository (would be loaded from remote source in production)
    allApps << AppInfo{
        "org.gnome.Nautilus", "Files", "File manager", "Utilities",
        "43.0", "", "GNOME", 45, 4.5, false, ":/icons/files.svg", "flatpak"
    }
    << AppInfo{
        "com.github.gedit.gedit", "gedit", "Text editor", "Development",
        "43.0", "", "GNOME", 35, 4.2, false, ":/icons/text-editor.svg", "flatpak"
    }
    << AppInfo{
        "org.gimp.GIMP", "GIMP", "Image editor", "Graphics",
        "2.10.30", "", "GIMP Team", 180, 4.7, false, ":/icons/gimp.svg", "flatpak"
    }
    << AppInfo{
        "com.spotify.Client", "Spotify", "Music streaming", "Multimedia",
        "1.2.0", "", "Spotify", 220, 4.6, false, ":/icons/spotify.svg", "flatpak"
    }
    << AppInfo{
        "com.google.Chrome", "Google Chrome", "Web browser", "Network",
        "120.0", "", "Google", 280, 4.8, false, ":/icons/chrome.svg", "flatpak"
    }
    << AppInfo{
        "org.mozilla.firefox", "Firefox", "Web browser", "Network",
        "121.0", "", "Mozilla", 250, 4.7, false, ":/icons/firefox.svg", "flatpak"
    }
    << AppInfo{
        "org.libreoffice.LibreOffice", "LibreOffice", "Office suite", "Office",
        "7.5.0", "", "LibreOffice", 420, 4.4, false, ":/icons/libreoffice.svg", "flatpak"
    }
    << AppInfo{
        "com.valvesoftware.Steam", "Steam", "Game platform", "Games",
        "2.0", "", "Valve", 850, 4.5, false, ":/icons/steam.svg", "flatpak"
    };
    
    filterApps();
    statusLabel->setText(QString("Loaded %1 applications").arg(allApps.size()));
}

void AppStore::loadInstalledApps()
{
    // In production, would query flatpak list --app
    for (auto &app : allApps) {
        app.isInstalled = isAppInstalled(app.id);
    }
}

void AppStore::filterApps(const QString &query)
{
    filteredApps.clear();
    
    for (const auto &app : allApps) {
        // Filter by category
        if (currentCategory != "All" && app.category != currentCategory) {
            continue;
        }
        
        // Filter by installed status
        if (showInstalledOnly && !app.isInstalled) {
            continue;
        }
        
        // Filter by search query
        if (!query.isEmpty() &&
            !app.name.contains(query, Qt::CaseInsensitive) &&
            !app.description.contains(query, Qt::CaseInsensitive) &&
            !app.id.contains(query, Qt::CaseInsensitive)) {
            continue;
        }
        
        filteredApps.append(app);
    }
    
    // Update list widget
    appListWidget->clear();
    for (const auto &app : filteredApps) {
        const auto displayText = QString("%1 [%2]").arg(app.name, app.type);
        auto item = new QListWidgetItem(displayText, appListWidget);
        if (app.isInstalled) {
            item->setForeground(QBrush(QColor(76, 175, 80))); // Green for installed
        }
    }
}

void AppStore::displayAppDetails(const AppInfo &app)
{
    currentApp = app;
    
    appNameLabel->setText(app.name);
    appAuthorLabel->setText(QString("By: %1").arg(app.author));
    appDescriptionLabel->setText(app.description);
    appRatingLabel->setText(QString("Rating: %1/5 ⭐").arg(app.rating));
    appSizeLabel->setText(QString("Size: %1 MB").arg(app.downloadSize));
    appCategoryLabel->setText(QString("Category: %1").arg(app.category));
    appTypeLabel->setText(QString("Type: %1").arg(app.type));
    
    installButton->setEnabled(!app.isInstalled);
    uninstallButton->setEnabled(app.isInstalled);
    updateButton->setEnabled(app.isInstalled && app.version != app.installedVersion);
    
    statusLabel->setText(app.isInstalled ? "Installed" : "Not installed");
}

void AppStore::updateAppStatus(const AppInfo &app)
{
    currentApp = app;
    displayAppDetails(app);
}

bool AppStore::isFlatpakInstalled() const
{
    QProcess proc;
    proc.start("flatpak", QStringList() << "--version");
    return proc.waitForFinished() && proc.exitCode() == 0;
}

bool AppStore::isAppInstalled(const QString &appId) const
{
    QProcess proc;
    proc.start("flatpak", QStringList() << "info" << appId);
    return proc.waitForFinished() && proc.exitCode() == 0;
}

void AppStore::installApp(const QString &appId)
{
    if (!isFlatpakInstalled()) {
        statusLabel->setText("Flatpak not installed");
        return;
    }
    
    statusLabel->setText("Installing...");
    installProgress->setVisible(true);
    installButton->setEnabled(false);
    
    flatpakProcess->start("flatpak", QStringList() << "install" << "-y" << appId);
}

void AppStore::uninstallApp(const QString &appId)
{
    if (!isFlatpakInstalled()) {
        statusLabel->setText("Flatpak not installed");
        return;
    }
    
    statusLabel->setText("Uninstalling...");
    installProgress->setVisible(true);
    uninstallButton->setEnabled(false);
    
    flatpakProcess->start("flatpak", QStringList() << "uninstall" << "-y" << appId);
}

void AppStore::updateApp(const QString &appId)
{
    if (!isFlatpakInstalled()) {
        statusLabel->setText("Flatpak not installed");
        return;
    }
    
    statusLabel->setText("Updating...");
    installProgress->setVisible(true);
    updateButton->setEnabled(false);
    
    flatpakProcess->start("flatpak", QStringList() << "update" << "-y" << appId);
}

QString AppStore::formatSize(int sizeInMB) const
{
    if (sizeInMB < 1024) {
        return QString("%1 MB").arg(sizeInMB);
    } else {
        return QString("%1 GB").arg(sizeInMB / 1024.0, 0, 'f', 1);
    }
}

QString AppStore::getRatingStars(double rating) const
{
    QString stars;
    for (int i = 0; i < static_cast<int>(rating); ++i) {
        stars += "⭐";
    }
    return stars;
}

void AppStore::closeEvent(QCloseEvent *event)
{
    event->accept();
}

} // namespace Arbor::SoftwareCenter
