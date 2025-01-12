#ifndef SALETEMPLATEMANAGER_H
#define SALETEMPLATEMANAGER_H

#include <QAbstractListModel>

#include "model/UpdateToCustomer.h"

class SaleTemplateManager : public QAbstractListModel, public UpdateToCustomer
{
    Q_OBJECT

public:
    static SaleTemplateManager *instance();
    const QString &getId(const QModelIndex &index) const;
    void add(const QString &name);
    void remove(const QModelIndex &index);

    QString uniqueId() const override;
    void onCustomerSelectedChanged(
            const QString &customerId) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    explicit SaleTemplateManager(QObject *parent = nullptr);
    QList<QStringList> m_listOfStringList;
    void _clear();
    void saveInSettings() const;
    void loadFromSettings();
};

#endif // SALETEMPLATEMANAGER_H
