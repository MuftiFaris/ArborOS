#include "monitor-window.h"
#include "../include/style-provider.h"
#include <QApplication>
#include <QStyleFactory>

using namespace Arbor::SystemMonitor;
using namespace Arbor::DesignSystem;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Initialize design system
    app.setStyle(QStyleFactory::create("Fusion"));
    StyleProvider::instance()->setDarkMode(true);
    StyleProvider::instance()->applyStyle(&app);
    
    // Create and show main window
    MonitorWindow window;
    window.show();
    
    return app.exec();
}
