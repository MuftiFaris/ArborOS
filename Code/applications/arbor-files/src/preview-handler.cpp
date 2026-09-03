#include "preview-handler.h"
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPixmap>
#include <QMimeType>
#include <QMimeDatabase>
#include <QFileInfo>
#include <QFile>
#include <QDebug>
#include <QKeyEvent>
#include <QMouseEvent>

PreviewHandler::PreviewHandler(QWidget* parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentLabel(nullptr)
    , m_infoLabel(nullptr)
    , m_layout(nullptr)
    , m_visible(false)
{
    // Setup UI
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(16, 16, 16, 16);

    m_infoLabel = new QLabel(this);
    m_layout->addWidget(m_infoLabel);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);

    m_contentLabel = new QLabel(this);
    m_contentLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_scrollArea->setWidget(m_contentLabel);

    m_layout->addWidget(m_scrollArea);

    setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setStyleSheet("background-color: rgba(0, 0, 0, 0.9); color: white;");
    hide();
}

PreviewHandler::~PreviewHandler() {
}

void PreviewHandler::showPreview(const QString& filePath) {
    m_currentFile = filePath;

    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists()) {
        displayError("File not found: " + filePath);
        return;
    }

    QString fileType = getFileType(filePath);

    if (fileType.startsWith("image/")) {
        previewImage(filePath);
    } else if (fileType == "application/pdf") {
        previewPDF(filePath);
    } else if (fileType.startsWith("video/")) {
        previewVideo(filePath);
    } else if (fileType.startsWith("audio/")) {
        previewAudio(filePath);
    } else if (fileType.startsWith("text/")) {
        previewText(filePath);
    } else if (fileType.contains("text") || fileType.contains("code")) {
        previewCode(filePath);
    } else {
        displayError("Preview not available for this file type");
    }

    // Show preview window
    if (parentWidget()) {
        setGeometry(parentWidget()->geometry());
    }

    m_visible = true;
    show();
    raise();
    setFocus();
}

void PreviewHandler::closePreview() {
    clearPreview();
    m_visible = false;
    hide();
}

bool PreviewHandler::isVisible() const {
    return m_visible;
}

void PreviewHandler::showNextFile(const QStringList& fileList, const QString& currentFile) {
    int index = fileList.indexOf(currentFile);

    if (index != -1 && index < fileList.size() - 1) {
        showPreview(fileList[index + 1]);
    }
}

void PreviewHandler::showPreviousFile(const QStringList& fileList, const QString& currentFile) {
    int index = fileList.indexOf(currentFile);

    if (index > 0) {
        showPreview(fileList[index - 1]);
    }
}

void PreviewHandler::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        closePreview();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

void PreviewHandler::mousePressEvent(QMouseEvent* event) {
    closePreview();
    event->accept();
}

void PreviewHandler::previewImage(const QString& path) {
    QPixmap pixmap(path);

    if (pixmap.isNull()) {
        displayError("Failed to load image");
        return;
    }

    // Scale to fit window
    QSize maxSize = size() * 0.9;
    pixmap = pixmap.scaledToWidth(maxSize.width(), Qt::SmoothTransformation);

    m_contentLabel->setPixmap(pixmap);

    QFileInfo fileInfo(path);
    m_infoLabel->setText(QString("Image: %1 (%2x%3)")
                             .arg(fileInfo.fileName())
                             .arg(pixmap.width())
                             .arg(pixmap.height()));
}

void PreviewHandler::previewPDF(const QString& path) {
    QFileInfo fileInfo(path);
    m_contentLabel->setText("PDF Preview\n" + path);
    m_infoLabel->setText(QString("PDF: %1").arg(fileInfo.fileName()));
}

void PreviewHandler::previewVideo(const QString& path) {
    QFileInfo fileInfo(path);
    m_contentLabel->setText("Video Preview\n" + path);
    m_infoLabel->setText(QString("Video: %1").arg(fileInfo.fileName()));
}

void PreviewHandler::previewAudio(const QString& path) {
    QFileInfo fileInfo(path);
    m_contentLabel->setText("Audio Preview\n" + path);
    m_infoLabel->setText(QString("Audio: %1").arg(fileInfo.fileName()));
}

void PreviewHandler::previewText(const QString& path) {
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        displayError("Failed to open file");
        return;
    }

    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Limit preview size
    if (content.size() > 50000) {
        content = content.left(50000) + "\n... (truncated)";
    }

    m_contentLabel->setText(content);

    QFileInfo fileInfo(path);
    m_infoLabel->setText(QString("Text: %1 (%2 bytes)")
                             .arg(fileInfo.fileName())
                             .arg(fileInfo.size()));
}

void PreviewHandler::previewCode(const QString& path) {
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        displayError("Failed to open file");
        return;
    }

    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Limit preview size
    if (content.size() > 50000) {
        content = content.left(50000) + "\n... (truncated)";
    }

    m_contentLabel->setText(content);

    QFileInfo fileInfo(path);
    m_infoLabel->setText(QString("Code: %1 (%2 bytes)")
                             .arg(fileInfo.fileName())
                             .arg(fileInfo.size()));
}

QString PreviewHandler::getFileType(const QString& path) const {
    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(path);
    return mimeType.name();
}

void PreviewHandler::clearPreview() {
    m_contentLabel->clear();
    m_infoLabel->clear();
    m_currentFile.clear();
}

void PreviewHandler::displayError(const QString& message) {
    m_contentLabel->setText(message);
    m_infoLabel->setText("Error");
}
