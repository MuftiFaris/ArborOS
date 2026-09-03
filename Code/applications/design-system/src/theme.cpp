#include "theme.h"

namespace Arbor::Design {

QFont Theme::fontDisplayL() {
    QFont font(FONT_FAMILY_DISPLAY);
    font.setPixelSize(FONT_SIZE_DISPLAY_L);
    font.setWeight(FONT_WEIGHT_SEMIBOLD);
    return font;
}

QFont Theme::fontHeading1() {
    QFont font(FONT_FAMILY_PRIMARY);
    font.setPixelSize(FONT_SIZE_HEADING_1);
    font.setWeight(FONT_WEIGHT_SEMIBOLD);
    return font;
}

QFont Theme::fontHeading2() {
    QFont font(FONT_FAMILY_PRIMARY);
    font.setPixelSize(FONT_SIZE_HEADING_2);
    font.setWeight(FONT_WEIGHT_MEDIUM);
    return font;
}

QFont Theme::fontBody() {
    QFont font(FONT_FAMILY_PRIMARY);
    font.setPixelSize(FONT_SIZE_BODY);
    font.setWeight(FONT_WEIGHT_REGULAR);
    return font;
}

QFont Theme::fontCaption() {
    QFont font(FONT_FAMILY_PRIMARY);
    font.setPixelSize(FONT_SIZE_CAPTION);
    font.setWeight(FONT_WEIGHT_REGULAR);
    return font;
}

QFont Theme::fontCode() {
    QFont font(FONT_FAMILY_MONOSPACE);
    font.setPixelSize(FONT_SIZE_CODE);
    font.setWeight(FONT_WEIGHT_REGULAR);
    return font;
}

QMargins Theme::paddingSmall() {
    return QMargins(SPACING_SM, SPACING_SM, SPACING_SM, SPACING_SM);
}

QMargins Theme::paddingMedium() {
    return QMargins(SPACING_LG, SPACING_LG, SPACING_LG, SPACING_LG);
}

QMargins Theme::paddingLarge() {
    return QMargins(SPACING_2XL, SPACING_2XL, SPACING_2XL, SPACING_2XL);
}

} // namespace Arbor::Design
