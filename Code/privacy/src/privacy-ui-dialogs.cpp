#include "privacy-ui-dialogs.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QIcon>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QHeaderView>
#include <QPropertyAnimation>

// ============================================================================
// PermissionRequestDialog Implementation
// ============================================================================

PermissionRequestDialog::PermissionRequestDialog(
    const QString& appName,
    const QString& appId,
    PermissionCategory category,
    const QString& reason,
    QWidget* parent)
    : QDialog(parent), m_allowed(false), m_appName(appName), m_appId(appId),
      m_category(category), m_reason(reason)
{
    setWindowTitle("Permission Request");
    setModal(true);
    setMinimumWidth(450);
    setupUI();
}

void PermissionRequestDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Icon and app name
    QHBoxLayout* headerLayout = new QHBoxLayout();
    m_appIconLabel = new QLabel();
    m_appIconLabel->setPixmap(QPixmap(":/images/app-icon.png").scaledToWidth(64));
    headerLayout->addWidget(m_appIconLabel);

    QLabel* appNameLabel = new QLabel(QString("<b>%1</b> requests permission").arg(m_appName));
    QFont font = appNameLabel->font();
    font.setPointSize(12);
    appNameLabel->setFont(font);
    headerLayout->addWidget(appNameLabel);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // Category-specific description
    QString categoryDesc;
    QString riskWarning;

    switch (m_category) {
        case Microphone:
            categoryDesc = "This application wants to access your <b>microphone</b>.";
            riskWarning = "The application will be able to record audio and access all sounds from your microphone.";
            break;
        case Camera:
            categoryDesc = "This application wants to access your <b>camera</b>.";
            riskWarning = "The application will be able to record video and capture images using your camera.";
            break;
        case FileAccess:
            categoryDesc = "This application wants to access your <b>files</b>.";
            riskWarning = "The application will be able to read, modify, or delete files in selected directories.";
            break;
        case Network:
            categoryDesc = "This application wants to access the <b>network</b>.";
            riskWarning = "The application will be able to send and receive data over the internet.";
            break;
        case Location:
            categoryDesc = "This application wants to access your <b>location</b>.";
            riskWarning = "The application will be able to determine your approximate geographic location.";
            break;
    }

    m_descriptionLabel = new QLabel(categoryDesc);
    m_descriptionLabel->setWordWrap(true);
    mainLayout->addWidget(m_descriptionLabel);

    // Reason (if provided)
    if (!m_reason.isEmpty()) {
        QLabel* reasonLabel = new QLabel(QString("Reason: <i>%1</i>").arg(m_reason));
        reasonLabel->setWordWrap(true);
        mainLayout->addWidget(reasonLabel);
    }

    // Risk warning
    m_riskLabel = new QLabel(riskWarning);
    m_riskLabel->setStyleSheet("QLabel { background-color: #fff3cd; padding: 8px; border-radius: 4px; color: #856404; }");
    m_riskLabel->setWordWrap(true);
    mainLayout->addWidget(m_riskLabel);

    // Why this permission
    m_whyLabel = new QLabel("<b>Why does this app need this permission?</b>\n"
                           "Apps require permission to access sensitive resources on your system. "
                           "You can grant or deny these permissions at any time.");
    m_whyLabel->setWordWrap(true);
    mainLayout->addWidget(m_whyLabel);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_denyButton = new QPushButton("Deny");
    m_denyButton->setIcon(QIcon::fromTheme("dialog-cancel"));
    connect(m_denyButton, &QPushButton::clicked, this, &QDialog::reject);

    m_allowOnceButton = new QPushButton("Allow Once");
    m_allowOnceButton->setIcon(QIcon::fromTheme("document-properties"));
    connect(m_allowOnceButton, &QPushButton::clicked, this, [this]() {
        m_allowed = true;
        accept();
    });

    m_allowButton = new QPushButton("Allow");
    m_allowButton->setIcon(QIcon::fromTheme("dialog-ok"));
    m_allowButton->setStyleSheet("QPushButton { background-color: #007bff; color: white; padding: 5px; border-radius: 4px; }");
    m_allowButton->setDefault(true);
    connect(m_allowButton, &QPushButton::clicked, this, [this]() {
        m_allowed = true;
        accept();
    });

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_denyButton);
    buttonLayout->addWidget(m_allowOnceButton);
    buttonLayout->addWidget(m_allowButton);

    mainLayout->addLayout(buttonLayout);

    // Remember choice checkbox
    QCheckBox* rememberCheckbox = new QCheckBox("Remember this choice");
    mainLayout->addWidget(rememberCheckbox);
}

bool PermissionRequestDialog::isAllowed() const
{
    return m_allowed;
}

