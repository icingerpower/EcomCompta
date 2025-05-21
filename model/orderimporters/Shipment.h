#ifndef SHIPMENT_H
#define SHIPMENT_H

#include <QtCore/qdatetime.h>
#include <QtCore/qstring.h>
#include <QtCore/qset.h>
#include <QtCore/qhash.h>
#include <QtCore/qsharedpointer.h>

#include "Shipping.h"
#include "ArticleSold.h"
#include "Address.h"
#include <QtCore/qmap.h>

// TODO where to put vat regime and vat country ????

class Order;
struct Fees{
    double amountTotal;
    QStringList ids;
    QList<double> amounts;
    Fees();
};

struct Price{
    double taxed;
    double untaxed;
    double taxes;
    Price();
};

class Shipment
{
public:
    static QString VAT_REGIME_NONE;
    //static QString VAT_REGIME_UE_DDP;
    static QString VAT_REGIME_NORMAL;
    static QString VAT_REGIME_NORMAL_EXPORT;
    static QString VAT_REGIME_OSS;
    static QString VAT_REGIME_IOSS;
    static QString VAT_RESPONSIBLE_SELLER;
    static QString VAT_RESPONSIBLE_MARKETPLACE;
    static QStringList *allRegimes();
    static QString amazonCountryDeclToCode(const QString &countryNameVatReport);
    Shipment(const QString &id,
             const QHash<QString, QSharedPointer<ArticleSold>> &articlesShipped,
             const Shipping &shipping,
             const QDateTime &dateTime,
             const Address &addressFrom,
             const QString &currency,
             double vatForRoundCorrection);
    void setAmazonVatInformations(
            const QString &countryVatAmazon,
            const QString &countrySaleDeclarationAmazon,
            const QString &regimeVatAmazon,
            double totalPriceTaxedAmazon,
            double totalPriceTaxesAmazon);
    QString getAmazonVatRate() const;
    bool hasAmazonVatInformations() const;
    virtual ~Shipment();
    virtual void init(Order *order);
    bool isInitialized() const;
    bool isComplete() const;
    virtual bool isCompletelyLoaded() const;
    bool isRefund() const;
    bool containsSku(const QString &sku) const;
    virtual QString channel() const;
    virtual QString subchannel() const;
    bool isMarketplaceResponsibleVat() const;

    bool operator<(const Shipment &other) const;
    bool operator>(const Shipment &other) const;
    void merge(const Shipment &shipment);
    //void recomputeVat(const QHash<QString, double> &totalSaleDestCountry);
    //QString vatRegime() const;
    QString countryVat() const; ///Only in computed
    QString countryCodeTo() const; ///Only in computed
    QString countryCodeFrom() const; ///Only in computed
    QString countryNameTo() const; ///Only in computed
    QString countryNameFrom() const; ///Only in computed
    QSet<QString> skusSet() const;
    virtual void computeVatRegime(double &totalSaleCountryOss, bool alsoRecomputeVat = false);

    QString getId() const;
    const Order *getOrder() const;
    virtual QString orderId() const;
    bool isVatRegimeComputed() const;
    bool isService() const;

    QHash<QString, QSharedPointer<ArticleSold> > getArticlesShipped() const;
    QSharedPointer<ArticleSold> getArticleShipped(const QString &sku) const;
    int getArticleCount() const;
    int getUnitCounts() const;
    QHash<QString, QSharedPointer<ArticleSold>> articlesShipped() const;
    void setArticlesShipped(
            const QHash<QString, QSharedPointer<ArticleSold> > &articlesShipped);
    //void addArticleShipped(QSharedPointer<ArticleSold> article);
    void addArticleShipped(const QString &id, QSharedPointer<ArticleSold> article);

