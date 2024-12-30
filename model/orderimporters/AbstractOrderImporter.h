#ifndef ABSTRACTORDERIMPORTER_H
#define ABSTRACTORDERIMPORTER_H

#include <QtCore/qstring.h>
#include <QtCore/qexception.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qmap.h>
#include <QtCore/qhash.h>
#include <QtCore/qabstractitemmodel.h>

#include "OrderMapping.h"
#include "model/UpdateToCustomer.h"

struct ReportType {
    QStringList extensions;
    QString shortName;
    //QString name;
    QString helpText;
};
class OrderImporterException : public QException
{
public:
    void raise() const override;
    OrderImporterException *clone() const override;

    QString error() const;
    void setError(const QString &error);

private:
    QString m_error;
};


class AbstractOrderImporter : public UpdateToCustomer
{
public:
    AbstractOrderImporter();
    void onCustomerSelectedChanged(const QString &customerId) override;
    virtual ~AbstractOrderImporter() override;
    static QList<AbstractOrderImporter *>
        allImporters(); /// To update each time adding an importer
    static AbstractOrderImporter *
        importer(const QString &name); /// To update each time adding an importer
    QMultiHash<QString, QStringList> reportForOrderCompleteMap(const Order *order) const; // For amazon, vat report, payment report, transactions report…

    virtual QString name() const = 0;
    virtual QString invoicePrefix() const = 0;
    virtual QList<ReportType> reportTypes() const = 0; // For amazon, vat report, payment report, transactions report…
    virtual QList<QStringList> reportForOrderComplete(
            const Order *order) const = 0; // For amazon, vat report, payment report, transactions report…
    QString nameForSetting() const;

    bool isVatToRecompute() const;
    void setVatToRecompute(bool value);
    bool countryRequireVatDueThreshold(const QString &country) const;
    void setDefaultShippingAddress(const Address &address);
    QString getDefaultShippingAddressId() const;
    //QString getDefaultShippingCountryCode() const;
    //void setDefaultShippingCountryCode(const QString &contryCode);

     /// Return error or nothing if valid
    //virtual QString isReportValid(
            //const QString &reportTypeName,
            //const QString &fileName) const = 0;

    /// Previous function isReportValid was called already
    virtual QSharedPointer<OrdersMapping> loadReport(
            const QString &reportTypeName,
            const QString &fileName,
            int maxYear) const = 0; /// Can raise CsvHeaderExeption
    void updateOrderIfMicroCountry(Order *order) const;
    /*
    virtual QHash<int, QMap<QDateTime, int>> loadNumberOrders(
            const QString &reportTypeName,
            const QString &fileName) const = 0; /// Can raise CsvHeaderExeption
            //*/
private:
    QString _settingKeyVatRecompute() const;
    QString _settingKeyDefaultShippingAddress() const;
};

#endif // ABSTRACTORDERIMPORTER_H
