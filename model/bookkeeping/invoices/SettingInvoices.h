#ifndef SETTINGINVOICES_H
#define SETTINGINVOICES_H

#include <QAbstractItemModel>

#include "model/UpdateToCustomer.h"

class SettingInvoices : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT

public:
    static QString PAR_INVOICE_FIRST_NUMER;
    static QString PAR_INVOICE_PREFIX;
    static SettingInvoices *instance();
    void onCustomerSelectedChanged(
            const QString &customerId) override;
    QString uniqueId() const override;
    void addChangeDate();
    int invoiceFirstNumber() const;
    QString invoicePrefix(const QString &channel) const;
    void setAddressFrom(const QStringList &lines);
    QStringList addressFrom() const;
    void setTextBottomLegal(const QStringList &lines);
    QStringList textBottomLegal() const;
    void setTextBottomLaw(const QStringList &lines);
    QStringList textBottomLaw() const;
    void saveInSettings() const;
    void loadFromSettings();

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
    explicit SettingInvoices(QObject *parent = nullptr);
    void _initDefaultValues();
    QString _paramChannel(const QString &channelName) const;
    QStringList m_paramNames;
    QHash<QString, QVariant> m_paramValues;
    QString _settingKeyAddress() const;
    QString _settingKeyBottomLegal() const;
    QString _settingKeyBottomLaw() const;
};

#endif // SETTINGINVOICES_H
