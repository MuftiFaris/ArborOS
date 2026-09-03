#pragma once

#include <QMainWindow>
#include <QString>
#include <memory>

class QListWidget;
class QStackedWidget;
class SettingsModel;

/**
 * @brief Arbor Settings - Unified control center
 * 
 * Two-pane layout:
 * - Left: Category list (Network, Display, Sound, etc.)
 * - Right: Dynamic content for selected category
 */
class SettingsWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget* parent = nullptr);
    ~SettingsWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    // Navigation
    void onCategorySelected(int index);
    void onSearchTextChanged(const QString& text);
    void onSearchClear();

    // Settings changes
    void onSettingChanged(const QString& key, const QVariant& value);

    // Actions
    void onApplySettings();
    void onResetSettings();

private:
    // UI Setup
    void setupUI();
    void setupSearchBar();
    void setupSidebar();
    void setupContentArea();
    void setupMenuBar();
    void setupStatusBar();
    void setupConnections();
    void restoreState();
    void saveState();

    // Panel creation
    void createSystemPanel();
    void createSecurityPanel();
    void createPersonalizationPanel();
    void createApplicationsPanel();
    void createAboutPanel();

    // Utilities
    void filterCategories(const QString& filter);
    void showCategory(const QString& categoryName);

    // UI Components
    QLineEdit* m_searchBox;
    QListWidget* m_categoryList;
    QStackedWidget* m_contentStack;
    QLabel* m_statusLabel;

    // Models
    std::unique_ptr<SettingsModel> m_settingsModel;

    // State
    int m_currentPanelIndex;
};