bool PermissionRequestDialog::rememberChoice() const
{
    // Find checkbox in layout
    for (int i = 0; i < layout()->count(); ++i) {
        QCheckBox* checkbox = qobject_cast<QCheckBox*>(layout()->itemAt(i)->widget());
        if (checkbox) {
            return checkbox->isChecked();
        }
    }
    return false;
}

// ============================================================================
// PrivacyStatusIndicator Implementation
// ============================================================================

PrivacyStatusIndicator::PrivacyStatusIndicator(QWidget* parent)
    : QWidget(parent), m_micActive(false), m_camActive(false), m_netActive(false)
{
    setFixedSize(48, 24);
    setStyleSheet("QWidget { background-color: transparent; }");
    setCursor(Qt::PointingHandCursor);
}

void PrivacyStatusIndicator::setMicrophoneActive(bool active)
{
    if (m_micActive != active) {
        m_micActive = active;
        update();
    }
}

void PrivacyStatusIndicator::setCameraActive(bool active)
{
    if (m_camActive != active) {
        m_camActive = active;
        update();
    }
}

void PrivacyStatusIndicator::setNetworkActive(bool active)
{
    if (m_netActive != active) {
        m_netActive = active;
        update();
    }
}

void PrivacyStatusIndicator::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int offset = 0;

    // Microphone indicator
    if (m_micActive) {
        painter.fillRect(offset, 0, 12, 24, QColor(0xFF, 0x6B, 0x6B));  // Red
        offset += 14;
    }

    // Camera indicator
    if (m_camActive) {
        painter.fillRect(offset, 0, 12, 24, QColor(0xFF, 0xA5, 0x00));  // Orange
        offset += 14;
    }

    // Network indicator
    if (m_netActive) {
        painter.fillRect(offset, 0, 12, 24, QColor(0x1E, 0x90, 0xFF));  // Blue
        offset += 14;
    }

    if (!m_micActive && !m_camActive && !m_netActive) {
        painter.fillRect(rect(), QColor(0xC0, 0xC0, 0xC0));  // Gray - all inactive
    }
}

void PrivacyStatusIndicator::mousePressEvent(QMouseEvent* event)
{
    emit clicked();
}

// ============================================================================
// PrivacyNotification Implementation
// ============================================================================

PrivacyNotification::PrivacyNotification(
    const QString& title,
    const QString& message,
    NotificationType type,
    QWidget* parent)
    : QWidget(parent), m_type(type), m_title(title), m_message(message)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::TopLevelWidget | Qt::StaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setupUI();
    setupAnimation();
}

void PrivacyNotification::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Color based on type
    QString bgColor;
    QString textColor;

    switch (m_type) {
        case Info:
            bgColor = "#D1ECF1";
            textColor = "#0C5460";
            break;
        case Warning:
            bgColor = "#FFF3CD";
            textColor = "#856404";
            break;
        case Critical:
            bgColor = "#F8D7DA";
            textColor = "#721C24";
            break;
        case Success:
            bgColor = "#D4EDDA";
            textColor = "#155724";
            break;
    }

    setStyleSheet(QString("QWidget { background-color: %1; border-radius: 8px; padding: 12px; }").arg(bgColor));

    m_titleLabel = new QLabel(QString("<b>%1</b>").arg(m_title));
    m_titleLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(textColor));
    layout->addWidget(m_titleLabel);

    m_messageLabel = new QLabel(m_message);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(textColor));
    layout->addWidget(m_messageLabel);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_actionButton = new QPushButton("View");
    m_actionButton->setStyleSheet(QString("QPushButton { background-color: %1; color: white; padding: 4px 8px; border-radius: 4px; }").arg(textColor));
    m_dismissButton = new QPushButton("Dismiss");
    m_dismissButton->setStyleSheet(QString("QPushButton { background-color: transparent; color: %1; padding: 4px 8px; border: 1px solid %1; border-radius: 4px; }").arg(textColor));

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_actionButton);
    buttonLayout->addWidget(m_dismissButton);
    layout->addLayout(buttonLayout);

    connect(m_dismissButton, &QPushButton::clicked, this, &PrivacyNotification::dismiss);
    connect(m_actionButton, &QPushButton::clicked, this, &PrivacyNotification::onActionClicked);
}

void PrivacyNotification::setupAnimation()
{
    // Notifications auto-dismiss after 5 seconds
    QTimer::singleShot(5000, this, &PrivacyNotification::onDismissTimeout);
}

void PrivacyNotification::show()
{
    QWidget::show();
}

void PrivacyNotification::dismiss()
{
    close();
    deleteLater();
}

