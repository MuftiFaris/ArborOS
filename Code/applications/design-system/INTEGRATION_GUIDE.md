# Arbor Design System Integration Guide

Complete guide for using the Arbor Design System in applications.

## Quick Start

### 1. Link Design System Library

In your application's `CMakeLists.txt`:

```cmake
target_link_libraries(your-app PRIVATE arbor-design-system)
target_include_directories(your-app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../design-system/include)
```

### 2. Initialize in Main

In your `main.cpp`:

```cpp
#include <QApplication>
#include "style-provider.h"
#include "color-palette.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Initialize design system
    Arbor::Design::StyleProvider::initialize();
    
    // Apply theme (dark is default)
    Arbor::Design::ColorPalette::setTheme(Arbor::Design::ColorPalette::Theme::Dark);
    Arbor::Design::StyleProvider::applyStyle();
    
    // Your application code...
    
    return app.exec();
}
```

### 3. Use Colors in Code

```cpp
#include "color-palette.h"

void MyClass::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    
    // Get semantic color
    QColor bgColor = Arbor::Design::ColorPalette::color(
        Arbor::Design::ColorPalette::SemanticRole::SurfaceBase,
        Arbor::Design::ColorPalette::Theme::Dark
    );
    
    painter.fillRect(rect(), bgColor);
}
```

### 4. Use Typography

```cpp
#include "theme.h"
#include <QLabel>

auto label = new QLabel("Hello, Arbor!");
label->setFont(Arbor::Design::Theme::fontHeading1());
label->setAlignment(Qt::AlignCenter);
```

## Color System

### Semantic Roles

All colors use semantic roles, not direct hex values. This allows theme switching.

**Base Colors:**
- `SurfaceBase` — Primary background (windows, dialogs)
- `SurfaceElev1` — Elevated surfaces (panels, sidebars)
- `SurfaceElev2` — Further elevated (cards, windows)

**Text Colors:**
- `TextPrimary` — Main text, labels
- `TextMuted` — Secondary text, placeholders

**Accent Colors:**
- `AccentPrimary` — Links, highlights, primary actions
- `AccentHover` — Hover state for accent elements

**Semantic Colors:**
- `SemanticSuccess` — Valid input, success messages
- `SemanticWarning` — Warning messages, caution states
- `SemanticDanger` — Errors, destructive actions

**Border:**
- `BorderNeutral` — Dividers, UI edges

### Using Colors

```cpp
#include "color-palette.h"

// Dark theme
QColor accent = Arbor::Design::ColorPalette::color(
    Arbor::Design::ColorPalette::SemanticRole::AccentPrimary,
    Arbor::Design::ColorPalette::Theme::Dark
);

// Light theme
QColor text = Arbor::Design::ColorPalette::color(
    Arbor::Design::ColorPalette::SemanticRole::TextPrimary,
    Arbor::Design::ColorPalette::Theme::Light
);

// Current theme (set via setTheme())
QColor current = Arbor::Design::ColorPalette::color(
    Arbor::Design::ColorPalette::SemanticRole::SurfaceBase
);
```

## Typography

### Font Scales

Use predefined font scales for consistency:

```cpp
#include "theme.h"

QLabel* heading1 = new QLabel("Main Title");
heading1->setFont(Arbor::Design::Theme::fontHeading1());

QLabel* body = new QLabel("Regular text content");
body->setFont(Arbor::Design::Theme::fontBody());

QLabel* code = new QLabel("monospace code");
code->setFont(Arbor::Design::Theme::fontCode());
```

### Font Families

- **Primary UI:** `Inter` (sans-serif)
- **Display/Headers:** `Inter Display` (semi-bold sans-serif)
- **Code/Terminal:** `JetBrains Mono` (monospace)

### Sizes

| Scale | Size | Weight | Usage |
|-------|------|--------|-------|
| Display L | 36px | 600 | Hero/page titles |
| Heading 1 | 24px | 600 | Main section titles |
| Heading 2 | 18px | 500 | Subsection titles |
| Body | 14px | 400 | Main text, labels |
| Caption | 12px | 400 | Helper text, muted |
| Code | 13px | 400 | Code blocks, terminal |

