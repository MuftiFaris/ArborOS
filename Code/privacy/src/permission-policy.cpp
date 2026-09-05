#include "permission-policy.h"
#include <QDebug>

// Static member initialization
QMap<PrivacyManager::PermissionCategory, PermissionPolicy::PermissionMetadata> PermissionPolicy::s_permissionMetadata;
QList<PermissionPolicy::PolicyRule> PermissionPolicy::s_policyRules;
bool PermissionPolicy::s_initialized = false;

PermissionPolicy::PermissionPolicy()
{
    if (!s_initialized) {
        initializeDefaultPolicies();
    }
}

PermissionPolicy::~PermissionPolicy()
{
}

PermissionPolicy::AppType PermissionPolicy::classifyApp(const QString& appId, const QString& appName)
{
    QString lowerIdName = (appId + " " + appName).toLower();

    // System apps
    if (lowerIdName.contains("arbor-files") || lowerIdName.contains("file")) return SystemApp;
    if (lowerIdName.contains("arbor-terminal") || lowerIdName.contains("terminal")) return SystemApp;
    if (lowerIdName.contains("arbor-settings") || lowerIdName.contains("settings")) return SystemApp;
    if (lowerIdName.contains("arbor-software") || lowerIdName.contains("software")) return SystemApp;
    if (lowerIdName.contains("arbor-update") || lowerIdName.contains("update")) return SystemApp;
    if (lowerIdName.contains("arbor-system") || lowerIdName.contains("system")) return SystemApp;

    // Multimedia apps
    if (lowerIdName.contains("video") || lowerIdName.contains("player") ||
        lowerIdName.contains("audio") || lowerIdName.contains("music") ||
        lowerIdName.contains("camera") || lowerIdName.contains("webcam") ||
        lowerIdName.contains("vlc") || lowerIdName.contains("mpv") ||
        lowerIdName.contains("pulse")) {
        return MultiMediaApp;
    }

    // Developer tools
    if (lowerIdName.contains("vscode") || lowerIdName.contains("vim") ||
        lowerIdName.contains("emacs") || lowerIdName.contains("compiler") ||
        lowerIdName.contains("git") || lowerIdName.contains("rust") ||
        lowerIdName.contains("python") || lowerIdName.contains("cmake")) {
        return DeveloperApp;
    }

    // Games
    if (lowerIdName.contains("game") || lowerIdName.contains("unity") ||
        lowerIdName.contains("unreal")) {
        return GameApp;
    }

    // Productivity
    if (lowerIdName.contains("libreoffice") || lowerIdName.contains("writer") ||
        lowerIdName.contains("calc") || lowerIdName.contains("impress") ||
        lowerIdName.contains("office")) {
        return ProductivityApp;
    }

    // Utility
    if (lowerIdName.contains("calculator") || lowerIdName.contains("calc") ||
        lowerIdName.contains("notepad") || lowerIdName.contains("text") ||
        lowerIdName.contains("editor")) {
        return UtilityApp;
    }

    return UnknownApp;
}

QList<PrivacyManager::PermissionCategory> PermissionPolicy::getDefaultPermissionsForType(AppType type)
{
    QList<PrivacyManager::PermissionCategory> permissions;

    switch (type) {
        case SystemApp:
            // System apps get basic permissions by default
            permissions << PrivacyManager::Files;
            permissions << PrivacyManager::Network;
            break;

        case UtilityApp:
            // Utilities get minimal permissions
            permissions << PrivacyManager::Files;  // To save/load
            break;

        case MultiMediaApp:
            // Media apps need audio, video, files
            permissions << PrivacyManager::Microphone;
            permissions << PrivacyManager::Camera;
            permissions << PrivacyManager::Audio;
            permissions << PrivacyManager::Files;
            break;

        case ProductivityApp:
            // Office apps need files, clipboard, maybe printing
            permissions << PrivacyManager::Files;
            permissions << PrivacyManager::Clipboard;
            permissions << PrivacyManager::Printing;
            break;

        case DeveloperApp:
            // Dev tools often need most permissions
            permissions << PrivacyManager::Files;
            permissions << PrivacyManager::Network;
            permissions << PrivacyManager::SystemMonitor;
            break;

        case GameApp:
            // Games vary, but usually limited
            permissions << PrivacyManager::Files;
            permissions << PrivacyManager::Audio;
            break;

        case UnknownApp:
            // Unknown apps get nothing by default
            break;
    }

    return permissions;
}

