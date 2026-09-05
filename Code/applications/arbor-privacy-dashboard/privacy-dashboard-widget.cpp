#include "privacy-dashboard-widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTimer>
#include <QDateEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QDateTime>
#include <QFileDialog>

PrivacyDashboardWidget::PrivacyDashboardWidget(QWidget* parent)
    : QWidget(parent), m_refreshTimer(nullptr)
{
    initializeUI();
    setupConnections();

    // Auto-refresh every 5 seconds
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &PrivacyDashboardWidget::onRefreshData);
    m_refreshTimer->start(5000);

    // Initial data load
    onRefreshData();
}

PrivacyDashboardWidget::~PrivacyDashboardWidget()
{
    if (m_refreshTimer) {
        m_refreshTimer->stop();
    }
}

void PrivacyDashboardWidget::initializeUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Create tab widget
    m_tabWidget = new QTabWidget(this);

    // Add tabs
    m_tabWidget->addTab(createOverviewTab(), "Overview");
    m_tabWidget->addTab(createApplicationsTab(), "Applications");
    m_tabWidget->addTab(createActivityTab(), "Activity");
    m_tabWidget->addTab(createRecommendationsTab(), "Recommendations");
    m_tabWidget->addTab(createPermissionsTab(), "Permissions");

    mainLayout->addWidget(m_tabWidget);
    setLayout(mainLayout);
}

QWidget* PrivacyDashboardWidget::createOverviewTab()
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // Title
    QLabel* titleLabel = new QLabel("Privacy Overview");
    QFont font = titleLabel->font();
    font.setPointSize(16);
    font.setBold(true);
    titleLabel->setFont(font);
    layout->addWidget(titleLabel);

    // System Privacy Score
    QHBoxLayout* scoreLayout = new QHBoxLayout();
    QLabel* scoreLabel = new QLabel("System Privacy Score:");
    m_systemPrivacyScore = new QLabel("--");
    m_overallPrivacyBar = new QProgressBar();
    m_overallPrivacyBar->setMaximum(100);
    m_overallPrivacyBar->setValue(0);
    scoreLayout->addWidget(scoreLabel);
    scoreLayout->addWidget(m_systemPrivacyScore);
    scoreLayout->addWidget(m_overallPrivacyBar);
    scoreLayout->addStretch();
    layout->addLayout(scoreLayout);

    // Overview info
    m_overviewLabel = new QLabel();
    m_overviewLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 10px; border-radius: 5px; }");
    layout->addWidget(m_overviewLabel);

    // Top violations
    QLabel* violationsTitle = new QLabel("Top Privacy Violations");
    QFont violFont = violationsTitle->font();
    violFont.setBold(true);
    violationsTitle->setFont(violFont);
    layout->addWidget(violationsTitle);

    m_topViolationsTable = new QTableWidget();
    m_topViolationsTable->setColumnCount(4);
    m_topViolationsTable->setHorizontalHeaderLabels({"Application", "Access Type", "Attempts", "Severity"});
    m_topViolationsTable->horizontalHeader()->setStretchLastSection(true);
    m_topViolationsTable->setMaximumHeight(200);
    layout->addWidget(m_topViolationsTable);

    // Recommended actions
    QLabel* actionsTitle = new QLabel("Recommended Actions");
    QFont actionFont = actionsTitle->font();
    actionFont.setBold(true);
    actionsTitle->setFont(actionFont);
    layout->addWidget(actionsTitle);

    m_recommendedActionsLabel = new QLabel();
    m_recommendedActionsLabel->setStyleSheet("QLabel { background-color: #fffacd; padding: 10px; border-radius: 5px; }");
    m_recommendedActionsLabel->setWordWrap(true);
    layout->addWidget(m_recommendedActionsLabel);

    layout->addStretch();
    return tab;
}

