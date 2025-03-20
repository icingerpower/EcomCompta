#ifndef SALEGROUPS_H
#define SALEGROUPS_H

#include <QAbstractItemModel>
#include <QSet>

#include "model/UpdateToCustomer.h"

class SaleGroups : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT

public:
    static SaleGroups *instance();
    static const QStringList AMAZONS;
    static const QString AMAZON_EU;
    static const QString AMAZON_CA;
    static const QString AMAZON_UK;
    static const QString AMAZON_US;
    static const QString AMAZON_JP;

    void add(const QString &name);
    void remove(const QModelIndex &index);
    QSet<QString> getAmazons(const QModelIndex &index) const; //amazon.fr, amazon.de...
    QSet<QString> getExtAmazons(const QModelIndex &index) const; // DE, FR...
    QStringList getKeywordsSkus(const QModelIndex &index) const;
    QSet<QString> getKeywordsSkusAsSet(const QModelIndex &index) const;
    void setKeywordsSkus(const QModelIndex &index, const QString &text);

    QString uniqueId() const override;
    void onCustomerSelectedChanged(
            const QString &customerId) override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    static const QStringList HEADERS;
    static const int IND_NAME;
    static const int IND_AMAZON;
    static const int IND_ID;
    explicit SaleGroups(QObject *parent = nullptr);
    QList<QVariantList> m_listOfVariantList;
    QStringList m_listOfKeywordSkus;
    void _clear();
    QString settingsKeyKeywordSkus() const;
    void saveInSettings() const;
    void loadFromSettings();
};

#endif // SALEGROUPS_H
