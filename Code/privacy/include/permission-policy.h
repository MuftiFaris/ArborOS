#ifndef PERMISSION_POLICY_H
#define PERMISSION_POLICY_H

#include <QString>
#include <QMap>
#include <QList>
#include "privacy-manager.h"

/**
 * PermissionPolicy - System-wide permission policies and enforcement
 *
 * Defines which permissions are allowed by default for different app types,
 * enforces policy rules, and provides policy decision making.
 */

class PermissionPolicy
{
public:
    // App classification types
    enum AppType {
        SystemApp,          // Core OS apps (Files, Terminal, Settings)
        UtilityApp,         // Helper apps (text editor, calculator)
        MultiMediaApp,      // Apps that legitimately need audio/video (video player, video call)
        ProductivityApp,    // Office-like apps
        DeveloperApp,       // Development tools
        GameApp,            // Games
        UnknownApp          // Unclassified
    };

    // Policy decision
    struct PolicyDecision {
        bool allowed;
        QString reason;
        bool requiresUserConfirmation;
    };

    // Permission requirement
    struct PermissionRequirement {
        PrivacyManager::PermissionCategory category;
        QString justification;  // Why app needs this
        bool isCritical;        // App won't work without it
    };

    PermissionPolicy();
    ~PermissionPolicy();

    // Classify application
    static AppType classifyApp(const QString& appId, const QString& appName);

    // Get default permissions for app type
    static QList<PrivacyManager::PermissionCategory> getDefaultPermissionsForType(AppType type);

    // Evaluate if permission should be allowed
    static PolicyDecision evaluatePermissionRequest(
        const QString& appId,
        PrivacyManager::PermissionCategory category,
        AppType appType,
        const QString& justification = ""
    );

    // Check if permission is required for app type
    static bool isPermissionRequiredForType(AppType type, PrivacyManager::PermissionCategory category);

    // Get system-defined dangerous permissions
    static QList<PrivacyManager::PermissionCategory> getDangerousPermissions();

    // Get system-defined benign permissions
    static QList<PrivacyManager::PermissionCategory> getBenignPermissions();

    // Validate permission request against policy
    static QString validatePermissionRequest(
        const QString& appId,
        PrivacyManager::PermissionCategory category,
        const QString& details
    );

    // Get permission explanation for user
    static QString getPermissionExplanation(PrivacyManager::PermissionCategory category);
    static QString getPermissionWarning(PrivacyManager::PermissionCategory category);
    static QString getAppTypeExplanation(AppType type);

    // Permission recommendations
    static QString recommendPermissionsForApp(const QString& appId, AppType appType);
    static QList<PermissionRequirement> getPermissionRequirements(const QString& appId);

    // Policy enforcement
    static bool enforcePolicy(const QString& appId, PrivacyManager::PermissionCategory category);

    // Initialize policy defaults
    static void initializeDefaultPolicies();

private:
    // Permission metadata
    struct PermissionMetadata {
        PrivacyManager::PermissionCategory category;
        QString displayName;
        QString description;
        QString userExplanation;
        QString warning;
        bool isDangerous;
        int privacyRiskScore;  // 0-100
        QList<AppType> typicallyNeeded;
    };

    // Policy rules
    struct PolicyRule {
        AppType appType;
        PrivacyManager::PermissionCategory category;
        PrivacyManager::PermissionState defaultState;
        bool canOverride;
    };

    static QMap<PrivacyManager::PermissionCategory, PermissionMetadata> s_permissionMetadata;
    static QList<PolicyRule> s_policyRules;
    static bool s_initialized;

    // Helper methods
    static void initializePermissionMetadata();
    static void initializePolicyRules();
    static PermissionMetadata getPermissionMetadata(PrivacyManager::PermissionCategory category);
};

#endif // PERMISSION_POLICY_H
