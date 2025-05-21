#ifndef ORDER_H
#define ORDER_H

#include <QtCore/qdatetime.h>
#include <QtCore/qstring.h>
#include <QtCore/qhash.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qlist.h>

#include "Shipment.h"
#include "Refund.h"

class Order
{
public:
    Order(const QDateTime &dateTime,
          const QString &id,
          const QString &currency,
          const QString &channel,
          const QString &subchannel, /// Same as channel if not sub channel like amazon / amazon.fr
          const Address &addressTo,
          const Address &addressBillingTo,
          const Shipping &shippingExtra,
          bool isFromMarketplace,
          const QHash<QString, QSharedPointer<Shipment>> &shipments);
    void merge(const Order &order);

    QSharedPointer<ArticleSold> article(const QString &sku) const;
    QMultiHash<QString, QSharedPointer<ArticleSold> > articlesSold() const;
    QHash<QString, QSharedPointer<ArticleSold> > articlesSoldUnited() const;
    double getVatForRoundCorrection() const;
    double getTotalPriceTaxed() const;
    double getTotalPriceTaxes() const;
    double getTotalPriceUntaxed() const;
    QMap<QString, double> getChargedFees() const;
    double getTotalPriceTaxedConverted() const;
    double getTotalPriceTaxesConverted() const;
    double getTotalPriceUntaxedConverted() const;
    //QMap<QString, double> getChargedFeesConverted() const;
    //QList<QSharedPointer<ArticleSold>> getArticles() const;
    int getArticleUnitCount() const;
    //void addReportFrom(const QString &reportName);
    QSet<QString> reportsFrom() const;
    QList<QStringList> reportCombinationMissing() const;

    QString getLangCode() const;
    QDateTime getDateTime() const;
    void setDateTime(const QDateTime &dateTime);

    QString getId() const;
    void setId(const QString &id);

    QSet<int> shipmentYears() const;
    void addShipment(QSharedPointer<Shipment> shipment); ///Add in shipment order to avoid order id issue
    QHash<QString, QSharedPointer<Shipment> > getShipments() const;
    QSharedPointer<Shipment> getShipmentFirst() const;
    QSharedPointer<Shipment> getShipment(const QString &id) const;
    void setShipments(const QHash<QString, QSharedPointer<Shipment> > &shipments);
    int getShipmentCount() const;
    bool isShippedNextYear() const;
    bool containsCountry(const QString &countryCode) const;
    bool containsSku(const QString &sku) const;
    QStringList paymentIds() const;

    QString getChannel() const;
    void setChannel(const QString &channel);

    Shipping getShipping() const;
    void setShipping(const Shipping &shipping);

    Address getAddressTo() const;
    void setAddressTo(const Address &addressTo);

    QString getSubchannel() const;
    void setSubchannel(const QString &subchannel);

    Address getAddressBillingTo() const;
    void setAddressBillingTo(
            const Address &addressBillingTo);

    bool getVatToRecompute() const;
    void setVatToRecompute(bool vatToRecompute);

    bool isBusinessCustomer() const;
    void setIsBusinessCustomer(bool isBusinessCustomer);

    QString getVatNumber() const;
    void setVatNumber(const QString &vatNumber);

    QString getCompanyName() const;
    void setCompanyName(const QString &companyName);

    bool getShippedBySeller() const;
    void setShippedBySeller(bool shippedBySeller);

    QString getCurrency() const;
    void setCurrency(const QString &currency);

    bool isVatNumberValid() const;
    void setIsVatNumberValid(bool isVatNumberValid);

    bool isFromMarketplace() const;
    void setIsFromMarketplace(bool newIsFromMarketplace);

private:
    QDateTime m_dateTime;
    QString m_id;
    bool m_shippedBySeller;
    QString m_currency;
    QString m_channel;
    QString m_subchannel;
    QString m_companyName;
    Address m_addressTo;
    Address m_addressBillingTo;
    Shipping m_shippingExtra;
    bool m_vatToRecompute;
    bool m_isBusinessCustomer;
    bool m_isVatNumberValid;
    bool m_isFromMarketplace;
    QString m_vatNumber;
    QHash<QString, QSharedPointer<Shipment>> m_shipments;
    //QSet<QString> m_reportsFrom;
};

#endif // ORDER_H
