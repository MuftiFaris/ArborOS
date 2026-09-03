#include "file-manager.h"
#include "tag-manager.h"
#include "preview-handler.h"
#include "theme.h"
#include "color-palette.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLineEdit>
#include <QPushButton>
#include <QListView>
#include <QTreeView>
#include <QLabel>
#include <QMenu>
#include <QStandardItemModel>
#include <QFileSystemModel>
#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QKeyEvent>
#include <QClipboard>
#include <QMimeData>
#include <QDebug>

FileManager::FileManager(QWidget* parent)
    : QMainWindow(parent)
    , m_backButton(nullptr)
    , m_forwardButton(nullptr)
    , m_homeButton(nullptr)
    , m_fileListView(nullptr)
    , m_fileTreeView(nullptr)
    , m_fsModel(nullptr)
    , m_placesModel(nullptr)
    , m_tagsModel(nullptr)
    , m_historyIndex(0)
    , m_showHiddenFiles(false)
    , m_clipboardIsCut(false)
{
    setObjectName("FileManager");

    // Initialize managers
    m_tagManager = std::make_unique<TagManager>(this);
    m_previewHandler = std::make_unique<PreviewHandler>(this);

    // Setup UI
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupSidebar();
    setupMainView();
    setupStatusBar();
    setupConnections();

    // Load saved state
    restoreState();

    // Navigate to home directory
    onHomeClicked();
}

FileManager::~FileManager() {
    saveState();
}

void FileManager::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Create main splitter
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Left sidebar will be added in setupSidebar()
    // Main view will be added in setupMainView()

    mainLayout->addWidget(m_mainSplitter);
    setCentralWidget(centralWidget);
}

void FileManager::setupMenuBar() {
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // File menu
    QMenu* fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(tr("&New Folder"), this, &FileManager::onNewFolder, QKeySequence::New);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), this, &QWidget::close, QKeySequence::Quit);

    // Edit menu
    QMenu* editMenu = menuBar->addMenu(tr("&Edit"));
    editMenu->addAction(tr("Cu&t"), this, &FileManager::onCutFile, QKeySequence::Cut);
    editMenu->addAction(tr("&Copy"), this, &FileManager::onCopyFile, QKeySequence::Copy);
    editMenu->addAction(tr("&Paste"), this, &FileManager::onPasteFile, QKeySequence::Paste);
    editMenu->addSeparator();
    editMenu->addAction(tr("&Delete"), this, &FileManager::onDeleteFile, QKeySequence::Delete);
    editMenu->addAction(tr("&Rename"), this, &FileManager::onRenameFile, Qt::CTRL + Qt::Key_R);

    // View menu
    QMenu* viewMenu = menuBar->addMenu(tr("&View"));
    viewMenu->addAction(tr("&Hidden Files"), this, &FileManager::onToggleHiddenFiles, Qt::CTRL + Qt::Key_H);

    // Help menu
    QMenu* helpMenu = menuBar->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, [this]() {
        qDebug() << "Arbor Files v1.0.0";
    });
}

void FileManager::setupToolBar() {
    QToolBar* toolBar = addToolBar(tr("Navigation"));
    toolBar->setMovable(false);

    m_backButton = toolBar->addAction(tr("◀"));
    connect(m_backButton, &QAction::triggered, this, &FileManager::onNavigateBack);

    m_forwardButton = toolBar->addAction(tr("▶"));
    connect(m_forwardButton, &QAction::triggered, this, &FileManager::onNavigateForward);

    m_homeButton = toolBar->addAction(tr("⌂"));
    connect(m_homeButton, &QAction::triggered, this, &FileManager::onHomeClicked);

    toolBar->addSeparator();

    m_locationBar = new QLineEdit(this);
    m_locationBar->setPlaceholderText(tr("Location..."));
    m_locationBar->setMaximumWidth(400);
    toolBar->addWidget(m_locationBar);
    connect(m_locationBar, &QLineEdit::returnPressed, this, [this]() {
        onLocationChanged(m_locationBar->text());
    });

    toolBar->addSeparator();

    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText(tr("Search..."));
    m_searchBox->setMaximumWidth(250);
    toolBar->addWidget(m_searchBox);
    connect(m_searchBox, &QLineEdit::textChanged, this, &FileManager::onSearchTextChanged);
}

