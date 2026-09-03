#include "settings-window.h"
#include "settings-model.h"
#include "system-info.h"
#include "theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QGroupBox>
#include <QSlider>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMenuBar>
#include <QStatusBar>
#include <QSettings>
#include <QDebug>

SettingsWindow::SettingsWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_searchBox(nullptr)
    , m_categoryList(nullptr)
    , m_contentStack(nullptr)
    , m_statusLabel(nullptr)
    , m_currentPanelIndex(0)
{
    setObjectName("SettingsWindow");

    m_settingsModel = std::make_unique<SettingsModel>(this);

    setupUI();
    setupMenuBar();
    setupStatusBar();
    setupConnections();

    restoreState();
}

SettingsWindow::~SettingsWindow() {
    saveState();
}

void SettingsWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Search bar
    mainLayout->addLayout(setupSearchBar());

    // Content area (2-pane)
    QHBoxLayout* contentLayout = new QHBoxLayout();

    // Left sidebar
    setupSidebar();
    contentLayout->addWidget(m_categoryList, 1);

    // Right content
    setupContentArea();
    contentLayout->addWidget(m_contentStack, 2);

    mainLayout->addLayout(contentLayout);

    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

QHBoxLayout* SettingsWindow::setupSearchBar() {
    QHBoxLayout* layout = new QHBoxLayout();

    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("Search settings... (Ctrl+K)");
    m_searchBox->setMaximumHeight(36);

    layout->addWidget(m_searchBox);

    return layout;
}

void SettingsWindow::setupSidebar() {
    m_categoryList = new QListWidget(this);
    m_categoryList->setMaximumWidth(200);
    m_categoryList->setMinimumWidth(160);

    // Add categories
    m_categoryList->addItem("System");
    m_categoryList->addItem("Security & Privacy");
    m_categoryList->addItem("Personalization");
    m_categoryList->addItem("Applications");
    m_categoryList->addItem("About");

    m_categoryList->setCurrentRow(0);
}

void SettingsWindow::setupContentArea() {
    m_contentStack = new QStackedWidget(this);

    // Create panels
    createSystemPanel();
    createSecurityPanel();
    createPersonalizationPanel();
    createApplicationsPanel();
    createAboutPanel();
}

void SettingsWindow::createSystemPanel() {
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);

    // Display section
    QGroupBox* displayGroup = new QGroupBox("Display", this);
    QVBoxLayout* displayLayout = new QVBoxLayout(displayGroup);

    QHBoxLayout* scalingLayout = new QHBoxLayout();
    scalingLayout->addWidget(new QLabel("Scaling:"));
    QSpinBox* scalingSpinBox = new QSpinBox(this);
    scalingSpinBox->setSuffix("%");
    scalingSpinBox->setRange(100, 200);
    scalingSpinBox->setValue(m_settingsModel->getDisplayScaling());
    scalingLayout->addWidget(scalingSpinBox);
    scalingLayout->addStretch();
    displayLayout->addLayout(scalingLayout);

    displayGroup->setLayout(displayLayout);
    layout->addWidget(displayGroup);

    // Sound section
    QGroupBox* soundGroup = new QGroupBox("Sound", this);
    QVBoxLayout* soundLayout = new QVBoxLayout(soundGroup);

    QHBoxLayout* volumeLayout = new QHBoxLayout();
    volumeLayout->addWidget(new QLabel("Volume:"));
    QSlider* volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(m_settingsModel->getVolume());
    volumeLayout->addWidget(volumeSlider);
    soundLayout->addLayout(volumeLayout);

    soundGroup->setLayout(soundLayout);
    layout->addWidget(soundGroup);

    // Power section
    QGroupBox* powerGroup = new QGroupBox("Power", this);
    QVBoxLayout* powerLayout = new QVBoxLayout(powerGroup);

    QHBoxLayout* profileLayout = new QHBoxLayout();
    profileLayout->addWidget(new QLabel("Power Profile:"));
    QComboBox* profileCombo = new QComboBox(this);
    profileCombo->addItems(QStringList() << "Balanced" << "Performance" << "Power Saver");
    profileLayout->addWidget(profileCombo);
    profileLayout->addStretch();
    powerLayout->addLayout(profileLayout);

    powerGroup->setLayout(powerLayout);
    layout->addWidget(powerGroup);

    layout->addStretch();
    panel->setLayout(layout);
    m_contentStack->addWidget(panel);
}

