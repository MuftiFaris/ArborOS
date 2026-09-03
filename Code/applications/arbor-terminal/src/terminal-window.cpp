#include "terminal-window.h"
#include "terminal-widget.h"
#include "theme.h"
#include <QTabWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QSettings>
#include <QKeyEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>

TerminalWindow::TerminalWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_tabWidget(nullptr)
    , m_currentSplitter(nullptr)
    , m_searchBox(nullptr)
    , m_fontSize(13)
    , m_fullscreen(false)
{
    setObjectName("TerminalWindow");

    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupConnections();

    restoreState();

    setAcceptDrops(true);
}

TerminalWindow::~TerminalWindow() {
    saveState();
    closeAllTerminals();
}

void TerminalWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    mainLayout->addWidget(m_tabWidget);

    // Create default terminal
    TerminalWidget* terminal = createTerminal();
    m_tabWidget->addTab(terminal, "Terminal 1");

    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void TerminalWindow::setupMenuBar() {
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // File menu
    QMenu* fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(tr("&New Tab"), this, &TerminalWindow::onNewTab, QKeySequence::New);
    fileMenu->addAction(tr("&Close Tab"), this, [this]() {
        onCloseTab(m_tabWidget->currentIndex());
    }, QKeySequence::Close);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), this, &QWidget::close, QKeySequence::Quit);

    // Edit menu
    QMenu* editMenu = menuBar->addMenu(tr("&Edit"));
    editMenu->addAction(tr("&Find"), this, [this]() {
        m_searchBox->setFocus();
        m_searchBox->selectAll();
    }, QKeySequence::Find);

    // View menu
    QMenu* viewMenu = menuBar->addMenu(tr("&View"));
    viewMenu->addAction(tr("&Split Vertical"), this, &TerminalWindow::onSplitVertical, Qt::CTRL + Qt::Key_Bar);
    viewMenu->addAction(tr("&Split Horizontal"), this, &TerminalWindow::onSplitHorizontal, Qt::CTRL + Qt::Key_Minus);
    viewMenu->addSeparator();
    viewMenu->addAction(tr("&Fullscreen"), this, &TerminalWindow::onToggleFullscreen, QKeySequence::FullScreen);

    // Help menu
    QMenu* helpMenu = menuBar->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, [this]() {
        qDebug() << "Arbor Terminal v1.0.0";
    });
}

void TerminalWindow::setupToolBar() {
    QToolBar* toolBar = addToolBar(tr("Terminal"));
    toolBar->setMovable(false);

    toolBar->addAction(tr("+"), this, &TerminalWindow::onNewTab);
    toolBar->addAction(tr("⋮"), this, [this]() {
        QMenu menu;
        menu.addAction(tr("Split Vertical"), this, &TerminalWindow::onSplitVertical);
        menu.addAction(tr("Split Horizontal"), this, &TerminalWindow::onSplitHorizontal);
        menu.exec(QCursor::pos());
    });

    toolBar->addSeparator();

    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText(tr("Search..."));
    m_searchBox->setMaximumWidth(250);
    toolBar->addWidget(m_searchBox);

    m_searchPrevButton = toolBar->addAction(tr("⬆"));
    m_searchNextButton = toolBar->addAction(tr("⬇"));

    connect(m_searchBox, &QLineEdit::textChanged, this, &TerminalWindow::onSearchTextChanged);
    connect(m_searchPrevButton, &QAction::triggered, this, &TerminalWindow::onSearchPrevious);
    connect(m_searchNextButton, &QAction::triggered, this, &TerminalWindow::onSearchNext);
}

void TerminalWindow::setupStatusBar() {
    m_statusLabel = new QLabel(this);
    m_statusLabel->setText("Ready");
    statusBar()->addWidget(m_statusLabel);
}

void TerminalWindow::setupConnections() {
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &TerminalWindow::onTabChanged);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &TerminalWindow::onCloseTab);
}

void TerminalWindow::closeEvent(QCloseEvent* event) {
    saveState();
    closeAllTerminals();
    QMainWindow::closeEvent(event);
}

void TerminalWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Tab && event->modifiers() & Qt::ControlModifier) {
        int nextIndex = (m_tabWidget->currentIndex() + 1) % m_tabWidget->count();
        m_tabWidget->setCurrentIndex(nextIndex);
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Tab && event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) {
        int prevIndex = (m_tabWidget->currentIndex() - 1 + m_tabWidget->count()) % m_tabWidget->count();
        m_tabWidget->setCurrentIndex(prevIndex);
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void TerminalWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls() || event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}