void PrivacyNotification::onActionClicked()
{
    qDebug() << "Notification action clicked:" << m_title;
}

void PrivacyNotification::onDismissTimeout()
{
    dismiss();
}

// ============================================================================
// PrivacyTrayIcon Implementation
// ============================================================================

PrivacyTrayIcon::PrivacyTrayIcon(QObject* parent)
    : QObject(parent), m_trayIcon(nullptr), m_trayMenu(nullptr),
      m_micActive(false), m_camActive(false), m_netActive(false)
{
    setupTrayIcon();
}

PrivacyTrayIcon::~PrivacyTrayIcon()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

void PrivacyTrayIcon::setupTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon();

    m_trayMenu = new QMenu();

    QLabel* statusLabel = new QLabel("Privacy Status");
    QFont font = statusLabel->font();
    font.setBold(true);
    statusLabel->setFont(font);
    QAction* statusAction = m_trayMenu->addAction("");
    statusAction->setEnabled(false);

    m_trayMenu->addSeparator();

    m_micAction = m_trayMenu->addAction("🎤 Microphone: OFF");
    m_camAction = m_trayMenu->addAction("📷 Camera: OFF");
    m_netAction = m_trayMenu->addAction("🌐 Network: ON");

    connect(m_micAction, &QAction::triggered, this, &PrivacyTrayIcon::onMicrophoneToggled);
    connect(m_camAction, &QAction::triggered, this, &PrivacyTrayIcon::onCameraToggled);
    connect(m_netAction, &QAction::triggered, this, &PrivacyTrayIcon::onNetworkToggled);

    m_trayMenu->addSeparator();

    m_dashboardAction = m_trayMenu->addAction("Privacy Dashboard");
    connect(m_dashboardAction, &QAction::triggered, this, &PrivacyTrayIcon::onOpenDashboard);

    m_settingsAction = m_trayMenu->addAction("Settings");
    connect(m_settingsAction, &QAction::triggered, this, &PrivacyTrayIcon::onOpenSettings);

    m_trayMenu->addSeparator();

    m_quitAction = m_trayMenu->addAction("Quit");
    connect(m_quitAction, &QAction::triggered, this, &PrivacyTrayIcon::onQuit);

    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->setIcon(QIcon::fromTheme("security-high"));

    connect(m_trayIcon, QOverload<QSystemTrayIcon::ActivationReason>::of(&QSystemTrayIcon::activated),
            this, &PrivacyTrayIcon::onTrayIconActivated);
}

void PrivacyTrayIcon::show()
{
    if (m_trayIcon) {
        m_trayIcon->show();
    }
}

void PrivacyTrayIcon::setMicrophoneActive(bool active)
{
    m_micActive = active;
    if (m_micAction) {
        m_micAction->setText(active ? "🎤 Microphone: ON" : "🎤 Microphone: OFF");
    }
}

void PrivacyTrayIcon::setCameraActive(bool active)
{
    m_camActive = active;
    if (m_camAction) {
        m_camAction->setText(active ? "📷 Camera: ON" : "📷 Camera: OFF");
    }
}

void PrivacyTrayIcon::setNetworkActive(bool active)
{
    m_netActive = active;
    if (m_netAction) {
        m_netAction->setText(active ? "🌐 Network: ON" : "🌐 Network: OFF");
    }
}

void PrivacyTrayIcon::showNotification(const QString& title, const QString& message)
{
    if (m_trayIcon) {
        m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 5000);
    }
}

void PrivacyTrayIcon::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        onOpenDashboard();
    }
}

void PrivacyTrayIcon::onOpenDashboard()
{
    qDebug() << "Opening Privacy Dashboard";
}

void PrivacyTrayIcon::onOpenSettings()
{
    qDebug() << "Opening Settings";
}

void PrivacyTrayIcon::onMicrophoneToggled()
{
    m_micActive = !m_micActive;
    setMicrophoneActive(m_micActive);
    qDebug() << "Microphone toggled:" << m_micActive;
}

void PrivacyTrayIcon::onCameraToggled()
{
    m_camActive = !m_camActive;
    setCameraActive(m_camActive);
    qDebug() << "Camera toggled:" << m_camActive;
}

void PrivacyTrayIcon::onNetworkToggled()
{
    m_netActive = !m_netActive;
    setNetworkActive(m_netActive);
    qDebug() << "Network toggled:" << m_netActive;
}

void PrivacyTrayIcon::onQuit()
{
    QApplication::quit();
}

// ============================================================================
// QuickSettingsPanel Implementation
// ============================================================================

