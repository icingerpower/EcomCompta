#ifndef SETTINGINVOICESHEADOFFICE_H
#define SETTINGINVOICESHEADOFFICE_H

#include <QAbstractTableModel>

#include "model/UpdateToCustomer.h"

class SettingInvoicesHeadOffice : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT

public:
    static QStringList colNames;
    explicit SettingInvoicesHeadOffice(QObject *parent = nullptr);
    static SettingInvoicesHeadOffice *instance();

    QStringList addressFrom(const QDate &date) const;
    QStringList textBottomLegal(const QDate &date) const;
    QStringList textBottomLaw(const QDate &date) const;
    void addDate(const QDate &date);
    void removeDate(const QModelIndex &row);

    void saveInSettings() const;
    void loadFromSettings();

    void onCustomerSelectedChanged(
            const QString &customerId) override;
    QString uniqueId() const override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    QList<QList<QVariant>> m_listOfVariantList;
    int _getRowOfDate(const QDate &date) const;
};

#endif // SETTINGINVOICESHEADOFFICE_H
