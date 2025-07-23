#include <QtCore/qsettings.h>
#include <QtCore/qset.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qsharedpointer.h>

#include "../common/countries/CountryManager.h"

#include "SettingManager.h"
#include "CustomerManager.h"

//----------------------------------------------------------
QString SettingManager::SEP_COL = ";%|;";
QString SettingManager::SEP_LINES = ":%|:";
QString SettingManager::SEP_TO_STRING_1 = ",%|,";
QString SettingManager::SEP_TO_STRING_2 = ",|%,";
QString SettingManager::SEP_TO_STRING_LIST = ",|L%,";
QString SettingManager::SEP_TO_STRING_MAP_KEY = ",|M%,";
QString SettingManager::DATE_FORMAT_ORDER = "yyyy-MM-dd";
QString SettingManager::DATE_TIME_FORMAT_ORDER = "yyyy-MM-dd_mm:hh:ss";
QString SettingManager::DATE_FORMAT_DISPLAY = QObject::tr("dd/MM/yyyy", "date format for display");
QString SettingManager::DATE_TIME_FORMAT_DISPLAY = QObject::tr("dd/MM/yyyy mm:hh:ss", "date time format for display");
//----------------------------------------------------------
SettingManager *SettingManager::instance()
{
    static SettingManager uniqueInstance(nullptr);
    return &uniqueInstance;
}
//----------------------------------------------------------
SettingManager::SettingManager(QObject *object) : QObject(object)
{
    QFileInfo settingFileInfo(QSettings().fileName());
    m_workingDir = settingFileInfo.absoluteDir();
}
//----------------------------------------------------------
const QStringList *SettingManager::countriesUE(int year)
{
    static QHash<int, QSharedPointer<QStringList>> countries
            = []() -> QHash<int, QSharedPointer<QStringList>> {
        QHash<int, QSharedPointer<QStringList>> countries;
        QStringList countriesAfterBrexit = {"FR","MC","DE","IT","ES","PL","CZ","NL","SE","BE","BG","DK","EE","GR","IE","EL","HR","CY","LV","LT","LU","HU","MT","AT","PT","RO","SI","SK","FI"};
        QStringList countriesBeforeBrexit = countriesAfterBrexit;
        countriesBeforeBrexit.append("GB");
        countriesBeforeBrexit.append("UK");
        countriesAfterBrexit.append("GB-NIR");
        QSharedPointer<QStringList> pCountriesBeforeBrexit(new QStringList(countriesBeforeBrexit));
        for (int year = 2000; year <= 2020; ++year) {
            countries[year] = pCountriesBeforeBrexit;
        }
        QSharedPointer<QStringList> pCountriesAfterBrexit(new QStringList(countriesAfterBrexit));
        for (int year = 2021; year <= 2050; ++year) {
            countries[year] = pCountriesAfterBrexit;
        }
        return countries;
    }();
    return countries[year].data();
}
//----------------------------------------------------------
const QStringList *SettingManager::countriesUEfrom2020()
{
    static QStringList countries
            = []() -> QStringList {
        QSet<QString> countries;
        for (int year = 2020; year <= QDate::currentDate().year(); ++year) {
            for (auto country : *countriesUE(year)) {
                countries << country;
            }
        }
        return countries.toList();
    }();
    return &countries;
}
//----------------------------------------------------------
const QStringList *SettingManager::months()
{
    static QStringList months
            = {tr("Janvier"), tr("Févier"), tr("Mars"),
              tr("Avril"), tr("Mai"), tr("Juin"),
              tr("Juillet"), tr("Août"), tr("Septembre"),
              tr("Octobre"), tr("Novembre"), tr("Décembre")};
    return &months;
}

