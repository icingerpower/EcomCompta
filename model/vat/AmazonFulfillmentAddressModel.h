#ifndef FBACENTERADDRESSMODEL_H
#define FBACENTERADDRESSMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qstringlist.h>

#include "model/orderimporters/Address.h"

struct FBACenterItem {
    static FBACenterItem parentKnown;
    static FBACenterItem childKnown;
    static FBACenterItem parentAdded;
    static FBACenterItem childAdded;
    //ParentCenterItem *parent;
    //ParentCenterItem(ParentCenterItem *parent = nullptr);
    //bool hasParent() const;
};

class AmazonFulfillmentAddressModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    static AmazonFulfillmentAddressModel *instance();
    ~AmazonFulfillmentAddressModel() override;
    static QStringList countriesUE();

    bool contains(const QString &name) const;
    Address getAddress(const QString &centerName) const;
    void add(const Address &address);
    void remove(int index);
    void saveInSettings() const;
    void loadFromSettings();

    QModelIndex index(
            int row,
            int column,
            const QModelIndex &parent = QModelIndex()
            ) const override;
    QModelIndex parent(
            const QModelIndex &index) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int	rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int	columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role) const override;
    QVariant data(const QModelIndex &index,
                  int role) const override;
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;

private:
    AmazonFulfillmentAddressModel(QObject *parent = nullptr);
    QList<Address> m_addresses;
    QHash<QString, Address> m_centersByCode;
    QList<Address> m_addedAddresses;
    QHash<QString, Address> m_addedCentersByCode;
    void _loadKnownCenters();
};

#endif // FBACENTERADDRESSMODEL_H
