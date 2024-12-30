#ifndef SERVICESALESMODEL_H
#define SERVICESALESMODEL_H

#include <QAbstractTableModel>

#include "model/UpdateToCustomer.h"

class ServiceSalesModel : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT

public:
    static const int IND_COL_DATE;
    static const int IND_COL_REFERENCE;
    static const int IND_COL_AMOUNT;
    static const int IND_COL_UNIT;
    static const int IND_COL_TITLE;
    static const int IND_COL_CURRENCY;
    static const int IND_COL_CUSTOMER_ID;
    static const int IND_COL_ADDRESS_ID;
    static const int N_COLS;
    static ServiceSalesModel *instance();
    QList<QList<QVariant>> loadListOfVariantList(const QString &filePath);
    QString getFilePath(int year) const;
    void load(int year);
    void addSale(
            const QDate &date,
            const QString &label,
            double amountUnit,
            int units,
            const QString &title,
            const QString &currency,
            const QString &customerId,
            const QString &addressId);
    void remove(const QModelIndex &index);
    void onCustomerSelectedChanged(const QString &customerId) override;
    QString uniqueId() const override;

    // Header:
    QVariant headerData(
            int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
    explicit ServiceSalesModel(QObject *parent = nullptr);
    QList<QList<QVariant>> m_listOfVariantList;
    void _saveInSettings();
    int m_year;
    void _clear();
};

#endif // SERVICESALESMODEL_H
