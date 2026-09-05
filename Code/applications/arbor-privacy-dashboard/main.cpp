#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include "privacy-dashboard-widget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("Arbor Privacy Dashboard");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("Privacy Dashboard");

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Arbor Privacy Dashboard");
    mainWindow.setGeometry(100, 100, 1200, 800);

    PrivacyDashboardWidget* dashboard = new PrivacyDashboardWidget(&mainWindow);
    mainWindow.setCentralWidget(dashboard);

    mainWindow.show();

    qDebug() << "Privacy Dashboard started";

    return app.exec();
}