PermissionPolicy::PolicyDecision PermissionPolicy::evaluatePermissionRequest(
    const QString& appId,
    PrivacyManager::PermissionCategory category,
    AppType appType,
    const QString& justification)
{
    if (!s_initialized) {
        initializeDefaultPolicies();
    }

    PolicyDecision decision;
    decision.requiresUserConfirmation = false;

    // Check if it's a dangerous permission
    QList<PrivacyManager::PermissionCategory> dangerous = getDangerousPermissions();
    if (dangerous.contains(category)) {
        decision.allowed = false;
        decision.reason = "Dangerous permission requires user approval";
        decision.requiresUserConfirmation = true;
        return decision;
    }

    // Check if app type typically needs this permission
    if (isPermissionRequiredForType(appType, category)) {
        decision.allowed = true;
        decision.reason = "Permission is typical for this app type";
        return decision;
    }

    // If justification provided, consider it
    if (!justification.isEmpty()) {
        decision.allowed = true;
        decision.reason = QString("Permission justified: %1").arg(justification);
        decision.requiresUserConfirmation = true;  // Still ask user
        return decision;
    }

    // Default to asking user
    decision.allowed = false;
    decision.reason = "Not typical for app type, user confirmation needed";
    decision.requiresUserConfirmation = true;

    return decision;
}

bool PermissionPolicy::isPermissionRequiredForType(AppType type,
                                                 PrivacyManager::PermissionCategory category)
{
    QList<PrivacyManager::PermissionCategory> defaults = getDefaultPermissionsForType(type);
    return defaults.contains(category);
}

QList<PrivacyManager::PermissionCategory> PermissionPolicy::getDangerousPermissions()
{
    return {
        PrivacyManager::Microphone,
        PrivacyManager::Camera,
        PrivacyManager::Location,
        PrivacyManager::Contacts,
        PrivacyManager::Calendar,
        PrivacyManager::Photos,
        PrivacyManager::SystemMonitor
    };
}

QList<PrivacyManager::PermissionCategory> PermissionPolicy::getBenignPermissions()
{
    return {
        PrivacyManager::Clipboard,
        PrivacyManager::Printing,
        PrivacyManager::USB,
        PrivacyManager::Bluetooth
    };
}

QString PermissionPolicy::validatePermissionRequest(const QString& appId,
                                                   PrivacyManager::PermissionCategory category,
                                                   const QString& details)
{
    // Validate app ID format
    if (appId.isEmpty()) {
        return "App ID cannot be empty";
    }

    if (appId.length() > 255) {
        return "App ID too long (max 255 chars)";
    }

    // Check for suspicious patterns
    if (appId.contains("..") || appId.contains("/") || appId.contains("\\")) {
        return "Invalid characters in app ID";
    }

    return "";  // Valid
}

QString PermissionPolicy::getPermissionExplanation(PrivacyManager::PermissionCategory category)
{
    switch (category) {
        case PrivacyManager::Microphone:
            return "Allows the app to record audio from your microphone";
        case PrivacyManager::Camera:
            return "Allows the app to access your webcam";
        case PrivacyManager::Files:
            return "Allows the app to read and write files in your home directory";
        case PrivacyManager::Network:
            return "Allows the app to connect to the internet";
        case PrivacyManager::Location:
            return "Allows the app to access your approximate location";
        case PrivacyManager::Clipboard:
            return "Allows the app to read what you copy/paste";
        case PrivacyManager::Contacts:
            return "Allows the app to access your contacts list";
        case PrivacyManager::Calendar:
            return "Allows the app to access your calendar events";
        case PrivacyManager::Photos:
            return "Allows the app to access your photo library";
        case PrivacyManager::Bluetooth:
            return "Allows the app to connect to Bluetooth devices";
        case PrivacyManager::USB:
            return "Allows the app to access USB devices";
        case PrivacyManager::Printing:
            return "Allows the app to access your printers";
        case PrivacyManager::Screenshots:
            return "Allows the app to take screenshots";
        case PrivacyManager::SystemMonitor:
            return "Allows the app to monitor system resources";
        case PrivacyManager::Audio:
            return "Allows the app to access system audio";
        default:
            return "Unknown permission";
    }
}

QString PermissionPolicy::getPermissionWarning(PrivacyManager::PermissionCategory category)
{
    switch (category) {
        case PrivacyManager::Microphone:
            return "⚠ WARNING: This allows the app to record everything you say";
        case PrivacyManager::Camera:
            return "⚠ WARNING: This allows the app to see through your webcam";
        case PrivacyManager::Location:
            return "⚠ WARNING: This allows the app to track your location";
        case PrivacyManager::Contacts:
            return "⚠ WARNING: This allows the app to access all your contacts";
        case PrivacyManager::Calendar:
            return "⚠ WARNING: This allows the app to see your calendar";
        case PrivacyManager::Photos:
            return "⚠ WARNING: This allows the app to access all your photos";
        case PrivacyManager::Clipboard:
            return "⚠ WARNING: This allows the app to read everything you copy";
        case PrivacyManager::SystemMonitor:
            return "⚠ WARNING: This allows the app to monitor all running processes";
        default:
            return "";
    }
}

