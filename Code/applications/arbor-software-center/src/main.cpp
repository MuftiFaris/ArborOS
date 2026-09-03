#include "app-store.h"
#include "../include/style-provider.h"
#include <QApplication>
#include <QStyleFactory>
#include <QLocale>

using namespace Arbor::SoftwareCenter;
using namespace Arbor::DesignSystem;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Initialize design system
    app.setStyle(QStyleFactory::create("Fusion"));
    StyleProvider::instance()->setDarkMode(true);
    StyleProvider::instance()->applyStyle(&app);
    
    // Create and show main window
    AppStore window;
    window.show();
    
    return app.exec();
}
