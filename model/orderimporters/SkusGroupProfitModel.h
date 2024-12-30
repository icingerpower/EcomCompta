#ifndef SKUSGROUPPROFITMODEL_H
#define SKUSGROUPPROFITMODEL_H

#include <QAbstractListModel>

#include "model/UpdateToCustomer.h"

class SkusGroupProfitModel : public QAbstractListModel, public UpdateToCustomer
{
    Q_OBJECT

public:
    static SkusGroupProfitModel *instance();

    QString addGroup(const QString &name);
    void insertGroup(int index, const QString &name);
    void removeGroup(const QModelIndex &index);
    QString groupId(const QModelIndex &index) const;

    QString getTextOfGroup(const QModelIndex &index) const;
    void setTextOfGroup(const QModelIndex &index, const QString &text);
    void setTextOfGroup(const QString &groupId, const QString &text);

    void onCustomerSelectedChanged(const QString &customerId) override;
    QString uniqueId() const override;

    // Header:
    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(
            const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    QList<QStringList> m_listOfStringList;
    explicit SkusGroupProfitModel(QObject *parent = nullptr);
    void _clear();
    void _saveInSettings();
    void _loadFromSettings();
    inline QString _genId(const QString &name) const;
    inline QString _settingIdGroup(const QString &groupId) const;

};

#endif // SKUSGROUPPROFITMODEL_H
