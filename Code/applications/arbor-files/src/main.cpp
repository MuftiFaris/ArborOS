#include <QApplication>
#include "file-manager.h"
#include "style-provider.h"
#include "color-palette.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Set application metadata
    app.setApplicationName("Arbor Files");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("Arbor Files");
    app.setOrganizationName("ArborOS");
    app.setOrganizationDomain("arbor-os.org");

    // Initialize design system
    Arbor::Design::StyleProvider::initialize();
    Arbor::Design::ColorPalette::setTheme(Arbor::Design::ColorPalette::Theme::Dark);
    Arbor::Design::StyleProvider::applyStyle();

    // Create and show main window
    FileManager fileManager;
    fileManager.setWindowTitle("Arbor Files");
    fileManager.setWindowIcon(QIcon(":/icons/arbor-files.svg"));
    fileManager.resize(900, 600);
    fileManager.show();

    return app.exec();
}