QuickSettingsPanel::QuickSettingsPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void QuickSettingsPanel::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("Quick Privacy Controls");
    QFont font = titleLabel->font();
    font.setBold(true);
    font.setPointSize(12);
    titleLabel->setFont(font);
    layout->addWidget(titleLabel);

    // Microphone toggle
    QHBoxLayout* micLayout = new QHBoxLayout();
    m_micCheckbox = new QCheckBox("Global Microphone Access");
    m_micCheckbox->setChecked(true);
    micLayout->addWidget(m_micCheckbox);
    micLayout->addStretch();
    layout->addLayout(micLayout);

    // Camera toggle
    QHBoxLayout* camLayout = new QHBoxLayout();
    m_camCheckbox = new QCheckBox("Global Camera Access");
    m_camCheckbox->setChecked(true);
    camLayout->addWidget(m_camCheckbox);
    camLayout->addStretch();
    layout->addLayout(camLayout);

    // Network toggle
    QHBoxLayout* netLayout = new QHBoxLayout();
    m_netCheckbox = new QCheckBox("Global Network Access");
    m_netCheckbox->setChecked(true);
    netLayout->addWidget(m_netCheckbox);
    netLayout->addStretch();
    layout->addLayout(netLayout);

    layout->addSpacing(10);

    // Privacy score
    QLabel* scoreLabel = new QLabel("System Privacy Score:");
    layout->addWidget(scoreLabel);

    m_privacyScoreBar = new QProgressBar();
    m_privacyScoreBar->setMaximum(100);
    m_privacyScoreBar->setValue(75);
    layout->addWidget(m_privacyScoreBar);

    // Dashboard button
    m_openDashboardButton = new QPushButton("Open Full Dashboard");
    layout->addWidget(m_openDashboardButton);

    layout->addStretch();

    // Connections
    connect(m_micCheckbox, &QCheckBox::toggled, this, &QuickSettingsPanel::onMicrophoneToggled);
    connect(m_camCheckbox, &QCheckBox::toggled, this, &QuickSettingsPanel::onCameraToggled);
    connect(m_netCheckbox, &QCheckBox::toggled, this, &QuickSettingsPanel::onNetworkToggled);
}

void QuickSettingsPanel::onMicrophoneToggled(bool checked)
{
    emit globalMicrophoneToggled(checked);
}

void QuickSettingsPanel::onCameraToggled(bool checked)
{
    emit globalCameraToggled(checked);
}

void QuickSettingsPanel::onNetworkToggled(bool checked)
{
    emit globalNetworkToggled(checked);
}

// ============================================================================
// PermissionHistoryWidget Implementation
// ============================================================================

PermissionHistoryWidget::PermissionHistoryWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void PermissionHistoryWidget::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("Permission History");
    QFont font = titleLabel->font();
    font.setBold(true);
    titleLabel->setFont(font);
    layout->addWidget(titleLabel);

    // Filters
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("Filter:"));
    m_filterCombo = new QComboBox();
    m_filterCombo->addItems({"All", "Microphone", "Camera", "File Access", "Network"});
    filterLayout->addWidget(m_filterCombo);

    m_searchBox = new QLineEdit();
    m_searchBox->setPlaceholderText("Search...");
    filterLayout->addWidget(m_searchBox);
    layout->addLayout(filterLayout);

    // History table
    m_historyTable = new QTableWidget();
    m_historyTable->setColumnCount(5);
    m_historyTable->setHorizontalHeaderLabels({"Timestamp", "Application", "Permission", "Action", "Result"});
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_historyTable);
}

// ============================================================================
// PrivacyTipsWidget Implementation
// ============================================================================

PrivacyTipsWidget::PrivacyTipsWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    loadTips();
}

void PrivacyTipsWidget::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("Privacy Tips & Warnings");
    QFont font = titleLabel->font();
    font.setBold(true);
    font.setPointSize(12);
    titleLabel->setFont(font);
    layout->addWidget(titleLabel);

    m_tipsList = new QListWidget();
    layout->addWidget(m_tipsList);

    m_descriptionLabel = new QLabel();
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 10px; border-radius: 4px; }");
    layout->addWidget(m_descriptionLabel);
}

void PrivacyTipsWidget::loadTips()
{
    m_tipsList->addItem("📌 Keep your system and applications updated");
    m_tipsList->addItem("📌 Review app permissions regularly");
    m_tipsList->addItem("📌 Use strong, unique passwords");
    m_tipsList->addItem("📌 Enable VPN for public WiFi");
    m_tipsList->addItem("📌 Disable unused hardware (camera, microphone)");
    m_tipsList->addItem("⚠️  WARNING: Multiple failed login attempts detected");
    m_tipsList->addItem("⚠️  WARNING: App requesting excessive permissions");
}
