#ifndef PRIVACY_DASHBOARD_H
#define PRIVACY_DASHBOARD_H

#include <QMainWindow>

class PrivacyDashboard : public QMainWindow
{
    Q_OBJECT

public:
    explicit PrivacyDashboard(QWidget *parent = nullptr);
    ~PrivacyDashboard();

private:
    void setupUI();
    void setupConnections();
    void updatePrivacyScore();
    void loadAuditTrail();
};

#endif // PRIVACY_DASHBOARD_H
