#pragma once

#include <QMainWindow>
#include <QString>
#include <memory>

class QTabWidget;
class QSplitter;
class TerminalWidget;

/**
 * @brief Main terminal window with tabs and splits
 * 
 * Features:
 * - Multi-tab terminal sessions
 * - Split panes (horizontal/vertical)
 * - Search across terminal history
 * - Keyboard navigation
 */
class TerminalWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit TerminalWindow(QWidget* parent = nullptr);
    ~TerminalWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    // Tab management
    void onNewTab();
    void onCloseTab(int index);
    void onTabChanged(int index);
    void onTerminalTitleChanged(const QString& title);

    // Split operations
    void onSplitVertical();
    void onSplitHorizontal();
    void onCloseSplit();

    // Search
    void onSearchTextChanged(const QString& text);
    void onSearchPrevious();
    void onSearchNext();
    void onClearSearch();

    // Preferences
    void onShowPreferences();
    void onToggleFullscreen();

    // Settings
    void onFontSizeChanged(int delta);
    void onThemeChanged();

private:
    // UI Setup
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();
    void restoreState();
    void saveState();

    // Terminal management
    TerminalWidget* createTerminal();
    void closeAllTerminals();

    // Utilities
    int getCurrentTabIndex() const;
    TerminalWidget* getCurrentTerminal() const;

    // UI Components
    QTabWidget* m_tabWidget;
    QSplitter* m_currentSplitter;
    QLineEdit* m_searchBox;
    QPushButton* m_searchPrevButton;
    QPushButton* m_searchNextButton;
    QLabel* m_statusLabel;

    // State
    int m_fontSize;
    bool m_fullscreen;
};
