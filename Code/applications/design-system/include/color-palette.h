#pragma once

#include <QColor>
#include <QString>
#include <QMap>

namespace Arbor::Design {

/**
 * @brief Arbor Color Palette v1
 * 
 * Unified color system for all ArborOS applications.
 * Supports dark theme (default) and light theme.
 */
class ColorPalette {
public:
    enum class Theme {
        Dark,
        Light
    };

    enum class SemanticRole {
        SurfaceBase,
        SurfaceElev1,
        SurfaceElev2,
        AccentPrimary,
        AccentHover,
        TextPrimary,
        TextMuted,
        BorderNeutral,
        SemanticSuccess,
        SemanticWarning,
        SemanticDanger
    };

    /**
     * Get color by semantic role
     */
    static QColor color(SemanticRole role, Theme theme = Theme::Dark);

    /**
     * Get color by name (for CSS/QSS)
     */
    static QColor colorByName(const QString& name, Theme theme = Theme::Dark);

    /**
     * Set active theme
     */
    static void setTheme(Theme theme);

    /**
     * Get current theme
     */
    static Theme currentTheme();

    /**
     * Dark theme colors
     */
    static constexpr const char* DARK_SURFACE_BASE = "#0F1215";
    static constexpr const char* DARK_SURFACE_ELEV1 = "#161B22";
    static constexpr const char* DARK_SURFACE_ELEV2 = "#21262D";
    static constexpr const char* DARK_ACCENT_PRIMARY = "#2DD4BF";
    static constexpr const char* DARK_ACCENT_HOVER = "#5EEAD4";
    static constexpr const char* DARK_TEXT_PRIMARY = "#F0F6FC";
    static constexpr const char* DARK_TEXT_MUTED = "#8B949E";
    static constexpr const char* DARK_BORDER_NEUTRAL = "rgba(255, 255, 255, 0.08)";
    static constexpr const char* DARK_SEMANTIC_SUCCESS = "#3FB950";
    static constexpr const char* DARK_SEMANTIC_WARNING = "#F0883E";
    static constexpr const char* DARK_SEMANTIC_DANGER = "#F85149";

    /**
     * Light theme colors
     */
    static constexpr const char* LIGHT_SURFACE_BASE = "#F6F8FA";
    static constexpr const char* LIGHT_SURFACE_ELEV1 = "#FFFFFF";
    static constexpr const char* LIGHT_SURFACE_ELEV2 = "#EEF2F6";
    static constexpr const char* LIGHT_ACCENT_PRIMARY = "#0D9488";
    static constexpr const char* LIGHT_TEXT_PRIMARY = "#1F2328";
    static constexpr const char* LIGHT_TEXT_MUTED = "#67707E";
    static constexpr const char* LIGHT_BORDER_NEUTRAL = "rgba(0, 0, 0, 0.08)";

private:
    static Theme s_currentTheme;
    static QMap<QString, QColor> s_colorMap;

    static void initializeColorMap();
};

} // namespace Arbor::Design
