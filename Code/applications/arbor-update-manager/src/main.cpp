#include "update-window.h"
#include "../include/style-provider.h"
#include <QApplication>
#include <QStyleFactory>

using namespace Arbor::UpdateManager;
using namespace Arbor::DesignSystem;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Initialize design system
    app.setStyle(QStyleFactory::create("Fusion"));
    StyleProvider::instance()->setDarkMode(true);
    StyleProvider::instance()->applyStyle(&app);
    
    // Create and show main window
    UpdateWindow window;
    window.show();
    
    return app.exec();
}
