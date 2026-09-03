#include <QApplication>
#include "terminal-window.h"
#include "style-provider.h"
#include "color-palette.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Set application metadata
    app.setApplicationName("Arbor Terminal");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("Arbor Terminal");
    app.setOrganizationName("ArborOS");
    app.setOrganizationDomain("arbor-os.org");

    // Initialize design system
    Arbor::Design::StyleProvider::initialize();
    Arbor::Design::ColorPalette::setTheme(Arbor::Design::ColorPalette::Theme::Dark);
    Arbor::Design::StyleProvider::applyStyle();

    // Create and show main window
    TerminalWindow window;
    window.setWindowTitle("Arbor Terminal");
    window.setWindowIcon(QIcon(":/icons/arbor-terminal.svg"));
    window.resize(900, 600);
    window.show();

    return app.exec();
}