QString SettingManager::formatVatRate(double vatRate)
{
    QString vatRateString = QString::number(
                vatRate,
                'f',
                4);
    for (int i=0; i<2; ++i)
    {
        if (vatRateString.endsWith("0"))
        {
            vatRateString.remove(vatRateString.size()-1, 1);
        }
    }
    return vatRateString;
}
//----------------------------------------------------------
QStringList SettingManager::currencies() const
{
    static QStringList values
            = {"EUR", "USD", "GBP", "CAD", "JPN", "MXN", "SEK", "PLN"}; // TODO add sames value as UI, then use this in UI
    return values;
}
//----------------------------------------------------------
/*
QString SettingManager::currency() const
{
    return "EUR"; //TODO After translation, this could change
}
//----------------------------------------------------------
QString SettingManager::companyCountryName() const
{
    return CountryManager::FRANCE; //TODO After translation, this could change
}
//----------------------------------------------------------
QString SettingManager::companyCountryCode() const
{
    return "FR";
}
//*/
/*
//----------------------------------------------------------
QString SettingManager::countryCodeDomTom(
        const QString &countryCode, const QString &postalCode) const
{
    if (countryCode == "FR") {
        if (postalCode.startsWith("971")) {
            return "GP";
        } else if (postalCode.startsWith("972")) {
            return "MQ";
        } else if (postalCode.startsWith("973")) {
            return "GF";
        } else if (postalCode.startsWith("974")) {
            return "RE";
        } else if (postalCode.startsWith("976")) {
            return "YT";
        }
    }
    return countryCode;
}
//----------------------------------------------------------
QString SettingManager::countryName(
        const QString &countryCode) const
{
    static QHash<QString, QString> codeToName
            = {{"FR", tr("France")},
                {"GB", tr("Grande-bretagne")},
                {"DE", tr("Allemagne")},
                {"IT", tr("Italie")},
                {"ES", tr("Espagne")},
                {"PL", tr("Pologne")},
                {"CZ", tr("République Tchèque")},
                {"NL", tr("Hollande")},
                {"SE", tr("Suède")},
                {"BE", tr("Belgique")},
                {"BG", tr("Bulgarie")},
                {"DK", tr("Danemark")},
                {"EE", tr("Estonie")},
                {"IE", tr("Irelande")},
                {"EL", tr("Grèce")},
                {"HR", tr("Croatie")},
                {"CY", tr("Chypre")},
                {"LV", tr("Lettonie")},
                {"LT", tr("Lituanie")},
                {"GR", tr("Grèce")},
                {"LU", tr("Luxembourg")},
                {"HU", tr("Hongrie")},
                {"MT", tr("Malte")},
                {"AT", tr("Autriche")},
                {"PT", tr("Portugal")},
                {"RO", tr("Roumanie")},
                {"SI", tr("Slovénie")},
                {"SK", tr("Slovaquie")},
                {"FI", tr("Finlande")},
                {"GP", tr("Guadeloupe")},
                {"UK", tr("Royaume-uni")},
                {"MC", tr("Monaco")},
                {"IS", tr("Icelande")},
                {"CA", tr("Canada")},
                {"AE", tr("Émirats arabes unis")},
                {"AU", tr("Australie")},
                {"US", tr("États-Unis")},
                {"RE", tr("Réunion")},
                {"MQ", tr("Martinique")},
                {"GF", tr("Guyane")},
                {"GP", tr("Guadeloupe")},
                {"YT", tr("Mayotte")},
                {"CH", tr("Suisse")},
                {"", ""}
               };
    //Q_ASSERT(!countryCode.isEmpty());
    //Q_ASSERT(codeToName.contains(countryCode.toUpper()));
    return codeToName[countryCode.toUpper()];
}
//----------------------------------------------------------
QString SettingManager::countryCode(const QString &countryName) const
{
    static QHash<QString, QString> codeToName = []() -> QHash<QString, QString>{
        QHash<QString, QString> nameToCode;
        for (auto code : *countriesUEfrom2020()) {
            nameToCode[code] = code;
            nameToCode[SettingManager::instance()->countryName(code)] = code;
        }
        return nameToCode;
    }();
    return codeToName[countryName];
}
//*/
//----------------------------------------------------------
QBrush SettingManager::colorOange() const
{
    return QColor("#ffccaa");
}
//----------------------------------------------------------
QBrush SettingManager::brushOrange() const
{
    return QBrush(colorOange());
}
//----------------------------------------------------------
QBrush SettingManager::colorRed() const
{
    return QColor("#ffaaaa");
}
//----------------------------------------------------------
QBrush SettingManager::brushRed() const
{
    return QBrush(colorRed());
}
//----------------------------------------------------------
QBrush SettingManager::colorGreen() const
{
    return QColor("#d5ffe6");
    //return QColor("#d7f4ee");
}
//----------------------------------------------------------
QBrush SettingManager::brushGreen() const
{
    return QBrush(colorGreen());
}
//----------------------------------------------------------
QBrush SettingManager::colorLightBlue() const
{
    return QColor("#d5e5ff");
}
//----------------------------------------------------------
QBrush SettingManager::brushLightBlue() const
{
    return QBrush(colorLightBlue());
}
//----------------------------------------------------------
QBrush SettingManager::colorBlue() const
{
    return QColor("#afc6e9");
}
//----------------------------------------------------------
QBrush SettingManager::brushBlue() const
{
    return QBrush(colorBlue());
}
//----------------------------------------------------------
QBrush SettingManager::colorTurquoise() const
{
    return QColor("#d7f4ee");
}
//----------------------------------------------------------
QBrush SettingManager::brushTurquoise() const
{
    return QBrush(colorTurquoise());
}
//----------------------------------------------------------
QBrush SettingManager::colorPurple() const
{
    return QColor("#e5d5ff");
}
//----------------------------------------------------------
QBrush SettingManager::brushPurple() const
{
    return QBrush(colorPurple());
}
//----------------------------------------------------------
QString SettingManager::returnLine() const
{
    return "\r\n"; //TODO find return line by system
}
//----------------------------------------------------------
QDir SettingManager::dirInventory() const
{
    QString customerId = CustomerManager::instance()->getSelectedCustomerId();
    QDir dir = workingDirectory(customerId);
    QString dirName = "inventory";
    dir.mkpath(dirName);
    dir.cd(dirName);
    return dir;
}
//----------------------------------------------------------
QDir SettingManager::dirInventory(int year) const
{
    QDir dir = dirInventory();
    QString dirName = QString::number(year);
    dir.mkpath(dirName);
    dir.cd(dirName);
    return dir;
}
//----------------------------------------------------------
QDir SettingManager::dirInventoryMergedCodes() const
{
    QDir dir = dirInventory();
    QString dirName = "merged-codes";
    dir.mkpath(dirName);
    dir.cd(dirName);
    return dir;
}
//----------------------------------------------------------
QDir SettingManager::dirInventoryBundles() const
{
    QDir dir = dirInventory();
    QString dirName = "bundles";
    dir.mkpath(dirName);
    dir.cd(dirName);
    return dir;
}
//----------------------------------------------------------
QDir SettingManager::dirInventoryBegin(int year) const
{
    QDir dir = dirInventory(year);
    QString dirName = QObject::tr("debut", "begining of the year (no space please in the translation)");
    dir.mkpath(dirName);
    dir.cd(dirName);
    return dir;
}
//----------------------------------------------------------
QDir SettingManager::dirInventoryPurchase(int year) const
{
    QDir dir = dirInventory(year);
    QString dirName = QObject::tr("achats");
    dir.mkpath(dirName);
    dir.cd(dirName);
    return dir;
}
//----------------------------------------------------------
QDir SettingManager::dirInventoryAmzReturns(int year) const // amazon-storage-inventory
{
    QDir dir = dirInventory(year);
    QString dirName = QObject::tr("retours-amazon", "amazon-returns (no space please in the translation)");
    dir.mkpath(dirName);
    dir.cd(dirName);
    return dir;
}
//----------------------------------------------------------
QString SettingManager::settingsFilePath() const
{
    return m_workingDir.filePath("EcomCompta.conf");
}
//----------------------------------------------------------
void SettingManager::setWorkingDirectory(const QString dirPath)
{
    m_workingDir.setPath(dirPath);
}
//----------------------------------------------------------
QDir SettingManager::workingDirectory() const
{
    return m_workingDir;
    /*
    QFileInfo settingFileInfo(QSettings().fileName());
    QDir dir = settingFileInfo.absoluteDir();
    return dir;
    //*/
}
//----------------------------------------------------------
QDir SettingManager::workingDirectory(const QString &customerId) const
{
    QDir dir = workingDirectory();
    dir.mkpath(customerId);
    dir.cd(customerId);
    return dir;
}
//----------------------------------------------------------
QDir SettingManager::bookKeepingDir() const
{
    QString customerId = CustomerManager::instance()->getSelectedCustomerId();
    QDir dir = workingDirectory(customerId);
    QString dirName1 = "bookkeeping";
    dir.mkpath(dirName1);
    dir.cd(dirName1);
    return dir;
}
//----------------------------------------------------------
QDir SettingManager::bookKeepingDirBank() const
{
    QDir dirBank = bookKeepingDir();
    QString dirNameBank = "banks";
    dirBank.mkpath(dirNameBank);
    dirBank.cd(dirNameBank);
    return dirBank;
}
//----------------------------------------------------------
QDir SettingManager::bookKeepingDirBank(
        const QString &bankName) const
{
    QDir dir = bookKeepingDirBank();
    dir.mkpath(bankName);
    dir.cd(bankName);
    return dir;
}
//----------------------------------------------------------
QDir SettingManager::bookKeepingDirBank(
        const QString &bankName, int year) const
{
    QDir dir = bookKeepingDirBank(bankName);
    QString dirName = QString::number(year);
    dir.mkpath(dirName);
    dir.cd(dirName);
    return dir;
}
//----------------------------------------------------------
QDir SettingManager::bookKeepingDirPurchase() const
{
    QDir purchaseDir = bookKeepingDir();
    QString dirName2 = "invoices";
    purchaseDir.mkpath(dirName2);
    purchaseDir.cd(dirName2);
    return purchaseDir;
}
//----------------------------------------------------------
QDir SettingManager::bookKeepingDirPurchase(int year) const
{
    QDir dir = bookKeepingDirPurchase();
    QString yearStr = QString::number(year);
    dir.mkpath(yearStr);
    dir.cd(yearStr);
    return dir;
}
//----------------------------------------------------------
QDir SettingManager::bookKeepingDirPurchase(int year, int month) const
{
    QDir dir = bookKeepingDirPurchase(year);
    QString monthStr = QString::number(month).rightJustified(2, '0');
    dir.mkpath(monthStr);
    dir.cd(monthStr);
    return dir;
}
//----------------------------------------------------------
QDir SettingManager::bookKeepingDirPurchaseImport() const
{
    QDir purchaseDir = bookKeepingDir();
    QString dirName2 = "invoices-import";
    purchaseDir.mkpath(dirName2);
    purchaseDir.cd(dirName2);
    return purchaseDir;
}
//----------------------------------------------------------
QDir SettingManager::bookKeepingDirPurchaseImport(
        int year) const
{
    QDir dir = bookKeepingDirPurchaseImport();
    QString yearStr = QString::number(year);
    dir.mkpath(yearStr);
    dir.cd(yearStr);
    return dir;
}
//----------------------------------------------------------
QDir SettingManager::bookKeepingDirPurchaseImport(
        int year, int month) const
{
    QDir dir = bookKeepingDirPurchaseImport(year);
    QString monthStr = QString::number(month).rightJustified(2, '0');
    dir.mkpath(monthStr);
    dir.cd(monthStr);
    return dir;
}
//----------------------------------------------------------
QDir SettingManager::reportDirectory() const
{
    QString customerId = CustomerManager::instance()->getSelectedCustomerId();
    QDir reportDir = workingDirectory(customerId);
    QString reportDirName = "reports";
    reportDir.mkpath(reportDirName);
    reportDir.cd(reportDirName);
    return reportDir;
}
//----------------------------------------------------------
QDir SettingManager::reportDirectory(
        const QString &importerName, const QString &reportShortName) const
{
    QDir reportDir = reportDirectory();
    QString reportPathRel = importerName + QDir::separator() + reportShortName;
    reportDir.mkpath(reportPathRel);
    reportDir.cd(reportPathRel);
    return reportDir;
}
//----------------------------------------------------------
QDir SettingManager::reportDirectory(
        const QString &importerName,
        const QString &reportShortName,
        const QString &yearDir) const
{
    QDir reportDir = reportDirectory(importerName, reportShortName);
    reportDir.mkpath(yearDir);
    reportDir.cd(yearDir);
    return reportDir;
}
//----------------------------------------------------------
void SettingManager::exportSettings(
        const QString &dirPath, const QString &fileName) const
{
    /// TODO compress directory
}
//----------------------------------------------------------
void SettingManager::loadSettings(const QString &fileName)
{
    /// TODO uncompress fileName
}
/*
//----------------------------------------------------------
void SettingManager::registerDirectory(const QDir &dir)
{
}
//*/
//----------------------------------------------------------
