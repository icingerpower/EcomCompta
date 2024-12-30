#ifndef SETTINGBOOKKEEPING_H
#define SETTINGBOOKKEEPING_H

#include "model/UpdateToCustomer.h"

class SettingBookKeeping : public UpdateToCustomer
{
public:
    static SettingBookKeeping *instance();
    ~SettingBookKeeping() override;
    void onCustomerSelectedChanged(
            const QString &customerId) override;
    QString uniqueId() const override;
    QString saverName() const;
    void setSaverName(const QString &saver);
    QString dirPath() const;
    void setDirPath(const QString &dirPath);
    QString internWireAccount() const;

private:
    SettingBookKeeping();
    QString settingKeySaver() const;
    QString settingKeyBookKeepingDir() const;
};

#endif // SETTINGBOOKKEEPING_H