QWidget* PrivacyDashboardWidget::createApplicationsTab()
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // Search box
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel("Search:"));
    m_appSearchBox = new QLineEdit();
    m_appSearchBox->setPlaceholderText("Type app name or ID...");
    searchLayout->addWidget(m_appSearchBox);
    searchLayout->addStretch();
    layout->addLayout(searchLayout);

    // Applications table
    m_applicationsTable = new QTableWidget();
    m_applicationsTable->setColumnCount(6);
    m_applicationsTable->setHorizontalHeaderLabels({
        "Application", "Privacy Score", "Permissions", "Access Count",
        "Denials", "Last Activity"
    });
    m_applicationsTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_applicationsTable);

    // Selected app details
    QLabel* detailsTitle = new QLabel("Application Details");
    QFont detailsFont = detailsTitle->font();
    detailsFont.setBold(true);
    detailsTitle->setFont(detailsFont);
    layout->addWidget(detailsTitle);

    m_selectedAppDetails = new QLabel("Select an application to view details");
    m_selectedAppDetails->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 10px; border-radius: 5px; }");
    m_selectedAppDetails->setWordWrap(true);
    layout->addWidget(m_selectedAppDetails);

    m_selectedAppPrivacyBar = new QProgressBar();
    m_selectedAppPrivacyBar->setMaximum(100);
    layout->addWidget(m_selectedAppPrivacyBar);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* manageBtn = new QPushButton("Manage Permissions");
    QPushButton* detailsBtn = new QPushButton("View Full History");
    buttonLayout->addWidget(manageBtn);
    buttonLayout->addWidget(detailsBtn);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    connect(manageBtn, &QPushButton::clicked, this, &PrivacyDashboardWidget::onManagePermissions);
    connect(detailsBtn, &QPushButton::clicked, this, &PrivacyDashboardWidget::onViewDetails);

    return tab;
}

QWidget* PrivacyDashboardWidget::createActivityTab()
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // Filters
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("Filter by Type:"));
    m_activityFilterType = new QComboBox();
    m_activityFilterType->addItems({"All", "Microphone", "Camera", "File Access", "Network", "Denied"});
    filterLayout->addWidget(m_activityFilterType);

    filterLayout->addWidget(new QLabel("Days Back:"));
    m_activityFilterDate = new QComboBox();
    m_activityFilterDate->addItems({"Today", "Last 7 days", "Last 30 days", "All time"});
    filterLayout->addWidget(m_activityFilterDate);

    filterLayout->addStretch();
    layout->addLayout(filterLayout);

    // Activity history table
    m_activityHistoryTable = new QTableWidget();
    m_activityHistoryTable->setColumnCount(7);
    m_activityHistoryTable->setHorizontalHeaderLabels({
        "Timestamp", "Application", "Type", "Action", "Details", "Status", "Result"
    });
    m_activityHistoryTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_activityHistoryTable);

    return tab;
}

QWidget* PrivacyDashboardWidget::createRecommendationsTab()
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    QLabel* recTitle = new QLabel("Privacy Recommendations");
    QFont recFont = recTitle->font();
    recFont.setPointSize(14);
    recFont.setBold(true);
    recTitle->setFont(recFont);
    layout->addWidget(recTitle);

    m_recommendationsTable = new QTableWidget();
    m_recommendationsTable->setColumnCount(5);
    m_recommendationsTable->setHorizontalHeaderLabels({
        "Title", "Severity", "Application", "Description", "Action"
    });
    m_recommendationsTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_recommendationsTable);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    QPushButton* applyBtn = new QPushButton("Apply Recommendation");
    QPushButton* dismissBtn = new QPushButton("Dismiss");
    actionLayout->addWidget(applyBtn);
    actionLayout->addWidget(dismissBtn);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    return tab;
}

QWidget* PrivacyDashboardWidget::createPermissionsTab()
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // Category filter
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("Category:"));
    m_categoryFilter = new QComboBox();
    m_categoryFilter->addItems({
        "All", "Microphone", "Camera", "File Access", "Network", "Location"
    });
    filterLayout->addWidget(m_categoryFilter);
    filterLayout->addStretch();
    layout->addLayout(filterLayout);

    // Permissions table
    m_permissionsTable = new QTableWidget();
    m_permissionsTable->setColumnCount(5);
    m_permissionsTable->setHorizontalHeaderLabels({
        "Application", "Category", "Permission", "Status", "Last Used"
    });
    m_permissionsTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_permissionsTable);

    // Action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* revokeBtn = new QPushButton("Revoke Permission");
    QPushButton* exportBtn = new QPushButton("Export Report");
    buttonLayout->addWidget(revokeBtn);
    buttonLayout->addWidget(exportBtn);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    connect(exportBtn, &QPushButton::clicked, this, &PrivacyDashboardWidget::onExportReport);

    return tab;
}

void PrivacyDashboardWidget::setupConnections()
{
    connect(m_appSearchBox, &QLineEdit::textChanged, this, &PrivacyDashboardWidget::onSearchApplications);
    connect(m_activityFilterType, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, [this]() { populateActivityTab(); });
    connect(m_activityFilterDate, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, [this]() { populateActivityTab(); });
    connect(m_categoryFilter, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, [this]() { populatePermissionsTab(); });
    connect(m_applicationsTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        if (m_applicationsTable->currentRow() >= 0) {
            QString appId = m_applicationsTable->item(m_applicationsTable->currentRow(), 0)->text();
            onApplicationSelected(appId);
        }
    });
}

