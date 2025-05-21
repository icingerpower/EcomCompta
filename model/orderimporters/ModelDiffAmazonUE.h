#ifndef MODELDIFFAMAZONUE_H
#define MODELDIFFAMAZONUE_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qhash.h>
#include <QtCore/qvariant.h>
#include <model/utils/SortedMap.h>

class OrderManager;
class Shipment;

class ModelDiffAmazonUE : public QAbstractTableModel
{
    Q_OBJECT

public:
    static QString COL_SHIPMENT_ID;
    static QString COL_ORDER_ID;
    static QString COL_TRANSACTION_TYPE;
    static QString COL_INVOICE_NUMBER;
    static QString COL_DATE;
    static QString COL_VAT_REGIME;
    static QString COL_MARKETPLACE;
    static QString COL_COUNTRY_VAT;
    static QString COL_COUNTRY_DECLARATION;
    static QString COL_VAT;
    static QString COL_TOTAL;
    static QString COL_CURRENCY;
    static QString COL_BUYER_VAT_NUMBER;
    static QString COL_COUNTRY_TO;
    static QString COL_COUNTRY_FROM;
    static QString COL_SKU;
    static QString COL_QTY;
    static QString COL_REFUND_ID;
    static QString COL_MERGED_SKUS;
    static QString LOC_AMAZON_ONLY;
    static QString LOC_REPORTS_ONLY;
    static QString LOC_ALL;
    explicit ModelDiffAmazonUE(QObject *parent = nullptr);
    void compute(const OrderManager *orderManager, int year);
    void clear();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int totalShipmentAnalyzed() const;

private:
    void computeAmazonVatData(int year);
    struct ColInfo {
        QString name;
        QVariant (*getValue)(const Shipment *shipment,
                             const QHash<QString, QString> &amazonValues);
        bool compareWithPrevious;
    };
    int m_totalShipmentAnalyzed;
    QList<ColInfo> *colInfos() const;
    QList<QVector<QVariant>> m_values;
    QHash<QString, QHash<QString, QString>> m_valuesAmazon;
};

#endif // MODELDIFFAMAZONUE_H