void FileManager::setupSidebar() {
    // Places sidebar
    QWidget* sidebar = new QWidget(this);
    QVBoxLayout* sidebarLayout = new QVBoxLayout(sidebar);

    QLabel* placesLabel = new QLabel(tr("PLACES"), this);
    sidebarLayout->addWidget(placesLabel);

    m_placesModel = new QStandardItemModel(this);
    m_placesView = new QListView(this);
    m_placesView->setModel(m_placesModel);
    sidebarLayout->addWidget(m_placesView);

    // Add default places
    auto addPlace = [this](const QString& name, const QString& path) {
        QStandardItem* item = new QStandardItem(name);
        item->setData(path, Qt::UserRole);
        m_placesModel->appendRow(item);
    };

    addPlace(tr("⌂ Home"), QDir::homePath());
    addPlace(tr("📥 Downloads"), QDir::homePath() + "/Downloads");
    addPlace(tr("📄 Documents"), QDir::homePath() + "/Documents");
    addPlace(tr("🎵 Music"), QDir::homePath() + "/Music");
    addPlace(tr("🖼 Pictures"), QDir::homePath() + "/Pictures");
    addPlace(tr("🎬 Videos"), QDir::homePath() + "/Videos");

    QLabel* tagsLabel = new QLabel(tr("TAGS"), this);
    sidebarLayout->addWidget(tagsLabel);

    m_tagsModel = new QStandardItemModel(this);
    m_tagsView = new QListView(this);
    m_tagsView->setModel(m_tagsModel);
    sidebarLayout->addWidget(m_tagsView);

    // Add to splitter
    sidebar->setMaximumWidth(200);
    m_mainSplitter->addWidget(sidebar);
}

void FileManager::setupMainView() {
    m_fsModel = new QFileSystemModel(this);
    m_fsModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);

    // List view (default)
    m_fileListView = new QListView(this);
    m_fileListView->setModel(m_fsModel);
    m_fileListView->setIconSize(QSize(32, 32));
    m_fileListView->setUniformItemSizes(true);

    // Tree view (alternative)
    m_fileTreeView = new QTreeView(this);
    m_fileTreeView->setModel(m_fsModel);
    m_fileTreeView->hideColumn(1); // Hide type column by default
    m_fileTreeView->setColumnWidth(0, 300);

    // Use list view by default
    m_mainSplitter->addWidget(m_fileListView);

    connect(m_fileListView, QOverload<const QModelIndex&>::of(&QListView::doubleClicked),
            this, &FileManager::onFileDoubleClicked);
    connect(m_fileListView, &QListView::customContextMenuRequested,
            this, &FileManager::onFileRightClicked);

    m_fileListView->setContextMenuPolicy(Qt::CustomContextMenu);
}

void FileManager::setupStatusBar() {
    m_statusLabel = new QLabel(this);
    m_selectionLabel = new QLabel(this);

    statusBar()->addWidget(m_statusLabel);
    statusBar()->addPermanentWidget(m_selectionLabel);

    onSelectionChanged();
}

void FileManager::setupConnections() {
    connect(m_placesView, &QListView::clicked, this, [this](const QModelIndex& index) {
        QString path = m_placesModel->item(index.row())->data(Qt::UserRole).toString();
        loadDirectory(path);
    });

    connect(m_tagsView, &QListView::clicked, this, [this](const QModelIndex& index) {
        QString tagName = m_tagsModel->item(index.row())->text();
        onTagClicked(tagName);
    });
}

void FileManager::closeEvent(QCloseEvent* event) {
    saveState();
    QMainWindow::closeEvent(event);
}

