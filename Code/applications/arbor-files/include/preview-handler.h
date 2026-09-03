#pragma once

#include <QWidget>
#include <QString>
#include <memory>

class QLabel;
class QScrollArea;
class QVBoxLayout;

/**
 * @brief Quick preview handler for files (Spacebar activation)
 * 
 * Supports:
 * - Images (JPEG, PNG, WebP, HEIC)
 * - PDFs (with page thumbnails)
 * - Videos (frame grabber)
 * - Text/Markdown (with syntax highlighting)
 * - Audio (waveform display)
 * - Code files (syntax-highlighted)
 */
class PreviewHandler : public QWidget {
    Q_OBJECT

public:
    explicit PreviewHandler(QWidget* parent = nullptr);
    ~PreviewHandler();

    /**
     * Show preview for a file
     */
    void showPreview(const QString& filePath);

    /**
     * Close preview
     */
    void closePreview();

    /**
     * Check if preview is visible
     */
    bool isVisible() const;

    /**
     * Navigate to next file in preview (arrow keys)
     */
    void showNextFile(const QStringList& fileList, const QString& currentFile);

    /**
     * Navigate to previous file in preview
     */
    void showPreviousFile(const QStringList& fileList, const QString& currentFile);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onCloseRequested();

private:
    // Preview generators
    void previewImage(const QString& path);
    void previewPDF(const QString& path);
    void previewVideo(const QString& path);
    void previewAudio(const QString& path);
    void previewText(const QString& path);
    void previewCode(const QString& path);

    // Utilities
    QString getFileType(const QString& path) const;
    void clearPreview();
    void displayError(const QString& message);

    // UI Components
    QScrollArea* m_scrollArea;
    QLabel* m_contentLabel;
    QLabel* m_infoLabel;
    QVBoxLayout* m_layout;

    // State
    QString m_currentFile;
    bool m_visible;
};
