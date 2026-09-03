#pragma once

#include <QString>
#include <QWidget>
#include "color-palette.h"

namespace Arbor::Design {

/**
 * @brief Arbor Style Provider
 * 
 * Applies consistent styling across all applications.
 * Manages QSS stylesheets and theme switching.
 */
class StyleProvider {
public:
    /**
     * Initialize style provider (call once at app startup)
     */
    static void initialize();

    /**
     * Apply Arbor theme to a widget/application
     */
    static void applyStyle(QWidget* widget = nullptr);

    /**
     * Switch theme and update all widgets
     */
    static void setTheme(ColorPalette::Theme theme);

    /**
     * Get stylesheet as string
     */
    static QString getStylesheet(ColorPalette::Theme theme);

    /**
     * Set custom stylesheet path (for development)
     */
    static void setStylesheetPath(const QString& path);

private:
    static QString s_stylesheetPath;
    static bool s_initialized;

    static QString loadStylesheet(const QString& path);
};

} // namespace Arbor::Design
