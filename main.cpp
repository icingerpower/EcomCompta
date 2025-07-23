#include <qapplication.h>
#include <QtCore/qsettings.h>
#include <QMetaType>
#include <QDoubleSpinBox>
#include <QItemEditorFactory>
#include <QItemEditorCreator>
#include <qdebug.h>

#include "../common/currencies/CurrencyRateManager.h"

#include "gui/DialogOpenConfig.h"

#include "model/SettingManager.h"

#include "gui/MainWindow.h"

class DoubleSpinBox3Digits : public QDoubleSpinBox {
public:
    explicit DoubleSpinBox3Digits(QWidget *p=nullptr) : QDoubleSpinBox(p) {
        setDecimals(3);
    }
    ~DoubleSpinBox3Digits(){}
};

int main(int argc, char *argv[])
{
    //qRegisterMetaType<QList<QList<QVariant>>>(); // Can’t work because it leds to conflicts with similar types saved
    //qRegisterMetaTypeStreamOperators<QList<QList<QVariant>>>("QList<QList<QVariant>>");
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Icinger Power");
    QCoreApplication::setApplicationName("EcomCompta");

    auto *fac = new QItemEditorFactory;
    fac->registerEditor(QVariant::Double,
                        new QItemEditorCreator<DoubleSpinBox3Digits>{"DoubleSpinBox3Digits"});

    QItemEditorFactory::setDefaultFactory(fac);
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