    double getTotalPriceTaxed() const;
    double getTotalPriceTaxes(bool round = true) const;
    double getTotalPriceUntaxed() const;
    double getTotalPriceTaxedConverted() const;
    double getTotalPriceTaxesConverted() const;
    double getTotalPriceUntaxedConverted() const;
    QMap<QString, QMap<QString, Price>> getTotalPriceTaxesByVatRate() const;
    QMap<QString, QMap<QString, Price>> getTotalPriceTaxesByVatRateConverted() const;
    double getTotalPriceTaxedAmazon() const;
    double getTotalPriceTaxedAmazonConverted() const;
    double getTotalPriceUntaxedAmazon() const;
    double getTotalPriceUntaxedAmazonConverted() const;
    double getTotalPriceTaxesAmazon() const;
    double getTotalPriceTaxesAmazonConverted() const;
    //QMap<QString, double> getArticleChargedFees() const;

    Shipping getShipping() const;
    void setShipping(const Shipping &value);
    bool isFirstShipment() const;

    double getVatForRoundCorrection() const;
    void setVatForRoundCorrection(double vatForRoundCorrection);
    void computeVatForRoundCorrection();
    QString invoicePrefix(int year) const;
    static QString invoicePrefixRefundKeyword();

    int getTotalQuantity() const;

    const QMap<QString, QHash<QString, Fees>> &getChargedFeesBySKU() const;
    void addChargedFee(
            const QString &lineId,
            const QString &feeName,
            const QString &sku,
            double amount);

    QDateTime getDateTime() const;
    void setDateTime(const QDateTime &dateTime);

    Address getAddressFrom() const;
    void setAddressFrom(const Address &addressFrom);

    int getNumberOfTheYear() const;
    void setNumberOfTheYear(int numberOfTheYear);

    const QString &getRegimeVat() const;

    QString getCountryCodeVat() const;
    QString getCountryNameVat() const;

    QString getPaymentId() const;
    void setPaymentId(const QString &paymentId);


    QString getCurrency() const;

    QString getVatCollectResponsible() const;
    void setVatCollectResponsible(const QString &vatCollectResponsible);

    QString getInvoiceNameMarketPlace() const;
    void setInvoiceNameMarketplace(const QString &invoiceNumber);

    QString getCountrySaleDeclaration() const;
    QString getCountrySaleDeclarationName() const;

    QString getInvoiceName() const;
    void setInvoiceName(const QString &invoiceNumber);

    bool getFromAmazonVatReports() const;
    void setFromAmazonVatReports(bool fromAmazonVatReports);

    void addReportFrom(const QDate &date, const QString &reportName);
    QHash<QString, QDate> getReportsFrom() const;
    QSet<QString> getSaleTypes() const;

    QString vatScheme() const;
    void setVatScheme(const QString &newVatScheme);

protected:
    static QHash<QString, QHash<QString, int>> m_countriesFromByCustomerId;
    QString m_id;
    QString m_invoiceNumberMarketplace;
    QString m_vatScheme;
    QString m_invoiceNumber;
    QDateTime m_dateTime;
    QHash<QString, QSharedPointer<ArticleSold>> m_articlesShipped;
    QMap<QString, QHash<QString, Fees>> m_chargedFeesBySKU;
    //QMap<QString, double> getChargedFees() const;
    //void setChargedFees(const QMap<QString, double> &chargedFees);
    void _recordAddressFromCountry();
    Address m_addressFrom;
    Shipping m_shipping;
    double m_vatForRoundCorrection;
    Order *m_order;
    int m_numberOfTheYear;
    QString m_currency;
    QString m_countryVat;
    QString m_countrySaleDeclaration;
    QString m_vatCollectResponsible;
    QString m_regimeVat;
    QString m_paymentId;
    bool m_fromAmazonVatReports;
    QString m_countryVatAmazon;
    QString m_countrySaleDeclarationAmazon;
    QString m_regimeVatAmazon;
    double m_totalPriceTaxedAmazon;
    double m_totalPriceTaxesAmazon;
    QHash<QString, QDate> m_reportsFrom;
};

#endif // SHIPMENT_H
