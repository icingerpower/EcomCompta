#include <qapplication.h>
#include <QtCore/qsettings.h>
#include <QMetaType>
#include <qdebug.h>

#include "../common/currencies/CurrencyRateManager.h"
//#include "../common/types/types.h"

#include "gui/DialogOpenConfig.h"

#include "model/SettingManager.h"

#include "gui/MainWindow.h"

/*
QDataStream & operator << (
        QDataStream &stream, const QList<QList<QVariant>> &listOfVariantList)
{
    int nLines = listOfVariantList.size();
    int nColumns = 0;
    if (nLines> 0) {
        nColumns = listOfVariantList[0].size();
    }
    stream << QString::number(nLines) + "-" + QString::number(nColumns);
    for (auto itLine=listOfVariantList.begin(); itLine!=listOfVariantList.end(); ++itLine) {
        for (auto itElement = itLine->begin(); itElement != itLine->end(); ++itElement) {
            stream << *itElement;
        }
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream, QList<QList<QVariant>> &listOfVariantList)
{
    QString sizeString;
    stream >> sizeString;
    QStringList sizeInfo = sizeString.split("-");
    int nLines = sizeInfo[0].toInt();
    int nColumns = sizeInfo[1].toInt();
    for (int i=0; i<nLines; ++i) {
        QList<QVariant> variantList;
        for (int j=0; j<nColumns; ++j) {
            QVariant value;
            stream >> value;
            variantList << value;
        }
        listOfVariantList << variantList;
    }
    return stream;
}
Q_DECLARE_METATYPE(QList<QList<QVariant>>);
//*/

int main(int argc, char *argv[])
{
    //qRegisterMetaType<QList<QList<QVariant>>>(); // Can’t work because it leds to conflicts with similar types saved
    //qRegisterMetaTypeStreamOperators<QList<QList<QVariant>>>("QList<QList<QVariant>>");
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
