#include "style-provider.h"
#include <QApplication>
#include <QFile>
#include <QDebug>

namespace Arbor::Design {

QString StyleProvider::s_stylesheetPath;
bool StyleProvider::s_initialized = false;

void StyleProvider::initialize() {
    if (s_initialized) return;

    // Try to load from standard data directories
    QString paths[] = {
        ":/stylesheets/arbor-dark.qss",
        "/usr/share/arbor/themes/arbor-dark.qss",
        "/usr/local/share/arbor/themes/arbor-dark.qss",
        "./themes/arbor-dark.qss"
    };

    for (const QString& path : paths) {
        QFile file(path);
        if (file.exists()) {
            s_stylesheetPath = path;
            break;
        }
    }

    s_initialized = true;
}

void StyleProvider::applyStyle(QWidget* widget) {
    if (!s_initialized) initialize();

    QString stylesheet = getStylesheet(ColorPalette::currentTheme());
    
    if (widget) {
        widget->setStyleSheet(stylesheet);
    } else if (qApp) {
        qApp->setStyle("Fusion");
        qApp->setStyleSheet(stylesheet);
    }
}

void StyleProvider::setTheme(ColorPalette::Theme theme) {
    ColorPalette::setTheme(theme);
    applyStyle();
}

QString StyleProvider::getStylesheet(ColorPalette::Theme theme) {
    QString stylesheet;

    // Build inline stylesheet with current theme colors
    auto surfaceBase = ColorPalette::color(ColorPalette::SemanticRole::SurfaceBase, theme);
    auto surfaceElev1 = ColorPalette::color(ColorPalette::SemanticRole::SurfaceElev1, theme);
    auto surfaceElev2 = ColorPalette::color(ColorPalette::SemanticRole::SurfaceElev2, theme);
    auto accentPrimary = ColorPalette::color(ColorPalette::SemanticRole::AccentPrimary, theme);
    auto textPrimary = ColorPalette::color(ColorPalette::SemanticRole::TextPrimary, theme);
    auto textMuted = ColorPalette::color(ColorPalette::SemanticRole::TextMuted, theme);
    auto borderNeutral = ColorPalette::color(ColorPalette::SemanticRole::BorderNeutral, theme);

    // QSS Stylesheet
    stylesheet = QString(R"(
        QApplication, QMainWindow, QWidget {
            background-color: %1;
            color: %2;
        }

        QMainWindow {
            background-color: %1;
        }

        QPushButton {
            background-color: %3;
            color: %4;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-weight: bold;
            font-size: 14px;
        }

        QPushButton:hover {
            background-color: %5;
        }

        QPushButton:pressed {
            background-color: %6;
        }

        QPushButton:focus {
            outline: 2px solid %7;
            outline-offset: 2px;
        }

        QLineEdit, QTextEdit, QPlainTextEdit {
            background-color: %8;
            color: %2;
            border: 1px solid %9;
            border-radius: 4px;
            padding: 6px 8px;
            font-size: 14px;
        }

        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
            border: 2px solid %7;
        }

        QLabel {
            color: %2;
            font-size: 14px;
        }

        QTabBar::tab {
            background-color: %8;
            color: %2;
            padding: 6px 12px;
            border: none;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }

        QTabBar::tab:selected {
            background-color: %3;
            color: %4;
        }

        QTreeView, QListView, QTableView {
            background-color: %8;
            color: %2;
            border: 1px solid %9;
            border-radius: 4px;
            gridline-color: %9;
        }

        QTreeView::item:selected, QListView::item:selected, QTableView::item:selected {
            background-color: %3;
            color: %4;
        }

        QMenuBar {
            background-color: %3;
            color: %4;
        }

        QMenuBar::item:selected {
            background-color: %5;
        }

        QMenu {
            background-color: %8;
            color: %2;
            border: 1px solid %9;
            border-radius: 4px;
            padding: 4px 0px;
        }

        QMenu::item:selected {
            background-color: %3;
            color: %4;
        }

        QScrollBar:vertical {
            background-color: %8;
            width: 12px;
            border-radius: 6px;
        }

        QScrollBar::handle:vertical {
            background-color: %6;
            border-radius: 6px;
            min-height: 20px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: %7;
        }

        QComboBox {
            background-color: %8;
            color: %2;
            border: 1px solid %9;
            border-radius: 4px;
            padding: 4px 8px;
        }

        QComboBox:focus {
            border: 2px solid %7;
        }

        QSpinBox, QDoubleSpinBox {
            background-color: %8;
            color: %2;
            border: 1px solid %9;
            border-radius: 4px;
            padding: 4px 8px;
        }
    )")
    .arg(surfaceBase.name())
    .arg(textPrimary.name())
    .arg(accentPrimary.name())
    .arg(surfaceBase.name())
    .arg(ColorPalette::color(ColorPalette::SemanticRole::AccentHover, theme).name())
    .arg(textMuted.name())
    .arg(accentPrimary.name())
    .arg(surfaceElev1.name())
    .arg(borderNeutral.name());

    return stylesheet;
}

void StyleProvider::setStylesheetPath(const QString& path) {
    s_stylesheetPath = path;
}

QString StyleProvider::loadStylesheet(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to load stylesheet:" << path;
        return "";
    }

    QString stylesheet = QString::fromUtf8(file.readAll());
    file.close();
    return stylesheet;
}

} // namespace Arbor::Design