void SettingsWindow::createSecurityPanel() {
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);

    // Privacy section
    QGroupBox* privacyGroup = new QGroupBox("Privacy", this);
    QVBoxLayout* privacyLayout = new QVBoxLayout(privacyGroup);

    QCheckBox* telemetryCheck = new QCheckBox("Enable Telemetry (Off by default)", this);
    privacyLayout->addWidget(telemetryCheck);

    QCheckBox* macRandomCheck = new QCheckBox("Random MAC Address (On by default)", this);
    macRandomCheck->setChecked(true);
    privacyLayout->addWidget(macRandomCheck);

    privacyGroup->setLayout(privacyLayout);
    layout->addWidget(privacyGroup);

    // Firewall section
    QGroupBox* firewallGroup = new QGroupBox("Firewall", this);
    QVBoxLayout* firewallLayout = new QVBoxLayout(firewallGroup);

    QCheckBox* firewallCheck = new QCheckBox("Enable Firewall", this);
    firewallCheck->setChecked(true);
    firewallLayout->addWidget(firewallCheck);

    firewallGroup->setLayout(firewallLayout);
    layout->addWidget(firewallGroup);

    // Encryption section
    QGroupBox* encryptionGroup = new QGroupBox("Encryption", this);
    QVBoxLayout* encryptionLayout = new QVBoxLayout(encryptionGroup);

    QLabel* diskStatusLabel = new QLabel(QString("Disk Encrypted: %1")
        .arg(SystemInfo::isDiskEncrypted() ? "Yes" : "No"), this);
    encryptionLayout->addWidget(diskStatusLabel);

    QLabel* tpmStatusLabel = new QLabel(QString("TPM Available: %1")
        .arg(SystemInfo::hasTPM() ? "Yes" : "No"), this);
    encryptionLayout->addWidget(tpmStatusLabel);

    encryptionGroup->setLayout(encryptionLayout);
    layout->addWidget(encryptionGroup);

    layout->addStretch();
    panel->setLayout(layout);
    m_contentStack->addWidget(panel);
}

void SettingsWindow::createPersonalizationPanel() {
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);

    // Appearance section
    QGroupBox* appearanceGroup = new QGroupBox("Appearance", this);
    QVBoxLayout* appearanceLayout = new QVBoxLayout(appearanceGroup);

    QHBoxLayout* themeLayout = new QHBoxLayout();
    themeLayout->addWidget(new QLabel("Theme:"));
    QComboBox* themeCombo = new QComboBox(this);
    themeCombo->addItems(QStringList() << "Dark" << "Light" << "Auto");
    themeLayout->addWidget(themeCombo);
    themeLayout->addStretch();
    appearanceLayout->addLayout(themeLayout);

    appearanceGroup->setLayout(appearanceLayout);
    layout->addWidget(appearanceGroup);

    // Wallpaper section
    QGroupBox* wallpaperGroup = new QGroupBox("Wallpaper", this);
    QVBoxLayout* wallpaperLayout = new QVBoxLayout(wallpaperGroup);

    QPushButton* selectWallpaper = new QPushButton("Select Wallpaper...", this);
    wallpaperLayout->addWidget(selectWallpaper);

    wallpaperGroup->setLayout(wallpaperLayout);
    layout->addWidget(wallpaperGroup);

    layout->addStretch();
    panel->setLayout(layout);
    m_contentStack->addWidget(panel);
}

void SettingsWindow::createApplicationsPanel() {
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);

    // Default applications
    QGroupBox* defaultsGroup = new QGroupBox("Default Applications", this);
    QVBoxLayout* defaultsLayout = new QVBoxLayout(defaultsGroup);

    QComboBox* browserCombo = new QComboBox(this);
    browserCombo->addItems(QStringList() << "Firefox" << "Chromium" << "Safari");
    defaultsLayout->addWidget(new QLabel("Web Browser:"));
    defaultsLayout->addWidget(browserCombo);

    defaultsGroup->setLayout(defaultsLayout);
    layout->addWidget(defaultsGroup);

    layout->addStretch();
    panel->setLayout(layout);
    m_contentStack->addWidget(panel);
}

