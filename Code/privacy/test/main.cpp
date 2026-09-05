#include "privacy-manager.h"
#include "permission-policy.h"
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== Privacy Framework Test ===\n";

    // Initialize PrivacyManager
    PrivacyManager* pm = PrivacyManager::instance();
    qDebug() << "✓ PrivacyManager initialized";

    // Register test app
    PrivacyManager::AppMetadata metadata;
    metadata.id = "test-app";
    metadata.name = "Test Application";
    metadata.execPath = "/usr/bin/test-app";
    pm->registerApp(metadata);
    qDebug() << "✓ Test app registered";

    // Test permission request
    qDebug() << "\nTesting permission request:";
    PrivacyManager::PermissionState currentState = pm->getPermission("test-app", PrivacyManager::Microphone);
    qDebug() << "  Current microphone state:" << currentState;

    // Set permission
    pm->setPermission("test-app", PrivacyManager::Microphone, PrivacyManager::AllowedAlways);
    qDebug() << "  Set to: AllowedAlways";

    // Verify
    currentState = pm->getPermission("test-app", PrivacyManager::Microphone);
    qDebug() << "  Verified state:" << currentState;

    // Test audit trail
    qDebug() << "\nTesting audit trail:";
    pm->logAccess("test-app", PrivacyManager::Microphone, "Testing microphone access");
    qDebug() << "  Logged access";

    QList<PrivacyManager::PermissionRecord> trail = pm->getAuditTrailForApp("test-app");
    qDebug() << "  Records in trail:" << trail.count();
    for (const auto& record : trail) {
        qDebug() << "    -" << record.appId << record.categoryName() << record.actionName();
    }

    // Test privacy score
    qDebug() << "\nTesting privacy score:";
    int score = pm->calculatePrivacyScore();
    qDebug() << "  Privacy score:" << score;

    // Test PermissionPolicy
    qDebug() << "\nTesting PermissionPolicy:";
    PermissionPolicy::AppType appType = PermissionPolicy::classifyApp("test-app", "Test Application");
    qDebug() << "  App type:" << appType;

    QString explanation = PermissionPolicy::getPermissionExplanation(PrivacyManager::Microphone);
    qDebug() << "  Microphone explanation:" << explanation;

    auto defaults = PermissionPolicy::getDefaultPermissionsForType(PermissionPolicy::MultiMediaApp);
    qDebug() << "  Default permissions for multimedia app:" << defaults.count();

    // Test policy evaluation
    auto decision = PermissionPolicy::evaluatePermissionRequest(
        "test-app",
        PrivacyManager::Camera,
        appType,
        "Testing camera access"
    );
    qDebug() << "  Camera policy decision:" << decision.allowed << decision.reason;

    qDebug() << "\n=== All Tests Completed ===\n";

    return 0;
}
