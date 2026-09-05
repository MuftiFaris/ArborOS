#ifndef PRIVACY_DASHBOARD_WIDGET_H
#define PRIVACY_DASHBOARD_WIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QProgressBar>
#include <QList>
#include <QMap>

/**
 * PrivacyDashboardWidget - Main dashboard UI for privacy management
 * 
 * Features:
 * - Real-time activity overview
 * - Application privacy scores (0-100)
 * - Access history with filtering
 * - Privacy recommendations
 * - Permission management UI
 * - Statistics and analytics
 */

class PrivacyDashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrivacyDashboardWidget(QWidget* parent = nullptr);
    ~PrivacyDashboardWidget();

private slots:
    void onRefreshData();
    void onSearchApplications(const QString& filter);
    void onFilterHistoryByType(const QString& filterType);
    void onFilterHistoryByDate(int daysBack);
    void onApplicationSelected(const QString& appId);
    void onManagePermissions();
    void onViewDetails();
    void onExportReport();

private:
    // UI Components
    QTabWidget* m_tabWidget;

    // Overview Tab
    QWidget* createOverviewTab();
    QLabel* m_overviewLabel;
    QLabel* m_systemPrivacyScore;
    QLabel* m_recommendedActionsLabel;
    QProgressBar* m_overallPrivacyBar;
    QTableWidget* m_topViolationsTable;

    // Applications Tab
    QWidget* createApplicationsTab();
    QLineEdit* m_appSearchBox;
    QTableWidget* m_applicationsTable;
    QLabel* m_selectedAppDetails;
    QProgressBar* m_selectedAppPrivacyBar;

    // Activity Tab
    QWidget* createActivityTab();
    QComboBox* m_activityFilterType;
    QComboBox* m_activityFilterDate;
    QTableWidget* m_activityHistoryTable;

    // Recommendations Tab
    QWidget* createRecommendationsTab();
    QTableWidget* m_recommendationsTable;

    // Permissions Tab
    QWidget* createPermissionsTab();
    QTableWidget* m_permissionsTable;
    QComboBox* m_categoryFilter;

    // Helper methods
    void initializeUI();
    void setupConnections();
    void populateOverviewTab();
    void populateApplicationsTab();
    void populateActivityTab();
    void populateRecommendationsTab();
    void populatePermissionsTab();
    void updatePrivacyScores();
    void generateRecommendations();

    // Data models
    struct AppPrivacyInfo {
        QString appId;
        QString appName;
        int privacyScore;  // 0-100
        int accessCount;
        int denialCount;
        QList<QString> permissionsGranted;
        QList<QString> recentActivity;
    };

    struct PrivacyRecommendation {
        QString title;
        QString description;
        QString severity;  // "critical", "high", "medium", "low"
        QString action;
        QString appId;
    };

    QMap<QString, AppPrivacyInfo> m_appPrivacyData;
    QList<PrivacyRecommendation> m_recommendations;

    // Refresh timer
    class QTimer* m_refreshTimer;

    void calculatePrivacyScores();
};

#endif // PRIVACY_DASHBOARD_WIDGET_H