void SettingsWindow::createAboutPanel() {
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);

    QGroupBox* systemGroup = new QGroupBox("System Information", this);
    QVBoxLayout* systemLayout = new QVBoxLayout(systemGroup);

    systemLayout->addWidget(new QLabel(QString("OS: %1").arg(SystemInfo::getOSName())));
    systemLayout->addWidget(new QLabel(QString("Version: %1").arg(SystemInfo::getOSVersion())));
    systemLayout->addWidget(new QLabel(QString("Kernel: %1").arg(SystemInfo::getKernelVersion())));
    systemLayout->addWidget(new QLabel(QString("Uptime: %1").arg(SystemInfo::getUptimeFormatted())));

    systemGroup->setLayout(systemLayout);
    layout->addWidget(systemGroup);

    QGroupBox* hardwareGroup = new QGroupBox("Hardware", this);
    QVBoxLayout* hardwareLayout = new QVBoxLayout(hardwareGroup);

    hardwareLayout->addWidget(new QLabel(QString("CPU: %1").arg(SystemInfo::getCPUModel())));
    hardwareLayout->addWidget(new QLabel(QString("Cores: %1").arg(SystemInfo::getCPUCoreCount())));
    hardwareLayout->addWidget(new QLabel(QString("Memory: %1 GB").arg(SystemInfo::getTotalMemory() / 1024 / 1024 / 1024)));
    hardwareLayout->addWidget(new QLabel(QString("GPU: %1").arg(SystemInfo::getGPUModel())));

    hardwareGroup->setLayout(hardwareLayout);
    layout->addWidget(hardwareGroup);

    layout->addStretch();
    panel->setLayout(layout);
    m_contentStack->addWidget(panel);
}

void SettingsWindow::setupMenuBar() {
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu* fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Quit"), this, &QWidget::close, QKeySequence::Quit);

    QMenu* helpMenu = menuBar->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, [this]() {
        qDebug() << "Arbor Settings v1.0.0";
    });
}

void SettingsWindow::setupStatusBar() {
    m_statusLabel = new QLabel(this);
    m_statusLabel->setText("Ready");
    statusBar()->addWidget(m_statusLabel);
}

void SettingsWindow::setupConnections() {
    connect(m_categoryList, &QListWidget::currentRowChanged, this, &SettingsWindow::onCategorySelected);
    connect(m_searchBox, &QLineEdit::textChanged, this, &SettingsWindow::onSearchTextChanged);
    connect(m_settingsModel.get(), &SettingsModel::settingChanged, this, &SettingsWindow::onSettingChanged);
}

void SettingsWindow::closeEvent(QCloseEvent* event) {
    saveState();
    QMainWindow::closeEvent(event);
}

void SettingsWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_K && event->modifiers() & Qt::ControlModifier) {
        m_searchBox->setFocus();
        m_searchBox->selectAll();
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void SettingsWindow::onCategorySelected(int index) {
    m_contentStack->setCurrentIndex(index);
    m_statusLabel->setText(QString("Category: %1").arg(m_categoryList->item(index)->text()));
}

void SettingsWindow::onSearchTextChanged(const QString& text) {
    filterCategories(text);
}

void SettingsWindow::onSearchClear() {
    m_searchBox->clear();
}

void SettingsWindow::onSettingChanged(const QString& key, const QVariant& value) {
    qDebug() << "Setting changed:" << key << "=" << value;
}

void SettingsWindow::onApplySettings() {
    m_statusLabel->setText("Settings applied");
}

void SettingsWindow::onResetSettings() {
    m_statusLabel->setText("Settings reset");
}

void SettingsWindow::filterCategories(const QString& filter) {
    for (int i = 0; i < m_categoryList->count(); ++i) {
        QListWidgetItem* item = m_categoryList->item(i);
        item->setHidden(!item->text().contains(filter, Qt::CaseInsensitive));
    }
}

void SettingsWindow::showCategory(const QString& categoryName) {
    for (int i = 0; i < m_categoryList->count(); ++i) {
        if (m_categoryList->item(i)->text() == categoryName) {
            m_categoryList->setCurrentRow(i);
            break;
        }
    }
}

void SettingsWindow::restoreState() {
    QSettings settings("ArborOS", "ArborSettings");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void SettingsWindow::saveState() {
    QSettings settings("ArborOS", "ArborSettings");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}