void PrivacyDashboardWidget::onRefreshData()
{
    calculatePrivacyScores();
    populateOverviewTab();
    populateApplicationsTab();
    populateActivityTab();
    populateRecommendationsTab();
    populatePermissionsTab();
}

void PrivacyDashboardWidget::onSearchApplications(const QString& filter)
{
    for (int i = 0; i < m_applicationsTable->rowCount(); ++i) {
        QString appName = m_applicationsTable->item(i, 0)->text();
        bool matches = appName.contains(filter, Qt::CaseInsensitive);
        m_applicationsTable->setRowHidden(i, !matches);
    }
}

void PrivacyDashboardWidget::onFilterHistoryByType(const QString& filterType)
{
    populateActivityTab();
}

void PrivacyDashboardWidget::onFilterHistoryByDate(int daysBack)
{
    populateActivityTab();
}

void PrivacyDashboardWidget::onApplicationSelected(const QString& appId)
{
    if (m_appPrivacyData.contains(appId)) {
        AppPrivacyInfo info = m_appPrivacyData[appId];
        QString details = QString(
            "<b>Application:</b> %1<br>"
            "<b>Privacy Score:</b> %2/100<br>"
            "<b>Access Attempts:</b> %3<br>"
            "<b>Denied Attempts:</b> %4<br>"
            "<b>Permissions:</b> %5"
        ).arg(info.appName)
         .arg(info.privacyScore)
         .arg(info.accessCount)
         .arg(info.denialCount)
         .arg(info.permissionsGranted.join(", "));

        m_selectedAppDetails->setText(details);
        m_selectedAppPrivacyBar->setValue(info.privacyScore);
    }
}

void PrivacyDashboardWidget::onManagePermissions()
{
    qDebug() << "Open permission management dialog";
}

void PrivacyDashboardWidget::onViewDetails()
{
    qDebug() << "Open detailed history view";
}

void PrivacyDashboardWidget::onExportReport()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Export Privacy Report", "", "PDF Files (*.pdf);;CSV Files (*.csv)");
    if (!filePath.isEmpty()) {
        qDebug() << "Exporting privacy report to:" << filePath;
    }
}

void PrivacyDashboardWidget::populateOverviewTab()
{
    calculatePrivacyScores();

    // Calculate system privacy score
    int totalScore = 0;
    for (const auto& info : m_appPrivacyData) {
        totalScore += info.privacyScore;
    }
    int avgScore = m_appPrivacyData.isEmpty() ? 100 : totalScore / m_appPrivacyData.size();

    m_systemPrivacyScore->setText(QString("%1%").arg(avgScore));
    m_overallPrivacyBar->setValue(avgScore);

    // Overview summary
    QString overview = QString(
        "<b>Active Applications:</b> %1<br>"
        "<b>Permissions Granted:</b> %2<br>"
        "<b>Access Denied (24h):</b> %3<br>"
        "<b>Last Updated:</b> %4"
    ).arg(m_appPrivacyData.size())
     .arg("N/A")
     .arg("N/A")
     .arg(QDateTime::currentDateTime().toString("HH:mm:ss"));

    m_overviewLabel->setText(overview);

    // Recommended actions
    QString actions = "• Review microphone/camera access permissions<br>"
                     "• Check network usage by applications<br>"
                     "• Consider enabling VPN for sensitive apps";
    m_recommendedActionsLabel->setText(actions);
}

void PrivacyDashboardWidget::populateApplicationsTab()
{
    m_applicationsTable->setRowCount(0);

    int row = 0;
    for (const auto& appId : m_appPrivacyData.keys()) {
        const AppPrivacyInfo& info = m_appPrivacyData[appId];

        m_applicationsTable->insertRow(row);
        m_applicationsTable->setItem(row, 0, new QTableWidgetItem(info.appName));
        m_applicationsTable->setItem(row, 1, new QTableWidgetItem(QString("%1%").arg(info.privacyScore)));
        m_applicationsTable->setItem(row, 2, new QTableWidgetItem(QString::number(info.permissionsGranted.count())));
        m_applicationsTable->setItem(row, 3, new QTableWidgetItem(QString::number(info.accessCount)));
        m_applicationsTable->setItem(row, 4, new QTableWidgetItem(QString::number(info.denialCount)));
        m_applicationsTable->setItem(row, 5, new QTableWidgetItem("Recently"));

        row++;
    }
}

