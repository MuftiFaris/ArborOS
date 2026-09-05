#include <QApplication>
#include "privacy-dashboard.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    PrivacyDashboard dashboard;
    dashboard.show();
    
    return app.exec();
}
