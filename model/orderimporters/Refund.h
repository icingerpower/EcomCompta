#ifndef REFUND_H
#define REFUND_H

#include <QtCore/qdatetime.h>
#include <QtCore/qstring.h>

#include "ArticleSold.h"
#include "Shipping.h"
#include "Shipment.h"

class Refund : public Shipment {
public:
    Refund(const QString &id,
           const QString &orderId,
           const QHash<QString, QSharedPointer<ArticleSold>> &articlesShipped,
           const Shipping &shipping,
           const QDateTime &dateTime,
           const Address &addressTo,
           const QString &currency,
           double vatForRoundCorrection);
    void init(Order *order) override;
    void reversePrices();
    QSharedPointer<Shipment> getFirstShipment(
            const QString &vatRegime, const QString &vatCountry) const;
    QList<QSharedPointer<Shipment> > getShipments() const;
    void merge(const Refund &refund);
    void setAddressTo(const Address &addressTo);
    void computeVatRegime(double &totalSaleCountryOss, bool alsoRecomputeVat = false) override;
    void computeVatRegime(bool alsoRecomputeVat = false);
    QString channel() const override;
    void setChannel(const QString &channel);
    QString subchannel() const override;
    void setSubchannel(const QString &subchannel);
    bool isCompletelyLoaded() const override;
    /*
    bool hasSeveralVat() const;
    QHash<QString, QHash<QString, QMap<QString, double>>> getTotalPriceTaxedByRegime() const;
    QHash<QString, QHash<QString, QMap<QString, double>>> getTotalPriceTaxesByRegime() const;
    QHash<QString, QHash<QString, QMap<QString, double>>> getTotalPriceUntaxedByRegime() const;
    //*/

    QString orderId() const override;
    static Refund *fromString(const QString &string);
    QString toString() const;


    bool wasGuessed() const;
    void setWasGuessed(bool newWasGuessed);

    bool retrieveAllShipmentsOnInit() const;
    void setRetrieveAllShipmentsOnInit(bool newRetrieveAllShipmentsOnInit);

private:
    ///   Regime        countryvat     vatrate
    //QHash<QString, QHash<QString, QMap<QString, double>>> m_pricesTaxedByRegime;
    //QHash<QString, QHash<QString, QMap<QString, double>>> m_pricesTaxesByRegime;
    QList<QSharedPointer<Shipment>> m_shipments;
    QMultiHash<QString, QSharedPointer<ArticleSold>> m_skuToArticleFromShipments;
    void _retrieveShipmentsFromOrder();
    //QSet<QString> m_vatRegimes;
    //QSet<QString> m_vatCountries;
    QString m_channel;
    QString m_subchannel;
    QString m_orderId;
    Address m_addressTo;
    Address addressTo() const;
    Address addressFrom() const;
    bool m_wasGuessed;
    bool m_retrieveAllShipmentsOnInit;

};
#endif // REFUND_H