void PrivacyDashboardWidget::populateActivityTab()
{
    m_activityHistoryTable->setRowCount(0);

    int row = 0;
    for (const auto& appId : m_appPrivacyData.keys()) {
        const AppPrivacyInfo& info = m_appPrivacyData[appId];

        for (const QString& activity : info.recentActivity) {
            m_activityHistoryTable->insertRow(row);
            m_activityHistoryTable->setItem(row, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("HH:mm:ss")));
            m_activityHistoryTable->setItem(row, 1, new QTableWidgetItem(info.appName));
            m_activityHistoryTable->setItem(row, 2, new QTableWidgetItem("Mixed"));
            m_activityHistoryTable->setItem(row, 3, new QTableWidgetItem("Access"));
            m_activityHistoryTable->setItem(row, 4, new QTableWidgetItem(activity));
            m_activityHistoryTable->setItem(row, 5, new QTableWidgetItem("Allowed"));
            m_activityHistoryTable->setItem(row, 6, new QTableWidgetItem("✓"));

            row++;
        }
    }
}

void PrivacyDashboardWidget::populateRecommendationsTab()
{
    generateRecommendations();

    m_recommendationsTable->setRowCount(0);

    int row = 0;
    for (const auto& rec : m_recommendations) {
        m_recommendationsTable->insertRow(row);
        m_recommendationsTable->setItem(row, 0, new QTableWidgetItem(rec.title));
        m_recommendationsTable->setItem(row, 1, new QTableWidgetItem(rec.severity));
        m_recommendationsTable->setItem(row, 2, new QTableWidgetItem(rec.appId));
        m_recommendationsTable->setItem(row, 3, new QTableWidgetItem(rec.description));
        m_recommendationsTable->setItem(row, 4, new QTableWidgetItem(rec.action));

        row++;
    }
}

void PrivacyDashboardWidget::populatePermissionsTab()
{
    m_permissionsTable->setRowCount(0);

    int row = 0;
    QList<QString> categories = {"Microphone", "Camera", "File Access", "Network"};

    for (const auto& appId : m_appPrivacyData.keys()) {
        const AppPrivacyInfo& info = m_appPrivacyData[appId];

        for (const QString& perm : info.permissionsGranted) {
            m_permissionsTable->insertRow(row);
            m_permissionsTable->setItem(row, 0, new QTableWidgetItem(info.appName));
            m_permissionsTable->setItem(row, 1, new QTableWidgetItem(perm));
            m_permissionsTable->setItem(row, 2, new QTableWidgetItem("Granted"));
            m_permissionsTable->setItem(row, 3, new QTableWidgetItem("Active"));
            m_permissionsTable->setItem(row, 4, new QTableWidgetItem("Just now"));

            row++;
        }
    }
}

void PrivacyDashboardWidget::calculatePrivacyScores()
{
    // Simulate privacy score calculation based on access patterns
    m_appPrivacyData.clear();

    // Sample app data (in real implementation, would query PrivacyManager)
    AppPrivacyInfo browser;
    browser.appId = "firefox";
    browser.appName = "Firefox";
    browser.privacyScore = 65;
    browser.accessCount = 24;
    browser.denialCount = 3;
    browser.permissionsGranted = {"Network", "File Access"};
    browser.recentActivity = {"DNS query to google.com", "File read from Downloads"};
    m_appPrivacyData["firefox"] = browser;

    AppPrivacyInfo camera;
    camera.appId = "cheese";
    camera.appName = "Cheese";
    camera.privacyScore = 80;
    camera.accessCount = 5;
    camera.denialCount = 1;
    camera.permissionsGranted = {"Camera"};
    camera.recentActivity = {"Camera access requested"};
    m_appPrivacyData["cheese"] = camera;

    AppPrivacyInfo mic;
    mic.appId = "audacity";
    mic.appName = "Audacity";
    mic.privacyScore = 75;
    mic.accessCount = 8;
    mic.denialCount = 0;
    mic.permissionsGranted = {"Microphone", "File Access"};
    mic.recentActivity = {"Microphone recording", "File save"};
    m_appPrivacyData["audacity"] = mic;
}

void PrivacyDashboardWidget::generateRecommendations()
{
    m_recommendations.clear();

    m_recommendations.append({
        "Disable unnecessary microphone access",
        "high",
        "audacity",
        "Audacity has microphone access but is not actively using it. Consider revoking this permission to improve privacy.",
        "Revoke Permission"
    });

    m_recommendations.append({
        "Monitor network activity",
        "medium",
        "firefox",
        "Firefox has made 24 network access attempts in the last 24 hours. Review DNS queries for suspicious domains.",
        "View DNS Log"
    });

    m_recommendations.append({
        "Enable VPN for sensitive apps",
        "medium",
        "*",
        "Consider enabling VPN for applications that access sensitive data or require privacy protection.",
        "Configure VPN"
    });
}
