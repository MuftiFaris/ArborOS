#pragma once

#include <QMainWindow>
#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <memory>

class QTreeView;
class QListView;
class QLineEdit;
class QPushButton;
class QSplitter;
class QLabel;
class QMenu;

class TagManager;
class PreviewHandler;

/**
 * @brief Arbor Files - Lightweight file manager
 * 
 * Features:
 * - Multi-column file browsing
 * - Tag-based organization
 * - Quick preview (Spacebar)
 * - Secure deletion & encryption
 */
class FileManager : public QMainWindow {
    Q_OBJECT

public:
    explicit FileManager(QWidget* parent = nullptr);
    ~FileManager();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    // Navigation
    void onNavigateBack();
    void onNavigateForward();
    void onHomeClicked();
    void onLocationChanged(const QString& path);
    void onDirectoryChanged();

    // File operations
    void onFileDoubleClicked(const QModelIndex& index);
    void onFileRightClicked(const QPoint& pos);
    void onDeleteFile();
    void onRenameFile();
    void onNewFolder();
    void onCutFile();
    void onCopyFile();
    void onPasteFile();

    // Tagging
    void onAddTag(const QString& tagName);
    void onRemoveTag(const QString& tagName);
    void onTagClicked(const QString& tagName);

    // Preview
    void onPreviewFile();
    void onClosePreview();

    // Search
    void onSearchTextChanged(const QString& text);
    void onSearchClear();

    // Settings
    void onToggleHiddenFiles();
    void onChangeViewMode();

    // Selection
    void onSelectionChanged();

private:
    // UI Setup
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupSidebar();
    void setupMainView();
    void setupStatusBar();
    void setupConnections();
    void restoreState();
    void saveState();

    // File operations
    void deleteFile(const QString& path);
    void renameFile(const QString& oldPath, const QString& newPath);
    void createFolder(const QString& path);
    void updateFileList();
    void loadDirectory(const QString& path);

    // Utilities
    QString getCurrentPath() const;
    QStringList getSelectedFiles() const;
    QModelIndex getCurrentIndex() const;

    // UI Components
    QLineEdit* m_locationBar;
    QPushButton* m_backButton;
    QPushButton* m_forwardButton;
    QPushButton* m_homeButton;
    QLineEdit* m_searchBox;

    QSplitter* m_mainSplitter;
    
    // Sidebar
    QListView* m_placesView;
    QListView* m_tagsView;
    
    // Main view
    QListView* m_fileListView;
    QTreeView* m_fileTreeView;
    
    // Status
    QLabel* m_statusLabel;
    QLabel* m_selectionLabel;

    // Models
    QFileSystemModel* m_fsModel;
    QStandardItemModel* m_placesModel;
    QStandardItemModel* m_tagsModel;

    // Managers
    std::unique_ptr<TagManager> m_tagManager;
    std::unique_ptr<PreviewHandler> m_previewHandler;

    // State
    QStringList m_navigationHistory;
    int m_historyIndex;
    bool m_showHiddenFiles;
    QString m_currentPath;
    QString m_clipboardPath;
    bool m_clipboardIsCut;

    // Menus
    QMenu* m_contextMenu;
};
