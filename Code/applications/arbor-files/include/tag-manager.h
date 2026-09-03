#pragma once

#include <QString>
#include <QStringList>
#include <QMap>
#include <QObject>
#include <memory>

class QSqlDatabase;

/**
 * @brief Tag management system for non-destructive file organization
 * 
 * Stores file tags in SQLite database (~/.local/share/arbor/tags.db)
 * Supports multiple tags per file, colored tags, and tag searching.
 */
class TagManager : public QObject {
    Q_OBJECT

public:
    explicit TagManager(QObject* parent = nullptr);
    ~TagManager();

    // Tag CRUD
    bool addTag(const QString& tagName, const QString& color = "#2DD4BF");
    bool removeTag(const QString& tagName);
    bool renameTag(const QString& oldName, const QString& newName);

    // File tagging
    bool tagFile(const QString& filePath, const QString& tagName);
    bool untagFile(const QString& filePath, const QString& tagName);
    bool clearFileTags(const QString& filePath);

    // Queries
    QStringList getTags() const;
    QStringList getTagsForFile(const QString& filePath) const;
    QStringList getFilesWithTag(const QString& tagName) const;
    QString getTagColor(const QString& tagName) const;
    int getTagCount(const QString& tagName) const;

    // Search
    QStringList searchFilesByTag(const QString& tagName) const;
    QStringList searchFilesByMultipleTags(const QStringList& tags, bool requireAll = false) const;

signals:
    void tagAdded(const QString& tagName);
    void tagRemoved(const QString& tagName);
    void fileTagged(const QString& filePath, const QString& tagName);
    void fileUntagged(const QString& filePath, const QString& tagName);

private:
    // Database setup
    bool initializeDatabase();
    bool createTables();

    // Utilities
    QString getDatabasePath() const;
    bool ensureDatabaseOpen();

    std::unique_ptr<QSqlDatabase> m_db;
    bool m_initialized;
};
