#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QtCore/qstring.h>
#include <QtCore/qdir.h>
#include <QtGui/qbrush.h>
#include <QtGui/qcolor.h>
#include <QtCore/qobject.h>

class SettingManager : public QObject
{
    Q_OBJECT
public:
    static QString SEP_COL;
    static QString SEP_LINES;
    static QString SEP_TO_STRING_1;
    static QString SEP_TO_STRING_2;
    static QString SEP_TO_STRING_LIST;
    static QString SEP_TO_STRING_MAP_KEY;
    static QString DATE_FORMAT_ORDER;
    static QString DATE_TIME_FORMAT_ORDER;
    static QString DATE_FORMAT_DISPLAY;
    static QString DATE_TIME_FORMAT_DISPLAY;
    static SettingManager *instance();
    static const QStringList *countriesUE(int year);
    static const QStringList *countriesUEfrom2020();
    static const QStringList *months();
    /*
    QString currency() const;
    QString companyCountryName() const;
    QString companyCountryCode() const;
    //*/

    QStringList currencies() const;
    /*
    QString countryCodeDomTom(const QString &countryCode, const QString &postalCode) const;
    QString countryName(const QString &countryCode) const;
    QString countryCode(const QString &countryName) const;
    //*/
    QBrush colorOange() const;
    QBrush brushOrange() const;
    QBrush colorRed() const;
    QBrush brushRed() const;
    QBrush colorGreen() const;
    QBrush brushGreen() const;
    QBrush colorLightBlue() const;
    QBrush brushLightBlue() const;
    QBrush colorBlue() const;
    QBrush brushBlue() const;
    QBrush colorTurquoise() const;
    QBrush brushTurquoise() const;
    QBrush colorPurple() const;
    QBrush brushPurple() const;
    QString returnLine() const;
    QDir dirInventory() const;
    QDir dirInventory(int year) const;
    QDir dirInventoryMergedCodes() const;
    QDir dirInventoryBundles() const;
    QDir dirInventoryBegin(int year) const;
    QDir dirInventoryPurchase(int year) const;
    QDir dirInventoryAmzReturns(int year) const;
    QString settingsFilePath() const;
    void setWorkingDirectory(const QString dirPath);
    QDir workingDirectory() const;
    QDir workingDirectory(const QString &customerId) const;
    QDir bookKeepingDir() const;
    QDir bookKeepingDirBank() const;
    QDir bookKeepingDirBank(const QString &bankName) const;
    QDir bookKeepingDirBank(const QString &bankName, int year) const;
    QDir bookKeepingDirPurchase() const;
    QDir bookKeepingDirPurchase(int year) const;
    QDir bookKeepingDirPurchase(int year, int month) const;
    QDir bookKeepingDirPurchaseImport() const;
    QDir bookKeepingDirPurchaseImport(int year) const;
    QDir bookKeepingDirPurchaseImport(int year, int month) const;
    QDir reportDirectory() const;
    QDir reportDirectory( ///TODO customer ID
            const QString &importerName, const QString &reportShortName) const;
    QDir reportDirectory(
            const QString &importerName, const QString &reportShortName, const QString &yearDir) const;
    //void setWorkingDirectory(const QDir &dir) const;
    void exportSettings(const QString &dirPath, const QString &fileName) const;
    void loadSettings(const QString &fileName);
    //void registerDirectory(const QDir &dir);

signals:
    void yearChanged(int year);

private:
    SettingManager(QObject *object);
    QDir m_workingDir;
};

#endif // SETTINGMANAGER_H