void FileManager::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space) {
        onPreviewFile();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        onClosePreview();
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void FileManager::onNavigateBack() {
    if (m_historyIndex > 0) {
        m_historyIndex--;
        loadDirectory(m_navigationHistory[m_historyIndex]);
    }
}

void FileManager::onNavigateForward() {
    if (m_historyIndex < m_navigationHistory.size() - 1) {
        m_historyIndex++;
        loadDirectory(m_navigationHistory[m_historyIndex]);
    }
}

void FileManager::onHomeClicked() {
    loadDirectory(QDir::homePath());
}

void FileManager::onLocationChanged(const QString& path) {
    loadDirectory(path);
}

void FileManager::onDirectoryChanged() {
    updateFileList();
}

void FileManager::onFileDoubleClicked(const QModelIndex& index) {
    QString filePath = m_fsModel->filePath(index);
    QFileInfo fileInfo(filePath);

    if (fileInfo.isDir()) {
        loadDirectory(filePath);
    } else {
        // Open file with default application
        qDebug() << "Opening file:" << filePath;
    }
}

void FileManager::onFileRightClicked(const QPoint& pos) {
    if (!m_contextMenu) {
        m_contextMenu = new QMenu(this);
    }
    m_contextMenu->clear();

    m_contextMenu->addAction(tr("&Cut"), this, &FileManager::onCutFile);
    m_contextMenu->addAction(tr("&Copy"), this, &FileManager::onCopyFile);
    m_contextMenu->addAction(tr("&Paste"), this, &FileManager::onPasteFile);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(tr("&Delete"), this, &FileManager::onDeleteFile);
    m_contextMenu->addAction(tr("&Rename"), this, &FileManager::onRenameFile);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(tr("&Properties"), this, [this]() {
        qDebug() << "Show properties";
    });

    m_contextMenu->exec(m_fileListView->mapToGlobal(pos));
}

void FileManager::onDeleteFile() {
    QStringList selectedFiles = getSelectedFiles();
    for (const QString& file : selectedFiles) {
        deleteFile(file);
    }
    updateFileList();
}

void FileManager::onRenameFile() {
    qDebug() << "Rename file";
}

void FileManager::onNewFolder() {
    qDebug() << "New folder";
}

void FileManager::onCutFile() {
    m_clipboardPath = getSelectedFiles().first();
    m_clipboardIsCut = true;
}

void FileManager::onCopyFile() {
    m_clipboardPath = getSelectedFiles().first();
    m_clipboardIsCut = false;
}

void FileManager::onPasteFile() {
    if (m_clipboardPath.isEmpty()) return;

    QFileInfo fileInfo(m_clipboardPath);
    QString destPath = getCurrentPath() + "/" + fileInfo.fileName();

    if (m_clipboardIsCut) {
        QFile::rename(m_clipboardPath, destPath);
        m_clipboardPath.clear();
    } else {
        QFile::copy(m_clipboardPath, destPath);
    }

    updateFileList();
}

void FileManager::onAddTag(const QString& tagName) {
    m_tagManager->addTag(tagName);
}

void FileManager::onRemoveTag(const QString& tagName) {
    m_tagManager->removeTag(tagName);
}

void FileManager::onTagClicked(const QString& tagName) {
    qDebug() << "Tag clicked:" << tagName;
}

void FileManager::onPreviewFile() {
    QStringList selected = getSelectedFiles();
    if (!selected.isEmpty()) {
        m_previewHandler->showPreview(selected.first());
    }
}

void FileManager::onClosePreview() {
    m_previewHandler->closePreview();
}

void FileManager::onSearchTextChanged(const QString& text) {
    if (text.isEmpty()) {
        updateFileList();
    } else {
        qDebug() << "Search:" << text;
    }
}

void FileManager::onToggleHiddenFiles() {
    m_showHiddenFiles = !m_showHiddenFiles;

    if (m_showHiddenFiles) {
        m_fsModel->setFilter(QDir::AllEntries);
    } else {
        m_fsModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    }

    updateFileList();
}

void FileManager::onChangeViewMode() {
    qDebug() << "Change view mode";
}

void FileManager::onSelectionChanged() {
    QStringList selected = getSelectedFiles();
    m_selectionLabel->setText(tr("%1 items selected").arg(selected.size()));
}

void FileManager::deleteFile(const QString& path) {
    QFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
        QDir(path).removeRecursively();
    } else {
        QFile::remove(path);
    }
}

void FileManager::renameFile(const QString& oldPath, const QString& newPath) {
    QFile::rename(oldPath, newPath);
}

void FileManager::createFolder(const QString& path) {
    QDir().mkdir(path);
}

void FileManager::updateFileList() {
    // Refresh model
    m_fsModel->refresh(m_fsModel->index(m_currentPath));
}

void FileManager::loadDirectory(const QString& path) {
    QFileInfo fileInfo(path);

    if (!fileInfo.exists() || !fileInfo.isDir()) {
        return;
    }

    m_currentPath = path;
    m_locationBar->setText(path);

    // Add to navigation history
    if (m_historyIndex < m_navigationHistory.size() - 1) {
        m_navigationHistory.remove(m_historyIndex + 1, m_navigationHistory.size() - m_historyIndex - 1);
    }
    m_navigationHistory.append(path);
    m_historyIndex = m_navigationHistory.size() - 1;

    // Update file system model root
    QModelIndex rootIndex = m_fsModel->setRootPath(path);
    m_fileListView->setRootIndex(rootIndex);
    m_fileTreeView->setRootIndex(rootIndex);

    // Update status
    m_statusLabel->setText(tr("Location: %1").arg(path));

    // Update button states
    m_backButton->setEnabled(m_historyIndex > 0);
    m_forwardButton->setEnabled(m_historyIndex < m_navigationHistory.size() - 1);
}

QString FileManager::getCurrentPath() const {
    return m_currentPath;
}

QStringList FileManager::getSelectedFiles() const {
    QStringList result;

    for (const QModelIndex& index : m_fileListView->selectionModel()->selectedIndexes()) {
        result.append(m_fsModel->filePath(index));
    }

    return result;
}

QModelIndex FileManager::getCurrentIndex() const {
    return m_fileListView->currentIndex();
}

void FileManager::restoreState() {
    QSettings settings("ArborOS", "ArborFiles");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    m_currentPath = settings.value("lastPath", QDir::homePath()).toString();
}

void FileManager::saveState() {
    QSettings settings("ArborOS", "ArborFiles");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("lastPath", m_currentPath);
}
