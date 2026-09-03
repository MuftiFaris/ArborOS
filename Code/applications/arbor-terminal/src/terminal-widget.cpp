#include "terminal-widget.h"
#include "theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QProcess>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

// ============ TerminalTextEdit ============

TerminalTextEdit::TerminalTextEdit(QWidget* parent)
    : QPlainTextEdit(parent)
    , m_inputStartPos(0)
{
    setFont(Arbor::Design::Theme::fontCode());
    setReadOnly(false);
}

void TerminalTextEdit::appendOutput(const QString& text) {
    appendPlainText(text);
    m_inputStartPos = document()->characterCount();
}

void TerminalTextEdit::appendError(const QString& text) {
    setTextColor(Qt::red);
    appendPlainText(text);
    setTextColor(Qt::white);
    m_inputStartPos = document()->characterCount();
}

QString TerminalTextEdit::getInput() const {
    QTextCursor cursor = textCursor();
    cursor.setPosition(m_inputStartPos);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    return cursor.selectedText();
}

void TerminalTextEdit::clearInput() {
    QTextCursor cursor = textCursor();
    cursor.setPosition(m_inputStartPos);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
}

void TerminalTextEdit::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QString input = getInput();
        if (!input.isEmpty()) {
            appendPlainText("");
            m_inputStartPos = document()->characterCount();
        }
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Backspace) {
        QTextCursor cursor = textCursor();
        if (cursor.position() <= m_inputStartPos) {
            event->ignore();
            return;
        }
    }

    QPlainTextEdit::keyPressEvent(event);
}

void TerminalTextEdit::contextMenuEvent(QContextMenuEvent* event) {
    QMenu menu(this);
    menu.addAction("Copy", [this]() {
        QApplication::clipboard()->setText(textCursor().selectedText());
    });
    menu.addAction("Paste", [this]() {
        insertPlainText(QApplication::clipboard()->text());
    });
    menu.exec(event->globalPos());
}

// ============ TerminalWidget ============

TerminalWidget::TerminalWidget(QWidget* parent)
    : QWidget(parent)
    , m_display(nullptr)
    , m_searchBox(nullptr)
    , m_statusLabel(nullptr)
    , m_historyIndex(0)
    , m_running(false)
{
    setupUI();
    setupProcess();
}

TerminalWidget::~TerminalWidget() {
    if (m_running) {
        terminate();
    }
}

void TerminalWidget::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Search bar
    QHBoxLayout* searchLayout = new QHBoxLayout();
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("Search (Ctrl+F)...");
    m_searchBox->setMaximumHeight(28);
    searchLayout->addWidget(m_searchBox);
    layout->addLayout(searchLayout);

    // Terminal display
    m_display = new TerminalTextEdit(this);
    m_display->setReadOnly(false);
    layout->addWidget(m_display);

    // Status bar
    m_statusLabel = new QLabel(this);
    m_statusLabel->setText("Terminal ready");
    layout->addWidget(m_statusLabel);

    setLayout(layout);
}

void TerminalWidget::setupProcess() {
    m_process = std::make_unique<QProcess>(this);

    connect(m_process.get(), QOverload<>::of(&QProcess::readyReadStandardOutput),
            this, &TerminalWidget::onProcessReadyReadStandardOutput);
    connect(m_process.get(), QOverload<>::of(&QProcess::readyReadStandardError),
            this, &TerminalWidget::onProcessReadyReadStandardError);
    connect(m_process.get(), QOverload<int>::of(&QProcess::finished),
            this, &TerminalWidget::onProcessFinished);
    connect(m_process.get(), &QProcess::started,
            this, &TerminalWidget::onProcessStarted);
}

void TerminalWidget::start() {
    m_currentDirectory = QDir::homePath();
    m_display->clear();

    QString shell = getShellCommand();
    m_process->setWorkingDirectory(m_currentDirectory);
    m_process->start(shell);

    if (!m_process->waitForStarted()) {
        m_display->appendError("Failed to start terminal");
    }
}

void TerminalWidget::terminate() {
    if (m_running) {
        m_process->terminate();
        m_process->waitForFinished(3000);
        m_running = false;
    }
}

void TerminalWidget::write(const QString& text) {
    if (m_running) {
        m_process->write((text + "\n").toUtf8());
    }
}

QString TerminalWidget::getTitle() const {
    return m_currentDirectory;
}

void TerminalWidget::executeCommand(const QString& command) {
    m_history.append(command);
    m_historyIndex = m_history.size();
    write(command);
}

QStringList TerminalWidget::search(const QString& pattern) const {
    QStringList results;

    for (const QString& line : m_display->toPlainText().split('\n')) {
        if (line.contains(pattern, Qt::CaseInsensitive)) {
            results.append(line);
        }
    }

    return results;
}

void TerminalWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateTerminalSize();
}

void TerminalWidget::onProcessReadyReadStandardOutput() {
    QString output = QString::fromUtf8(m_process->readAllStandardOutput());
    m_display->appendOutput(output);
}

void TerminalWidget::onProcessReadyReadStandardError() {
    QString error = QString::fromUtf8(m_process->readAllStandardError());
    m_display->appendError(error);
}

void TerminalWidget::onProcessFinished(int exitCode) {
    m_running = false;
    m_statusLabel->setText(QString("Process exited with code %1").arg(exitCode));
    emit processFinished(exitCode);
}

void TerminalWidget::onProcessStarted() {
    m_running = true;
    m_statusLabel->setText("Terminal running");
    m_display->appendOutput("$ ");
}

void TerminalWidget::updateTerminalSize() {
    if (!m_process) return;

    // Get character size from font metrics
    QFontMetrics fm(m_display->font());
    int charWidth = fm.averageCharWidth();
    int charHeight = fm.height();

    int cols = width() / charWidth;
    int rows = height() / charHeight;

    // This would require PTY support for real terminal size changes
    qDebug() << "Terminal size:" << cols << "x" << rows;
}

QString TerminalWidget::getShellCommand() const {
#ifdef Q_OS_WIN
    return "cmd.exe";
#else
    return "/bin/bash";
#endif
}
