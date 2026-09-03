#include "tag-manager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

TagManager::TagManager(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
{
    m_db = std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase("QSQLITE"));
    initializeDatabase();
}

TagManager::~TagManager() {
    if (m_db && m_db->isOpen()) {
        m_db->close();
    }
}

bool TagManager::initializeDatabase() {
    if (!ensureDatabaseOpen()) {
        return false;
    }

    if (!createTables()) {
        return false;
    }

    m_initialized = true;
    return true;
}

bool TagManager::createTables() {
    if (!m_db || !m_db->isOpen()) {
        return false;
    }

    QSqlQuery query(*m_db);

    // Tags table
    if (!query.exec("CREATE TABLE IF NOT EXISTS tags ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "name TEXT UNIQUE NOT NULL,"
                    "color TEXT DEFAULT '#2DD4BF',"
                    "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                    ")")) {
        qWarning() << "Failed to create tags table:" << query.lastError().text();
        return false;
    }

    // File tags table
    if (!query.exec("CREATE TABLE IF NOT EXISTS file_tags ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "file_path TEXT NOT NULL,"
                    "tag_id INTEGER NOT NULL,"
                    "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
                    "FOREIGN KEY(tag_id) REFERENCES tags(id),"
                    "UNIQUE(file_path, tag_id)"
                    ")")) {
        qWarning() << "Failed to create file_tags table:" << query.lastError().text();
        return false;
    }

    return true;
}

bool TagManager::ensureDatabaseOpen() {
    if (m_db && m_db->isOpen()) {
        return true;
    }

    QString dbPath = getDatabasePath();
    QDir dbDir(QFileInfo(dbPath).path());

    if (!dbDir.exists()) {
        dbDir.mkpath(".");
    }

    m_db->setDatabaseName(dbPath);

    if (!m_db->open()) {
        qWarning() << "Failed to open database:" << m_db->lastError().text();
        return false;
    }

    return true;
}

QString TagManager::getDatabasePath() const {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationLocalDataLocation);
    return dataDir + "/tags.db";
}

bool TagManager::addTag(const QString& tagName, const QString& color) {
    if (!ensureDatabaseOpen()) {
        return false;
    }

    QSqlQuery query(*m_db);
    query.prepare("INSERT INTO tags (name, color) VALUES (?, ?)");
    query.addBindValue(tagName);
    query.addBindValue(color);

    if (!query.exec()) {
        qWarning() << "Failed to add tag:" << query.lastError().text();
        return false;
    }

    emit tagAdded(tagName);
    return true;
}

bool TagManager::removeTag(const QString& tagName) {
    if (!ensureDatabaseOpen()) {
        return false;
    }

    QSqlQuery query(*m_db);

    // First, get the tag ID
    query.prepare("SELECT id FROM tags WHERE name = ?");
    query.addBindValue(tagName);

    if (!query.exec() || !query.next()) {
        qWarning() << "Tag not found:" << tagName;
        return false;
    }

    int tagId = query.value(0).toInt();

    // Delete all file associations
    query.prepare("DELETE FROM file_tags WHERE tag_id = ?");
    query.addBindValue(tagId);
    query.exec();

    // Delete the tag
    query.prepare("DELETE FROM tags WHERE id = ?");
    query.addBindValue(tagId);

    if (!query.exec()) {
        qWarning() << "Failed to remove tag:" << query.lastError().text();
        return false;
    }

    emit tagRemoved(tagName);
    return true;
}

bool TagManager::renameTag(const QString& oldName, const QString& newName) {
    if (!ensureDatabaseOpen()) {
        return false;
    }

    QSqlQuery query(*m_db);
    query.prepare("UPDATE tags SET name = ? WHERE name = ?");
    query.addBindValue(newName);
    query.addBindValue(oldName);

    if (!query.exec()) {
        qWarning() << "Failed to rename tag:" << query.lastError().text();
        return false;
    }

    return true;
}

bool TagManager::tagFile(const QString& filePath, const QString& tagName) {
    if (!ensureDatabaseOpen()) {
        return false;
    }

    QSqlQuery query(*m_db);

    // Get tag ID
    query.prepare("SELECT id FROM tags WHERE name = ?");
    query.addBindValue(tagName);

    if (!query.exec() || !query.next()) {
        qWarning() << "Tag not found:" << tagName;
        return false;
    }

    int tagId = query.value(0).toInt();

    // Add file-tag association
    query.prepare("INSERT OR IGNORE INTO file_tags (file_path, tag_id) VALUES (?, ?)");
    query.addBindValue(filePath);
    query.addBindValue(tagId);

    if (!query.exec()) {
        qWarning() << "Failed to tag file:" << query.lastError().text();
        return false;
    }

    emit fileTagged(filePath, tagName);
    return true;
}

