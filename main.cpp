#include <qapplication.h>
#include <QtCore/qsettings.h>
#include <qdebug.h>

#include "../common/currencies/CurrencyRateManager.h"

#include "gui/DialogOpenConfig.h"

#include "model/SettingManager.h"

#include "gui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Icinger Power");
    QCoreApplication::setApplicationName("EcomCompta");
    #ifndef Q_OS_LINUX
    QSettings::setDefaultFormat(QSettings::IniFormat);
    #endif
    qInfo() << "Setting file located: " << QSettings().fileName();

    DialogOpenConfig dialogOpenConfig;
    dialogOpenConfig.exec();
    if (dialogOpenConfig.wasAccepted()) {
        QString workingDirPath = dialogOpenConfig.dirPath();
        SettingManager::instance()->setWorkingDirectory(workingDirPath);
        CurrencyRateManager::instance()->setWorkingDir(workingDirPath);
    } else {
        return 0;
    }
    MainWindow w;
    w.showMaximized();

    return a.exec();
}