void TerminalWindow::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasUrls()) {
        for (const QUrl& url : event->mimeData()->urls()) {
            if (TerminalWidget* terminal = getCurrentTerminal()) {
                terminal->executeCommand("cd " + url.path());
            }
        }
    }
}

void TerminalWindow::onNewTab() {
    TerminalWidget* terminal = createTerminal();
    int index = m_tabWidget->addTab(terminal, QString("Terminal %1").arg(m_tabWidget->count() + 1));
    m_tabWidget->setCurrentIndex(index);
}

void TerminalWindow::onCloseTab(int index) {
    if (QWidget* widget = m_tabWidget->widget(index)) {
        if (TerminalWidget* terminal = qobject_cast<TerminalWidget*>(widget)) {
            terminal->terminate();
        }
        m_tabWidget->removeTab(index);
        widget->deleteLater();

        if (m_tabWidget->count() == 0) {
            onNewTab();
        }
    }
}

void TerminalWindow::onTabChanged(int index) {
    if (index >= 0) {
        m_statusLabel->setText(QString("Tab %1 active").arg(index + 1));
    }
}

void TerminalWindow::onTerminalTitleChanged(const QString& title) {
    if (TerminalWidget* terminal = qobject_cast<TerminalWidget*>(sender())) {
        int index = m_tabWidget->indexOf(terminal);
        if (index >= 0) {
            m_tabWidget->setTabText(index, title);
        }
    }
}

void TerminalWindow::onSplitVertical() {
    qDebug() << "Vertical split (future implementation)";
}

void TerminalWindow::onSplitHorizontal() {
    qDebug() << "Horizontal split (future implementation)";
}

void TerminalWindow::onCloseSplit() {
    qDebug() << "Close split";
}

void TerminalWindow::onSearchTextChanged(const QString& text) {
    if (TerminalWidget* terminal = getCurrentTerminal()) {
        QStringList results = terminal->search(text);
        m_statusLabel->setText(QString("Found %1 matches").arg(results.size()));
    }
}

void TerminalWindow::onSearchPrevious() {
    qDebug() << "Search previous";
}

void TerminalWindow::onSearchNext() {
    qDebug() << "Search next";
}

void TerminalWindow::onClearSearch() {
    m_searchBox->clear();
}

void TerminalWindow::onShowPreferences() {
    qDebug() << "Show preferences";
}

void TerminalWindow::onToggleFullscreen() {
    m_fullscreen = !m_fullscreen;

    if (m_fullscreen) {
        showFullScreen();
    } else {
        showNormal();
    }
}

void TerminalWindow::onFontSizeChanged(int delta) {
    m_fontSize += delta;
    m_fontSize = qBound(8, m_fontSize, 24);

    for (int i = 0; i < m_tabWidget->count(); ++i) {
        if (TerminalWidget* terminal = qobject_cast<TerminalWidget*>(m_tabWidget->widget(i))) {
            QFont font = Arbor::Design::Theme::fontCode();
            font.setPixelSize(m_fontSize);
            // Apply to terminal display
        }
    }
}

void TerminalWindow::onThemeChanged() {
    qDebug() << "Theme changed";
}

TerminalWidget* TerminalWindow::createTerminal() {
    TerminalWidget* terminal = new TerminalWidget(this);
    connect(terminal, &TerminalWidget::titleChanged, this, &TerminalWindow::onTerminalTitleChanged);
    connect(terminal, &TerminalWidget::processFinished, this, [this](int exitCode) {
        m_statusLabel->setText(QString("Process exited with code %1").arg(exitCode));
    });
    terminal->start();
    return terminal;
}

void TerminalWindow::closeAllTerminals() {
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        if (TerminalWidget* terminal = qobject_cast<TerminalWidget*>(m_tabWidget->widget(i))) {
            terminal->terminate();
        }
    }
}

int TerminalWindow::getCurrentTabIndex() const {
    return m_tabWidget->currentIndex();
}

TerminalWidget* TerminalWindow::getCurrentTerminal() const {
    return qobject_cast<TerminalWidget*>(m_tabWidget->currentWidget());
}

void TerminalWindow::restoreState() {
    QSettings settings("ArborOS", "ArborTerminal");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    m_fontSize = settings.value("fontSize", 13).toInt();
}

void TerminalWindow::saveState() {
    QSettings settings("ArborOS", "ArborTerminal");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("fontSize", m_fontSize);
}