## Spacing

### Grid System (8px baseline)

```cpp
#include "theme.h"

// Predefined spacing constants
int xs = Arbor::Design::Theme::SPACING_XS;      // 4px
int sm = Arbor::Design::Theme::SPACING_SM;      // 8px
int md = Arbor::Design::Theme::SPACING_MD;      // 12px
int lg = Arbor::Design::Theme::SPACING_LG;      // 16px
int xl = Arbor::Design::Theme::SPACING_XL;      // 24px
int xxl = Arbor::Design::Theme::SPACING_2XL;    // 32px

// Use for layout
layout->setContentsMargins(lg, lg, lg, lg);      // 16px on all sides
layout->setSpacing(sm);                          // 8px between items
```

### Padding Helpers

```cpp
#include "theme.h"

// Quick padding margin objects
QMargins small = Arbor::Design::Theme::paddingSmall();      // 8px
QMargins medium = Arbor::Design::Theme::paddingMedium();    // 16px
QMargins large = Arbor::Design::Theme::paddingLarge();      // 32px

widget->setContentsMargins(medium);
```

## Border Radius

```cpp
#include "theme.h"

// Predefined border radius values
int small = Arbor::Design::Theme::BORDER_RADIUS_SM;      // 4px - inputs
int medium = Arbor::Design::Theme::BORDER_RADIUS_MD;     // 8px - cards
int large = Arbor::Design::Theme::BORDER_RADIUS_LG;      // 12px - dialogs
int squircle = Arbor::Design::Theme::BORDER_RADIUS_SQUIRCLE; // 22px - app icons
```

## Stylesheets (QSS)

The design system includes comprehensive QSS stylesheets for Qt widgets.

### Loading Stylesheet

```cpp
#include "style-provider.h"

// Automatic stylesheet loading
Arbor::Design::StyleProvider::applyStyle();  // Applies to all widgets

// Or apply to specific widget
Arbor::Design::StyleProvider::applyStyle(myWidget);
```

### Custom Stylesheet

The QSS stylesheets handle all standard Qt widgets:
- QPushButton, QToolButton
- QLineEdit, QTextEdit, QPlainTextEdit
- QComboBox, QSpinBox, QDoubleSpinBox
- QCheckBox, QRadioButton
- QTreeView, QListView, QTableView
- QMenu, QMenuBar
- QProgressBar, QSlider
- QTabBar, QDockWidget
- QScrollBar, QToolBar, QStatusBar

## Animations & Transitions

### Spring Physics

Use spring curves for smooth, natural animations:

```cpp
// Constants
float stiffness = Arbor::Design::Theme::SPRING_STIFFNESS;  // 300
float damping = Arbor::Design::Theme::SPRING_DAMPING;      // 30

// For QPropertyAnimation with spring easing
QEasingCurve curve(QEasingCurve::OutElastic);
animation->setEasingCurve(curve);
```

### Timing

```cpp
// Predefined animation durations
int instant = Arbor::Design::Theme::ANIMATION_DURATION_INSTANT;   // 0ms
int short_anim = Arbor::Design::Theme::ANIMATION_DURATION_SHORT;  // 100ms
int normal = Arbor::Design::Theme::ANIMATION_DURATION_NORMAL;     // 200ms
int long_anim = Arbor::Design::Theme::ANIMATION_DURATION_LONG;    // 400ms
```

## Accessibility

### Focus Ring

The design system automatically applies focus rings to interactive elements.

For custom widgets:

```cpp
// In your QSS or stylesheet
*:focus {
    outline: 2px solid #2DD4BF;
    outline-offset: 2px;
}
```

### High Contrast Mode

Apply high contrast styling:

```cpp
// Add CSS class to enable high contrast
widget->setProperty("class", "HighContrast");
```

High contrast QSS is built-in:
- Black/white inversion
- Thicker borders
- More visible focus indicators

### Reduced Motion

Respect user's motion preferences:

```cpp
#include <QGuiApplication>

// Check if user prefers reduced motion
bool reducedMotion = qApp->palette().color(QPalette::Base) == 
                     qApp->palette().color(QPalette::Window);

if (reducedMotion) {
    // Disable animations
    animation->setDuration(0);
}
```

