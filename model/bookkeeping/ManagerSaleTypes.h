#ifndef MANAGERSALETYPES_H
#define MANAGERSALETYPES_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qstringlist.h>

class ManagerSaleTypes : public QAbstractListModel
{
    Q_OBJECT

public:
    static QString SALE_PRODUCTS;
    static QString SALE_SERVICES;
    static QString SALE_PAYMENT_FASCILITOR;
    static ManagerSaleTypes *instance();
    ~ManagerSaleTypes() override;

    // Basic functionality:
    int rowCount(
            const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(
            const QModelIndex &index,
            int role = Qt::DisplayRole) const override;

private:
    explicit ManagerSaleTypes(QObject *parent = nullptr);
    QStringList m_saleTypes;
};

#endif // MANAGERSALETYPES_H
