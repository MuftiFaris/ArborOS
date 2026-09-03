#pragma once

#include <QString>
#include <QFont>
#include <QSize>
#include <QMargins>

namespace Arbor::Design {

/**
 * @brief Arbor Design System Theme
 * 
 * Centralized typography, spacing, and styling constants.
 */
class Theme {
public:
    // Font sizes
    static constexpr int FONT_SIZE_DISPLAY_L = 36;
    static constexpr int FONT_SIZE_HEADING_1 = 24;
    static constexpr int FONT_SIZE_HEADING_2 = 18;
    static constexpr int FONT_SIZE_BODY = 14;
    static constexpr int FONT_SIZE_CAPTION = 12;
    static constexpr int FONT_SIZE_CODE = 13;

    // Font families
    static constexpr const char* FONT_FAMILY_PRIMARY = "Inter";
    static constexpr const char* FONT_FAMILY_DISPLAY = "Inter Display";
    static constexpr const char* FONT_FAMILY_MONOSPACE = "JetBrains Mono";

    // Font weights
    static constexpr int FONT_WEIGHT_REGULAR = 400;
    static constexpr int FONT_WEIGHT_MEDIUM = 500;
    static constexpr int FONT_WEIGHT_SEMIBOLD = 600;

    // Line heights
    static constexpr int LINE_HEIGHT_DISPLAY_L = 44;
    static constexpr int LINE_HEIGHT_HEADING_1 = 32;
    static constexpr int LINE_HEIGHT_HEADING_2 = 24;
    static constexpr int LINE_HEIGHT_BODY = 20;
    static constexpr int LINE_HEIGHT_CAPTION = 16;
    static constexpr int LINE_HEIGHT_CODE = 18;

    // Spacing grid (8px baseline)
    static constexpr int SPACING_XS = 4;     // Micro-spacing
    static constexpr int SPACING_SM = 8;     // Small padding
    static constexpr int SPACING_MD = 12;    // Medium padding
    static constexpr int SPACING_LG = 16;    // Large padding
    static constexpr int SPACING_XL = 24;    // Extra large padding
    static constexpr int SPACING_2XL = 32;   // 2x extra large

    // Border radius
    static constexpr int BORDER_RADIUS_SM = 4;
    static constexpr int BORDER_RADIUS_MD = 8;
    static constexpr int BORDER_RADIUS_LG = 12;
    static constexpr int BORDER_RADIUS_SQUIRCLE = 22; // App icon squircle

    // Focus ring
    static constexpr int FOCUS_RING_WIDTH = 2;
    static constexpr const char* FOCUS_RING_COLOR_DARK = "#2DD4BF";
    static constexpr const char* FOCUS_RING_COLOR_LIGHT = "#0D9488";

    // Animation timing
    static constexpr int ANIMATION_DURATION_INSTANT = 0;
    static constexpr int ANIMATION_DURATION_SHORT = 100;
    static constexpr int ANIMATION_DURATION_NORMAL = 200;
    static constexpr int ANIMATION_DURATION_LONG = 400;

    // Spring physics (for smooth animations)
    static constexpr float SPRING_STIFFNESS = 300.0f;
    static constexpr float SPRING_DAMPING = 30.0f;

    // Shadow depths (for elevation)
    static constexpr const char* SHADOW_NONE = "";
    static constexpr const char* SHADOW_SM = "0px 2px 4px rgba(0, 0, 0, 0.1)";
    static constexpr const char* SHADOW_MD = "0px 4px 12px rgba(0, 0, 0, 0.15)";
    static constexpr const char* SHADOW_LG = "0px 12px 24px rgba(0, 0, 0, 0.2)";

    // Font creation helpers
    static QFont fontDisplayL();
    static QFont fontHeading1();
    static QFont fontHeading2();
    static QFont fontBody();
    static QFont fontCaption();
    static QFont fontCode();

    // Common margins
    static QMargins paddingSmall();
    static QMargins paddingMedium();
    static QMargins paddingLarge();
};

} // namespace Arbor::Design
