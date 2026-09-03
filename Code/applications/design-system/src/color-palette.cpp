#include "color-palette.h"
#include <QDebug>

namespace Arbor::Design {

// Static member initialization
ColorPalette::Theme ColorPalette::s_currentTheme = Theme::Dark;
QMap<QString, QColor> ColorPalette::s_colorMap;

QColor ColorPalette::color(SemanticRole role, Theme theme) {
    const char* colorStr = nullptr;

    if (theme == Theme::Dark) {
        switch (role) {
            case SemanticRole::SurfaceBase:      colorStr = DARK_SURFACE_BASE; break;
            case SemanticRole::SurfaceElev1:     colorStr = DARK_SURFACE_ELEV1; break;
            case SemanticRole::SurfaceElev2:     colorStr = DARK_SURFACE_ELEV2; break;
            case SemanticRole::AccentPrimary:    colorStr = DARK_ACCENT_PRIMARY; break;
            case SemanticRole::AccentHover:      colorStr = DARK_ACCENT_HOVER; break;
            case SemanticRole::TextPrimary:      colorStr = DARK_TEXT_PRIMARY; break;
            case SemanticRole::TextMuted:        colorStr = DARK_TEXT_MUTED; break;
            case SemanticRole::BorderNeutral:    colorStr = DARK_BORDER_NEUTRAL; break;
            case SemanticRole::SemanticSuccess:  colorStr = DARK_SEMANTIC_SUCCESS; break;
            case SemanticRole::SemanticWarning:  colorStr = DARK_SEMANTIC_WARNING; break;
            case SemanticRole::SemanticDanger:   colorStr = DARK_SEMANTIC_DANGER; break;
        }
    } else {
        switch (role) {
            case SemanticRole::SurfaceBase:      colorStr = LIGHT_SURFACE_BASE; break;
            case SemanticRole::SurfaceElev1:     colorStr = LIGHT_SURFACE_ELEV1; break;
            case SemanticRole::SurfaceElev2:     colorStr = LIGHT_SURFACE_ELEV2; break;
            case SemanticRole::AccentPrimary:    colorStr = LIGHT_ACCENT_PRIMARY; break;
            case SemanticRole::AccentHover:      colorStr = LIGHT_TEXT_PRIMARY; break;
            case SemanticRole::TextPrimary:      colorStr = LIGHT_TEXT_PRIMARY; break;
            case SemanticRole::TextMuted:        colorStr = LIGHT_TEXT_MUTED; break;
            case SemanticRole::BorderNeutral:    colorStr = LIGHT_BORDER_NEUTRAL; break;
            case SemanticRole::SemanticSuccess:  colorStr = DARK_SEMANTIC_SUCCESS; break;
            case SemanticRole::SemanticWarning:  colorStr = DARK_SEMANTIC_WARNING; break;
            case SemanticRole::SemanticDanger:   colorStr = DARK_SEMANTIC_DANGER; break;
        }
    }

    return QColor(colorStr);
}

QColor ColorPalette::colorByName(const QString& name, Theme theme) {
    initializeColorMap();
    
    QString key = QString("%1_%2").arg(name, theme == Theme::Dark ? "dark" : "light");
    if (s_colorMap.contains(key)) {
        return s_colorMap[key];
    }

    qWarning() << "Color not found:" << name;
    return QColor(DARK_SURFACE_BASE);
}

void ColorPalette::setTheme(Theme theme) {
    s_currentTheme = theme;
}

ColorPalette::Theme ColorPalette::currentTheme() {
    return s_currentTheme;
}

void ColorPalette::initializeColorMap() {
    if (!s_colorMap.empty()) return;

    // Dark theme
    s_colorMap["SurfaceBase_dark"] = QColor(DARK_SURFACE_BASE);
    s_colorMap["SurfaceElev1_dark"] = QColor(DARK_SURFACE_ELEV1);
    s_colorMap["SurfaceElev2_dark"] = QColor(DARK_SURFACE_ELEV2);
    s_colorMap["AccentPrimary_dark"] = QColor(DARK_ACCENT_PRIMARY);
    s_colorMap["AccentHover_dark"] = QColor(DARK_ACCENT_HOVER);
    s_colorMap["TextPrimary_dark"] = QColor(DARK_TEXT_PRIMARY);
    s_colorMap["TextMuted_dark"] = QColor(DARK_TEXT_MUTED);
    s_colorMap["SemanticSuccess_dark"] = QColor(DARK_SEMANTIC_SUCCESS);
    s_colorMap["SemanticWarning_dark"] = QColor(DARK_SEMANTIC_WARNING);
    s_colorMap["SemanticDanger_dark"] = QColor(DARK_SEMANTIC_DANGER);

    // Light theme
    s_colorMap["SurfaceBase_light"] = QColor(LIGHT_SURFACE_BASE);
    s_colorMap["SurfaceElev1_light"] = QColor(LIGHT_SURFACE_ELEV1);
    s_colorMap["SurfaceElev2_light"] = QColor(LIGHT_SURFACE_ELEV2);
    s_colorMap["AccentPrimary_light"] = QColor(LIGHT_ACCENT_PRIMARY);
    s_colorMap["TextPrimary_light"] = QColor(LIGHT_TEXT_PRIMARY);
    s_colorMap["TextMuted_light"] = QColor(LIGHT_TEXT_MUTED);
    s_colorMap["SemanticSuccess_light"] = QColor(DARK_SEMANTIC_SUCCESS);
    s_colorMap["SemanticWarning_light"] = QColor(DARK_SEMANTIC_WARNING);
    s_colorMap["SemanticDanger_light"] = QColor(DARK_SEMANTIC_DANGER);
}

} // namespace Arbor::Design