## Theme Switching

### Runtime Theme Switching

```cpp
#include "style-provider.h"

// Switch to light theme
Arbor::Design::StyleProvider::setTheme(
    Arbor::Design::ColorPalette::Theme::Light
);

// Switch to dark theme
Arbor::Design::StyleProvider::setTheme(
    Arbor::Design::ColorPalette::Theme::Dark
);
```

### Listening to Theme Changes

```cpp
#include "color-palette.h"

// After theme switch, query current theme
auto current = Arbor::Design::ColorPalette::currentTheme();

if (current == Arbor::Design::ColorPalette::Theme::Dark) {
    // React to dark theme
} else {
    // React to light theme
}
```

## Best Practices

### 1. Always Use Semantic Roles

**Good:**
```cpp
QColor bg = Arbor::Design::ColorPalette::color(
    Arbor::Design::ColorPalette::SemanticRole::SurfaceBase
);
```

**Bad:**
```cpp
QColor bg("#0F1215");  // Hard-coded color
```

### 2. Use Predefined Spacing

**Good:**
```cpp
layout->setContentsMargins(
    Arbor::Design::Theme::SPACING_LG,
    Arbor::Design::Theme::SPACING_LG,
    Arbor::Design::Theme::SPACING_LG,
    Arbor::Design::Theme::SPACING_LG
);
```

**Bad:**
```cpp
layout->setContentsMargins(16, 16, 16, 16);  // Magic numbers
```

### 3. Prefer QSS for Styling

**Good:**
```cpp
// Let QSS handle all widget styling
Arbor::Design::StyleProvider::applyStyle();
```

**Bad:**
```cpp
// Manual QPalette/stylesheet manipulation
button->setStyleSheet("background-color: #2DD4BF; color: #0F1215;");
```

### 4. Always Declare Focus Styles

Interactive elements must have visible focus indicators for accessibility.

### 5. Test Both Themes

Test your UI in both dark and light themes to ensure consistency and readability.

## Troubleshooting

### Colors Not Updating on Theme Switch

Ensure you're using `colorByName()` or `color()` methods instead of caching colors:

```cpp
// Bad - color is cached at init time
QColor color = Arbor::Design::ColorPalette::color(...);
button->setPalette(palette);  // Won't update when theme changes

// Good - color retrieved on demand
void updateColors() {
    QColor color = Arbor::Design::ColorPalette::color(...);
    button->setPalette(palette);
}
```

### Stylesheet Not Applying

Verify stylesheets are loaded:

```cpp
// Check if stylesheet path is valid
Arbor::Design::StyleProvider::setStylesheetPath("/path/to/arbor-dark.qss");
```

### Focus Ring Not Visible

Ensure focus ring CSS is not overridden:

```cpp
// Don't use outline: none on any widget
// The design system's focus ring will be invisible
```

## Examples

### Complete Application

```cpp
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include "style-provider.h"
#include "color-palette.h"
#include "theme.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Initialize design system
    Arbor::Design::StyleProvider::initialize();
    Arbor::Design::StyleProvider::applyStyle();
    
    // Create main window
    QMainWindow window;
    window.setWindowTitle("Arbor Application");
    
    // Create button
    QPushButton* button = new QPushButton("Click Me");
    button->setFont(Arbor::Design::Theme::fontBody());
    
    // Create layout
    QWidget* central = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(central);
    layout->setContentsMargins(
        Arbor::Design::Theme::SPACING_LG,
        Arbor::Design::Theme::SPACING_LG,
        Arbor::Design::Theme::SPACING_LG,
        Arbor::Design::Theme::SPACING_LG
    );
    layout->addWidget(button);
    
    window.setCentralWidget(central);
    window.show();
    
    return app.exec();
}
```

## References

- [Arbor OS UX Design](../../Documentation/02-UX-Design/ux-design.md)
- [Qt6 Stylesheet Documentation](https://doc.qt.io/qt-6/stylesheet.html)
- [Qt6 Palette Documentation](https://doc.qt.io/qt-6/qpalette.html)

---

**Design System Version:** 1.0  
**Last Updated:** 2026-09-02