QString PermissionPolicy::getAppTypeExplanation(AppType type)
{
    switch (type) {
        case SystemApp:
            return "Core operating system application";
        case UtilityApp:
            return "Simple utility application";
        case MultiMediaApp:
            return "Multimedia application (audio/video)";
        case ProductivityApp:
            return "Productivity application";
        case DeveloperApp:
            return "Development/programming tool";
        case GameApp:
            return "Game application";
        case UnknownApp:
            return "Unknown application type";
        default:
            return "Unknown";
    }
}

QString PermissionPolicy::recommendPermissionsForApp(const QString& appId, AppType appType)
{
    QList<PrivacyManager::PermissionCategory> defaults = getDefaultPermissionsForType(appType);

    QString recommendation = QString("For a %1, we recommend:\n").arg(getAppTypeExplanation(appType));

    if (defaults.isEmpty()) {
        recommendation += "No permissions required for normal operation";
    } else {
        recommendation += "Permissions:\n";
        for (auto perm : defaults) {
            recommendation += QString("  • %1\n").arg(PrivacyManager::permissionCategoryToString(perm));
        }
    }

    return recommendation;
}

QList<PermissionPolicy::PermissionRequirement> PermissionPolicy::getPermissionRequirements(
    const QString& appId)
{
    QList<PermissionRequirement> requirements;

    // Analyze app name for clues
    QString lowerName = appId.toLower();

    // These are examples; real app would query system
    if (lowerName.contains("browser") || lowerName.contains("firefox") ||
        lowerName.contains("chrome") || lowerName.contains("chromium")) {
        requirements.append({PrivacyManager::Network, "Browser needs internet access", true});
        requirements.append({PrivacyManager::Files, "Browser needs to save downloads", false});
    }

    if (lowerName.contains("media") || lowerName.contains("video") ||
        lowerName.contains("audio")) {
        requirements.append({PrivacyManager::Audio, "Media player needs audio output", true});
        requirements.append({PrivacyManager::Files, "Media player needs file access", true});
    }

    return requirements;
}

bool PermissionPolicy::enforcePolicy(const QString& appId,
                                    PrivacyManager::PermissionCategory category)
{
    // This would integrate with OS-level enforcement (AppArmor, SELinux, etc.)
    // For now, just validate
    QString validation = validatePermissionRequest(appId, category, "");
    return validation.isEmpty();
}

void PermissionPolicy::initializeDefaultPolicies()
{
    if (s_initialized) {
        return;
    }

    initializePermissionMetadata();
    initializePolicyRules();

    s_initialized = true;
    qDebug() << "Permission policies initialized";
}