bool TagManager::untagFile(const QString& filePath, const QString& tagName) {
    if (!ensureDatabaseOpen()) {
        return false;
    }

    QSqlQuery query(*m_db);

    // Get tag ID
    query.prepare("SELECT id FROM tags WHERE name = ?");
    query.addBindValue(tagName);

    if (!query.exec() || !query.next()) {
        return false;
    }

    int tagId = query.value(0).toInt();

    // Remove file-tag association
    query.prepare("DELETE FROM file_tags WHERE file_path = ? AND tag_id = ?");
    query.addBindValue(filePath);
    query.addBindValue(tagId);

    if (!query.exec()) {
        qWarning() << "Failed to untag file:" << query.lastError().text();
        return false;
    }

    emit fileUntagged(filePath, tagName);
    return true;
}

bool TagManager::clearFileTags(const QString& filePath) {
    if (!ensureDatabaseOpen()) {
        return false;
    }

    QSqlQuery query(*m_db);
    query.prepare("DELETE FROM file_tags WHERE file_path = ?");
    query.addBindValue(filePath);

    return query.exec();
}

QStringList TagManager::getTags() const {
    QStringList result;

    if (!ensureDatabaseOpen()) {
        return result;
    }

    QSqlQuery query(*m_db);

    if (!query.exec("SELECT name FROM tags ORDER BY name")) {
        qWarning() << "Failed to get tags:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        result.append(query.value(0).toString());
    }

    return result;
}

QStringList TagManager::getTagsForFile(const QString& filePath) const {
    QStringList result;

    if (!ensureDatabaseOpen()) {
        return result;
    }

    QSqlQuery query(*m_db);
    query.prepare("SELECT t.name FROM tags t "
                  "JOIN file_tags ft ON t.id = ft.tag_id "
                  "WHERE ft.file_path = ? "
                  "ORDER BY t.name");
    query.addBindValue(filePath);

    if (!query.exec()) {
        qWarning() << "Failed to get file tags:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        result.append(query.value(0).toString());
    }

    return result;
}

QStringList TagManager::getFilesWithTag(const QString& tagName) const {
    QStringList result;

    if (!ensureDatabaseOpen()) {
        return result;
    }

    QSqlQuery query(*m_db);
    query.prepare("SELECT DISTINCT ft.file_path FROM file_tags ft "
                  "JOIN tags t ON ft.tag_id = t.id "
                  "WHERE t.name = ?");
    query.addBindValue(tagName);

    if (!query.exec()) {
        qWarning() << "Failed to get files with tag:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        result.append(query.value(0).toString());
    }

    return result;
}

QString TagManager::getTagColor(const QString& tagName) const {
    if (!ensureDatabaseOpen()) {
        return "#2DD4BF";
    }

    QSqlQuery query(*m_db);
    query.prepare("SELECT color FROM tags WHERE name = ?");
    query.addBindValue(tagName);

    if (!query.exec() || !query.next()) {
        return "#2DD4BF";
    }

    return query.value(0).toString();
}

int TagManager::getTagCount(const QString& tagName) const {
    if (!ensureDatabaseOpen()) {
        return 0;
    }

    QSqlQuery query(*m_db);
    query.prepare("SELECT COUNT(*) FROM file_tags ft "
                  "JOIN tags t ON ft.tag_id = t.id "
                  "WHERE t.name = ?");
    query.addBindValue(tagName);

    if (!query.exec() || !query.next()) {
        return 0;
    }

    return query.value(0).toInt();
}

QStringList TagManager::searchFilesByTag(const QString& tagName) const {
    return getFilesWithTag(tagName);
}

QStringList TagManager::searchFilesByMultipleTags(const QStringList& tags, bool requireAll) const {
    if (tags.isEmpty() || !ensureDatabaseOpen()) {
        return QStringList();
    }

    QSqlQuery query(*m_db);

    if (requireAll) {
        // Files with ALL tags
        QString placeholders = QString("?").repeated(tags.size()).replace("", ", ").trimmed();

        query.prepare(QString(
            "SELECT file_path FROM file_tags ft "
            "JOIN tags t ON ft.tag_id = t.id "
            "WHERE t.name IN (%1) "
            "GROUP BY ft.file_path "
            "HAVING COUNT(DISTINCT t.id) = %2"
        ).arg(placeholders, QString::number(tags.size())));

        for (const QString& tag : tags) {
            query.addBindValue(tag);
        }
    } else {
        // Files with ANY tag
        QString placeholders = QString("?").repeated(tags.size()).replace("", ", ").trimmed();

        query.prepare(QString(
            "SELECT DISTINCT file_path FROM file_tags ft "
            "JOIN tags t ON ft.tag_id = t.id "
            "WHERE t.name IN (%1)"
        ).arg(placeholders));

        for (const QString& tag : tags) {
            query.addBindValue(tag);
        }
    }

    QStringList result;

    if (!query.exec()) {
        qWarning() << "Failed to search by tags:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        result.append(query.value(0).toString());
    }

    return result;
}
