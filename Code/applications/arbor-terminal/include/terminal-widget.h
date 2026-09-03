#pragma once

#include <QWidget>
#include <QProcess>
#include <QString>
#include <memory>

class QPlainTextEdit;
class QLineEdit;
class QLabel;
class TerminalWidget;

/**
 * @brief Custom text edit for terminal display
 */
class TerminalTextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit TerminalTextEdit(QWidget* parent = nullptr);

    void appendOutput(const QString& text);
    void appendError(const QString& text);
    QString getInput() const;
    void clearInput();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    int m_inputStartPos;
};

/**
 * @brief Single terminal session widget
 * 
 * Manages shell process and terminal I/O
 */
class TerminalWidget : public QWidget {
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget* parent = nullptr);
    ~TerminalWidget();

    /**
     * Start terminal session with shell
     */
    void start();

    /**
     * Kill terminal process
     */
    void terminate();

    /**
     * Write input to terminal
     */
    void write(const QString& text);

    /**
     * Get terminal title/working directory
     */
    QString getTitle() const;

    /**
     * Execute command
     */
    void executeCommand(const QString& command);

    /**
     * Search in terminal history
     */
    QStringList search(const QString& pattern) const;

signals:
    void titleChanged(const QString& title);
    void processFinished(int exitCode);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onProcessReadyReadStandardOutput();
    void onProcessReadyReadStandardError();
    void onProcessFinished(int exitCode);
    void onProcessStarted();

private:
    void setupUI();
    void setupProcess();
    void updateTerminalSize();
    QString getShellCommand() const;

    // UI
    TerminalTextEdit* m_display;
    QLineEdit* m_searchBox;
    QLabel* m_statusLabel;

    // Process
    std::unique_ptr<QProcess> m_process;

    // State
    QString m_currentDirectory;
    QStringList m_history;
    int m_historyIndex;
    bool m_running;
};
