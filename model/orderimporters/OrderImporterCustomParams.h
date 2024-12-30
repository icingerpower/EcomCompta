#ifndef ORDERIMPORTERCUSTOMPARAMS_H
#define ORDERIMPORTERCUSTOMPARAMS_H

#include <QtCore/qstring.h>
#include <QAbstractTableModel>

class OrderImporterCustomParams : public QAbstractTableModel
{
    Q_OBJECT

public:
    static QString ID_CSV_SEP;
    static QString ID_CSV_DEL;
    static QString ID_CURRENCY;
    static QString ID_ORDER;
    static QString ID_DATE_TIME;
    static QString ID_DATE_TIME_FORMAT;
    static QString ID_DATE;
    static QString ID_DATE_FORMAT;
    static QString ID_TIME;
    static QString ID_TIME_FORMAT;
    static QString ID_SKU;
    static QString ID_ARTICLE_NAME;
    static QString ID_QUANTITY;
    static QString ID_SUBCHANNEL;

    static QString ID_TOTAL_PAID_ORDER;

    static QString ID_ARTICLE_UNIT_PRICE;
    static QString ID_ARTICLE_SUM_PRICE;
    static QString ID_ARTICLE_DISCOUNT;
    static QString ID_ARTICLE_DISCOUNT_TO_DEDUCT;
    static QString ID_ARTICLE_SHIPPING;

    static QString ID_TOTAL_REFUNDED;
    static QString ID_DATE_REFUNDED;
    static QString ID_DATE_TIME_REFUNDED;
    static QString ID_VALUES_REFUND_TO_DEDUCT;

    static QString ID_NAME;
    static QString ID_STREET1;
    static QString ID_STREET2;
    static QString ID_POSTAL_CODE;
    static QString ID_CITY;
    static QString ID_STATE;
    static QString ID_COUNTRY_CODE;
    static QString ID_BUSINESS_VAT_NUMBER;
    static QString ID_BILL_NAME;
    static QString ID_BILL_BUSINESS_NAME;
    static QString ID_BILL_STREET1;
    static QString ID_BILL_STREET2;
    static QString ID_BILL_POSTAL_CODE;
    static QString ID_BILL_CITY;
    static QString ID_BILL_STATE;
    static QString ID_BILL_COUNTRY_CODE;

    explicit OrderImporterCustomParams(
            const QString &uniqueId,
            QObject *parent = nullptr);
    void saveInSettings() const;
    void loadFromSettings();

    QStringList columns(const QString &id) const;
    bool hasCol(const QString &id) const;
    bool hasDefaultValue(const QString &id) const;
    QString valueCol(const QString &id) const;
    QString valueDefault(const QString &id) const;
    QString valueAnyCol(const QString &id) const;

    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QString uniqueId() const;

private:
    void _generateValues();
    QString _settingKey() const;
    QString m_uniqueId;
    QList<QStringList> m_values;
    QHash<QString, int> m_titleToIndex;
    int m_indexUntilAndatory;
};


#endif // ORDERIMPORTERCUSTOMPARAMS_H
