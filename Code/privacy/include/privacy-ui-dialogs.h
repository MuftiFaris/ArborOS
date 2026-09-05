#ifndef PRIVACY_UI_DIALOGS_H
#define PRIVACY_UI_DIALOGS_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QString>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QWidget>

/**
 * PrivacyUIDialogs - User-facing privacy UI components
 * 
 * Provides:
 * - Permission request dialogs with detailed explanations
 * - Status indicators (microphone, camera, network active)
 * - Notification system for privacy events
 * - System tray integration with quick controls
 */

// Permission Request Dialog
class PermissionRequestDialog : public QDialog
{
    Q_OBJECT

public:
    enum PermissionCategory {
        Microphone = 0,
        Camera = 1,
        FileAccess = 2,
        Network = 3,
        Location = 4
    };

    explicit PermissionRequestDialog(
        const QString& appName,
        const QString& appId,
        PermissionCategory category,
        const QString& reason = "",
        QWidget* parent = nullptr
    );

    bool isAllowed() const;
    bool rememberChoice() const;

private:
    void setupUI();
    QLabel* m_appIconLabel;
    QLabel* m_descriptionLabel;
    QLabel* m_whyLabel;
    QLabel* m_riskLabel;
    QPushButton* m_allowButton;
    QPushButton* m_denyButton;
    QPushButton* m_allowOnceButton;

    bool m_allowed;
    QString m_appName;
    QString m_appId;
    PermissionCategory m_category;
    QString m_reason;
};

// Status Indicator Widget (for taskbar/panel)
class PrivacyStatusIndicator : public QWidget
{
    Q_OBJECT

public:
    explicit PrivacyStatusIndicator(QWidget* parent = nullptr);

    void setMicrophoneActive(bool active);
    void setCameraActive(bool active);
    void setNetworkActive(bool active);

signals:
    void clicked();

protected:
    void paintEvent(class QPaintEvent* event) override;
    void mousePressEvent(class QMouseEvent* event) override;

private:
    bool m_micActive;
    bool m_camActive;
    bool m_netActive;
};

// Notification System
class PrivacyNotification : public QWidget
{
    Q_OBJECT

public:
    enum NotificationType {
        Info,
        Warning,
        Critical,
        Success
    };

    explicit PrivacyNotification(
        const QString& title,
        const QString& message,
        NotificationType type = Info,
        QWidget* parent = nullptr
    );

    void show();
    void dismiss();

private slots:
    void onActionClicked();
    void onDismissTimeout();

private:
    void setupUI();
    void setupAnimation();

    NotificationType m_type;
    QString m_title;
    QString m_message;
    QLabel* m_titleLabel;
    QLabel* m_messageLabel;
    QPushButton* m_actionButton;
    QPushButton* m_dismissButton;
};

// System Tray Integration
class PrivacyTrayIcon : public QObject
{
    Q_OBJECT

public:
    explicit PrivacyTrayIcon(QObject* parent = nullptr);
    ~PrivacyTrayIcon();

    void show();
    void setMicrophoneActive(bool active);
    void setCameraActive(bool active);
    void setNetworkActive(bool active);
    void showNotification(const QString& title, const QString& message);

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onOpenDashboard();
    void onOpenSettings();
    void onMicrophoneToggled();
    void onCameraToggled();
    void onNetworkToggled();
    void onQuit();

private:
    void setupTrayIcon();
    void updateTrayMenu();

    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;
    QWidget* m_mainWindow;

    bool m_micActive;
    bool m_camActive;
    bool m_netActive;

    QAction* m_micAction;
    QAction* m_camAction;
    QAction* m_netAction;
    QAction* m_dashboardAction;
    QAction* m_settingsAction;
    QAction* m_quitAction;
};

// Quick Settings Panel
class QuickSettingsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit QuickSettingsPanel(QWidget* parent = nullptr);

signals:
    void globalMicrophoneToggled(bool enabled);
    void globalCameraToggled(bool enabled);
    void globalNetworkToggled(bool enabled);

private slots:
    void onMicrophoneToggled(bool checked);
    void onCameraToggled(bool checked);
    void onNetworkToggled(bool checked);

private:
    void setupUI();

    class QCheckBox* m_micCheckbox;
    class QCheckBox* m_camCheckbox;
    class QCheckBox* m_netCheckbox;
    QProgressBar* m_privacyScoreBar;
    QPushButton* m_openDashboardButton;
};

// Permission History Widget
class PermissionHistoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PermissionHistoryWidget(QWidget* parent = nullptr);

private:
    void setupUI();

    class QTableWidget* m_historyTable;
    class QComboBox* m_filterCombo;
    class QLineEdit* m_searchBox;
};

// Privacy Tips/Warnings Widget
class PrivacyTipsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrivacyTipsWidget(QWidget* parent = nullptr);

private:
    void setupUI();
    void loadTips();

    class QListWidget* m_tipsList;
    QLabel* m_descriptionLabel;
};

#endif // PRIVACY_UI_DIALOGS_H
