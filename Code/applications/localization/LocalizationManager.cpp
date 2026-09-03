#include "LocalizationManager.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>

namespace Arbor::Localization {

std::unique_ptr<LocalizationManager> LocalizationManager::instance_;

LocalizationManager* LocalizationManager::instance()
{
    if (!instance_) {
        instance_ = std::make_unique<LocalizationManager>();
    }
    return instance_.get();
}

LocalizationManager::LocalizationManager()
    : currentLanguage("en")
{
    initializeSupportedLanguages();
    registerTranslators();
}

void LocalizationManager::setApplicationLanguage(const QString &languageCode)
{
    if (currentLanguage == languageCode) {
        return;
    }
    
    loadTranslation(languageCode);
    currentLanguage = languageCode;
    
    // Save preference
    QSettings settings;
    settings.setValue("language", languageCode);
    
    emit languageChanged(languageCode);
    qDebug() << "Language changed to:" << languageCode;
}

void LocalizationManager::loadTranslation(const QString &languageCode)
{
    if (!supportedLanguages.contains(languageCode)) {
        qWarning() << "Unsupported language:" << languageCode;
        return;
    }
    
    // Load .qm translation file
    const QString translationDir = QStandardPaths::locate(
        QStandardPaths::GenericDataLocation, 
        "arbor/translations", 
        QStandardPaths::LocateDirectory
    );
    
    QTranslator *translator = new QTranslator;
    if (translator->load(translationDir + "/arbor_" + languageCode + ".qm")) {
        qApp->installTranslator(translator);
        translators[languageCode] = translator;
        emit translationLoaded(languageCode);
        qDebug() << "Translation loaded:" << languageCode;
    } else {
        qWarning() << "Failed to load translation:" << languageCode;
        delete translator;
    }
}

QString LocalizationManager::getCurrentLanguage() const
{
    return currentLanguage;
}

QStringList LocalizationManager::getSupportedLanguages() const
{
    return supportedLanguages.keys();
}

QList<LanguageInfo> LocalizationManager::getLanguageInfoList() const
{
    QList<LanguageInfo> list;
    for (const auto &info : supportedLanguages) {
        list.append(info);
    }
    return list;
}

LanguageInfo LocalizationManager::getLanguageInfo(const QString &code) const
{
    return supportedLanguages.value(code);
}

QString LocalizationManager::translate(const QString &context, const QString &sourceText)
{
    const auto translated = QCoreApplication::translate(context.toLatin1(), sourceText.toLatin1());
    if (translated == sourceText) {
        emit translationMissing(context, sourceText);
    }
    return translated;
}

QString LocalizationManager::translatePlural(const QString &context, const QString &singular,
                                             const QString &plural, int n)
{
    // Use Qt's translation system for plurals
    if (n == 1) {
        return translate(context, singular);
    } else {
        return translate(context, plural);
    }
}

QString LocalizationManager::formatDate(const QDate &date) const
{
    QLocale locale(currentLanguage);
    return locale.toString(date, QLocale::LongFormat);
}

QString LocalizationManager::formatTime(const QTime &time) const
{
    QLocale locale(currentLanguage);
    return locale.toString(time, QLocale::LongFormat);
}

QString LocalizationManager::formatDateTime(const QDateTime &dateTime) const
{
    QLocale locale(currentLanguage);
    return locale.toString(dateTime, QLocale::LongFormat);
}

QString LocalizationManager::formatNumber(int number) const
{
    QLocale locale(currentLanguage);
    return locale.toString(number);
}

QString LocalizationManager::formatCurrency(double amount) const
{
    QLocale locale(currentLanguage);
    return locale.toCurrencyString(amount);
}

void LocalizationManager::initializeSupportedLanguages()
{
    // Supported languages with metadata
    supportedLanguages["en"] = LanguageInfo{
        "en", "English", "English", "en_US", 100, true
    };
    
    supportedLanguages["de"] = LanguageInfo{
        "de", "Deutsch", "German", "de_DE", 75, false
    };
    
    supportedLanguages["fr"] = LanguageInfo{
        "fr", "Français", "French", "fr_FR", 72, false
    };
    
    supportedLanguages["es"] = LanguageInfo{
        "es", "Español", "Spanish", "es_ES", 68, false
    };
    
    supportedLanguages["ja"] = LanguageInfo{
        "ja", "日本語", "Japanese", "ja_JP", 45, false
    };
    
    supportedLanguages["zh"] = LanguageInfo{
        "zh", "中文 (简体)", "Chinese (Simplified)", "zh_CN", 50, false
    };
    
    supportedLanguages["ru"] = LanguageInfo{
        "ru", "Русский", "Russian", "ru_RU", 55, false
    };
}

void LocalizationManager::registerTranslators()
{
    // Register default translator (English)
    QTranslator *defaultTranslator = new QTranslator;
    qApp->installTranslator(defaultTranslator);
    translators["en"] = defaultTranslator;
}

} // namespace Arbor::Localization
