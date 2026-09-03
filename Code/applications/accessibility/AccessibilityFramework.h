#ifndef ARBOR_ACCESSIBILITY_FRAMEWORK_H
#define ARBOR_ACCESSIBILITY_FRAMEWORK_H

#include <QString>
#include <QObject>
#include <QWidget>
#include <QAccessible>
#include <QAccessibleInterface>
#include <memory>

namespace Arbor::Accessibility {

/**
 * @enum AccessibilityLevel
 * @brief Accessibility compliance levels
 */
enum class AccessibilityLevel {
    WCAG_A,      ///< Level A (basic)
    WCAG_AA,     ///< Level AA (enhanced) - ArborOS target
    WCAG_AAA     ///< Level AAA (advanced)
};

/**
 * @struct AccessibilityIssue
 * @brief Detected accessibility problem
 */
struct AccessibilityIssue {
    QString id;              ///< Issue identifier
    QString widget;          ///< Widget/component name
    QString description;     ///< Issue description
    QString severity;        ///< "critical", "major", "minor"
    QString wcagCriterion;   ///< WCAG criterion (e.g., "1.4.3", "2.1.1")
    QString remediation;     ///< How to fix
};

/**
 * @struct AccessibilityAuditReport
 * @brief Results from accessibility audit
 */
struct AccessibilityAuditReport {
    QString appName;
    AccessibilityLevel targetLevel;
    int issueCount;
    int criticalCount;
    int majorCount;
    int minorCount;
    double complianceScore;  ///< 0-100%
    QList<AccessibilityIssue> issues;
    QString timestamp;
};

/**
 * @class AccessibilityProvider
 * @brief Central accessibility configuration and management
 *
 * Provides:
 * - Screen reader support
 * - Keyboard navigation
 * - High contrast modes
 * - Focus management
 * - Text scaling
 */
class AccessibilityProvider : public QObject {
    Q_OBJECT

public:
    static AccessibilityProvider* instance();

    /// Configuration
    void enableScreenReaderMode(bool enable);
    void setHighContrastMode(bool enable);
    void setTextScaleFactor(float scale); // 0.75 - 2.0
    void enableReducedMotion(bool enable);
    void enableHighContrast(bool enable);
    
    /// Query
    bool isScreenReaderEnabled() const;
    bool isHighContrastEnabled() const;
    float getTextScaleFactor() const;
    bool isReducedMotionEnabled() const;
    
    /// Keyboard navigation
    void focusNextWidget();
    void focusPreviousWidget();
    QWidget* getCurrentFocusWidget() const;
    void setTabOrder(const QList<QWidget*> &widgets);
    
    /// Announcements (for screen readers)
    void announceMessage(const QString &message);
    void announceStatus(const QString &status);
    
    /// Widget accessibility setup
    void setupAccessibilityFor(QWidget *widget, const QString &role, const QString &name);
    void setAccessibleDescription(QWidget *widget, const QString &description);
    void setAccessibleHints(QWidget *widget, const QString &hints);

signals:
    void screenReaderModeChanged(bool enabled);
    void highContrastModeChanged(bool enabled);
    void textScaleFactorChanged(float scale);
    void reducedMotionChanged(bool enabled);
    void announcementNeeded(const QString &message);

private:
    AccessibilityProvider();
    static std::unique_ptr<AccessibilityProvider> instance_;
    
    bool screenReaderEnabled;
    bool highContrastEnabled;
    float textScaleFactor;
    bool reducedMotionEnabled;
};

/**
 * @class AccessibilityAuditor
 * @brief Performs accessibility compliance audits
 *
 * Tests:
 * - Color contrast ratios (WCAG AA: 4.5:1 normal, 3:1 large)
 * - Keyboard accessibility (all functions via keyboard)
 * - Focus visibility (clear focus indicators)
 * - Text alternatives (alt text, labels)
 * - Semantic structure (headings, lists, roles)
 * - Motion/animation (reduced motion support)
 * - Form accessibility (labels, error messages)
 */
class AccessibilityAuditor : public QObject {
    Q_OBJECT

public:
    explicit AccessibilityAuditor(QWidget *rootWidget);
    
    /// Run audit
    AccessibilityAuditReport auditWidget(QWidget *widget);
    AccessibilityAuditReport auditApplication();
    
    /// Specific tests
    void testColorContrast();
    void testKeyboardNavigation();
    void testFocusVisibility();
    void testTextAlternatives();
    void testSemanticStructure();
    void testMotionSupport();
    void testFormAccessibility();

signals:
    void auditProgress(int percent);
    void issueFound(const AccessibilityIssue &issue);
    void auditComplete(const AccessibilityAuditReport &report);

private:
    /// Analysis helpers
    bool checkColorContrast(const QColor &foreground, const QColor &background);
    double calculateContrastRatio(const QColor &fg, const QColor &bg) const;
    bool hasAccessibleName(QWidget *widget) const;
    bool hasAccessibleDescription(QWidget *widget) const;
    bool isFocusable(QWidget *widget) const;
    bool hasFocusIndicator(QWidget *widget) const;
    
    QWidget *rootWidget;
    AccessibilityAuditReport currentReport;
};

} // namespace Arbor::Accessibility

#endif // ARBOR_ACCESSIBILITY_FRAMEWORK_H