void PermissionPolicy::initializePermissionMetadata()
{
    // Define metadata for each permission
    s_permissionMetadata[PrivacyManager::Microphone] = {
        PrivacyManager::Microphone,
        "Microphone",
        "Recording audio input",
        "Allows the app to record audio from your microphone",
        "⚠ App can record everything you say",
        true,
        95,
        {PermissionPolicy::MultiMediaApp}
    };

    s_permissionMetadata[PrivacyManager::Camera] = {
        PrivacyManager::Camera,
        "Camera",
        "Video input",
        "Allows the app to access your webcam",
        "⚠ App can see through your camera",
        true,
        95,
        {PermissionPolicy::MultiMediaApp}
    };

    s_permissionMetadata[PrivacyManager::Files] = {
        PrivacyManager::Files,
        "Files",
        "File system access",
        "Allows the app to read and write your files",
        "⚠ App can access all your documents and data",
        false,
        60,
        {PermissionPolicy::SystemApp, PermissionPolicy::DeveloperApp, PermissionPolicy::ProductivityApp}
    };

    s_permissionMetadata[PrivacyManager::Network] = {
        PrivacyManager::Network,
        "Network",
        "Internet connectivity",
        "Allows the app to connect to the internet",
        "",
        false,
        40,
        {PermissionPolicy::SystemApp, PermissionPolicy::DeveloperApp}
    };

    s_permissionMetadata[PrivacyManager::Location] = {
        PrivacyManager::Location,
        "Location",
        "GPS and location data",
        "Allows the app to access your location",
        "⚠ App can track where you are",
        true,
        90,
        {}
    };

    s_permissionMetadata[PrivacyManager::Clipboard] = {
        PrivacyManager::Clipboard,
        "Clipboard",
        "Copy/paste access",
        "Allows the app to read what you copy and paste",
        "⚠ App can see sensitive data you copy",
        false,
        50,
        {PermissionPolicy::ProductivityApp}
    };

    s_permissionMetadata[PrivacyManager::Contacts] = {
        PrivacyManager::Contacts,
        "Contacts",
        "Contact list access",
        "Allows the app to access your contacts",
        "⚠ App can see all your phone numbers and emails",
        true,
        85,
        {}
    };

    s_permissionMetadata[PrivacyManager::Calendar] = {
        PrivacyManager::Calendar,
        "Calendar",
        "Calendar access",
        "Allows the app to see your calendar events",
        "⚠ App can see your schedule and commitments",
        true,
        80,
        {}
    };

    s_permissionMetadata[PrivacyManager::Photos] = {
        PrivacyManager::Photos,
        "Photos",
        "Photo library access",
        "Allows the app to access your photos",
        "⚠ App can see all your images",
        true,
        85,
        {PermissionPolicy::MultiMediaApp}
    };

    s_permissionMetadata[PrivacyManager::Bluetooth] = {
        PrivacyManager::Bluetooth,
        "Bluetooth",
        "Bluetooth device access",
        "Allows the app to connect to Bluetooth devices",
        "",
        false,
        30,
        {PermissionPolicy::MultiMediaApp}
    };

    s_permissionMetadata[PrivacyManager::USB] = {
        PrivacyManager::USB,
        "USB",
        "USB device access",
        "Allows the app to access USB devices",
        "",
        false,
        40,
        {PermissionPolicy::DeveloperApp}
    };

    s_permissionMetadata[PrivacyManager::Printing] = {
        PrivacyManager::Printing,
        "Printing",
        "Printer access",
        "Allows the app to print documents",
        "",
        false,
        20,
        {PermissionPolicy::ProductivityApp}
    };

    s_permissionMetadata[PrivacyManager::Screenshots] = {
        PrivacyManager::Screenshots,
        "Screenshots",
        "Screen capture",
        "Allows the app to take screenshots",
        "⚠ App can capture sensitive information on screen",
        true,
        85,
        {}
    };

    s_permissionMetadata[PrivacyManager::SystemMonitor] = {
        PrivacyManager::SystemMonitor,
        "System Monitor",
        "Process and resource monitoring",
        "Allows the app to monitor system processes",
        "⚠ App can see all running programs and sensitive processes",
        true,
        75,
        {PermissionPolicy::DeveloperApp}
    };

    s_permissionMetadata[PrivacyManager::Audio] = {
        PrivacyManager::Audio,
        "Audio",
        "System audio access",
        "Allows the app to access system audio output",
        "",
        false,
        30,
        {PermissionPolicy::MultiMediaApp}
    };
}

void PermissionPolicy::initializePolicyRules()
{
    // Define default policy for each app type and permission combination
    s_policyRules.append({SystemApp, PrivacyManager::Files, PrivacyManager::AllowedAlways, false});
    s_policyRules.append({SystemApp, PrivacyManager::Network, PrivacyManager::AllowedAlways, false});
    s_policyRules.append({SystemApp, PrivacyManager::Microphone, PrivacyManager::AskEveryTime, true});
    s_policyRules.append({SystemApp, PrivacyManager::Camera, PrivacyManager::AskEveryTime, true});

    s_policyRules.append({MultiMediaApp, PrivacyManager::Microphone, PrivacyManager::AskEveryTime, true});
    s_policyRules.append({MultiMediaApp, PrivacyManager::Camera, PrivacyManager::AskEveryTime, true});
    s_policyRules.append({MultiMediaApp, PrivacyManager::Audio, PrivacyManager::AllowedAlways, false});

    s_policyRules.append({ProductivityApp, PrivacyManager::Files, PrivacyManager::AllowedAlways, false});
    s_policyRules.append({ProductivityApp, PrivacyManager::Clipboard, PrivacyManager::AllowedAlways, false});

    s_policyRules.append({DeveloperApp, PrivacyManager::Files, PrivacyManager::AllowedAlways, false});
    s_policyRules.append({DeveloperApp, PrivacyManager::Network, PrivacyManager::AllowedAlways, false});
    s_policyRules.append({DeveloperApp, PrivacyManager::SystemMonitor, PrivacyManager::AllowedAlways, false});

    s_policyRules.append({UtilityApp, PrivacyManager::Files, PrivacyManager::AllowedAlways, false});
    s_policyRules.append({UtilityApp, PrivacyManager::Microphone, PrivacyManager::Denied, false});
    s_policyRules.append({UtilityApp, PrivacyManager::Camera, PrivacyManager::Denied, false});
}

PermissionPolicy::PermissionMetadata PermissionPolicy::getPermissionMetadata(
    PrivacyManager::PermissionCategory category)
{
    if (s_permissionMetadata.contains(category)) {
        return s_permissionMetadata[category];
    }
    return PermissionMetadata();
}
