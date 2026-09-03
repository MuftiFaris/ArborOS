#include <QApplication>
#include "settings-window.h"
#include "style-provider.h"
#include "color-palette.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Set application metadata
    app.setApplicationName("Arbor Settings");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("Arbor Settings");
    app.setOrganizationName("ArborOS");
    app.setOrganizationDomain("arbor-os.org");

    // Initialize design system
    Arbor::Design::StyleProvider::initialize();
    Arbor::Design::ColorPalette::setTheme(Arbor::Design::ColorPalette::Theme::Dark);
    Arbor::Design::StyleProvider::applyStyle();

    // Create and show main window
    SettingsWindow window;
    window.setWindowTitle("Arbor Settings");
    window.setWindowIcon(QIcon(":/icons/arbor-settings.svg"));
    window.resize(900, 700);
    window.show();

    return app.exec();
}
