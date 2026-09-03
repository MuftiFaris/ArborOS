#ifndef ARBOR_LOCALIZATION_MANAGER_H
#define ARBOR_LOCALIZATION_MANAGER_H

#include <QString>
#include <QStringList>
#include <QLocale>
#include <QTranslator>
#include <QObject>
#include <QMap>
#include <memory>

namespace Arbor::Localization {

/**
 * @struct LanguageInfo
 * @brief Information about supported language
 */
struct LanguageInfo {
    QString code;           ///< Language code (e.g., "de", "fr", "ja")
    QString nativeName;     ///< Native language name (Deutsch, Français, 日本語)
    QString englishName;    ///< English name (German, French, Japanese)
    QString locale;         ///< Full locale (de_DE, fr_FR, ja_JP)
    int progressPercent;    ///< Translation progress (0-100%)
    bool isComplete;        ///< True if 100% translated
};

/**
 * @class LocalizationManager
 * @brief Manages application localization and translation
 *
 * Supported languages:
 * - English (en) - 100% (base language)
 * - German (de) - in progress
 * - French (fr) - in progress
 * - Spanish (es) - in progress
 * - Japanese (ja) - in progress
 * - Chinese Simplified (zh) - in progress
 * - Russian (ru) - in progress
 */
class LocalizationManager : public QObject {
    Q_OBJECT

public:
    static LocalizationManager* instance();

    /// Configuration
    void setApplicationLanguage(const QString &languageCode);
    void loadTranslation(const QString &languageCode);
    
    /// Query
    QString getCurrentLanguage() const;
    QStringList getSupportedLanguages() const;
    QList<LanguageInfo> getLanguageInfoList() const;
    LanguageInfo getLanguageInfo(const QString &code) const;
    
    /// Translation functions (should use tr() macro instead)
    QString translate(const QString &context, const QString &sourceText);
    QString translatePlural(const QString &context, const QString &singular, 
                           const QString &plural, int n);
    
    /// Formatting
    QString formatDate(const QDate &date) const;
    QString formatTime(const QTime &time) const;
    QString formatDateTime(const QDateTime &dateTime) const;
    QString formatNumber(int number) const;
    QString formatCurrency(double amount) const;

signals:
    void languageChanged(const QString &languageCode);
    void translationLoaded(const QString &languageCode);
    void translationMissing(const QString &context, const QString &text);

private:
    LocalizationManager();
    static std::unique_ptr<LocalizationManager> instance_;
    
    /// Translation setup
    void initializeSupportedLanguages();
    void registerTranslators();
    
    QString currentLanguage;
    QMap<QString, LanguageInfo> supportedLanguages;
    QMap<QString, QTranslator*> translators;
};

/**
 * @class TranslationProvider
 * @brief Base class for providing translations
 *
 * Subclasses override translate() for each language
 */
class TranslationProvider {
public:
    virtual ~TranslationProvider() = default;
    
    virtual QString translate(const QString &context, const QString &text) = 0;
    virtual bool isComplete() const = 0;
    virtual int progressPercent() const = 0;
};

} // namespace Arbor::Localization

/**
 * Localization macro - use throughout code for translatable strings
 * Qt's lupdate tool will extract these automatically
 */
#define QT_TRANSLATE_NOOP(context, string) (string)
#define ARBOR_TR(text) QCoreApplication::translate("Arbor", text)

#endif // ARBOR_LOCALIZATION_MANAGER_H
