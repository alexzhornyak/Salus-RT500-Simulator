#include "mainwindow.h"
#include <QApplication>

#ifdef _SCXML_EXTERN_MONITOR_
    #include "scxmlexternmonitor.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef _SCXML_EXTERN_MONITOR_
    g_ScxmlStateMachineName = "SalusRT500Logic";
    qInstallMessageHandler(myMessageOutput);
    QLoggingCategory::setFilterRules("qt.scxml.statemachine=true");
#endif

    MainWindow w;
    w.show();

    return a.exec();
}
