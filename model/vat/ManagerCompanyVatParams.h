#ifndef MANAGERCOMPANYVATPARAMS_H
#define MANAGERCOMPANYVATPARAMS_H

#include <QtCore/qabstractitemmodel.h>
#include <model/utils/SortedMap.h>

class ManagerCompanyVatParams : public QAbstractTableModel
{
    Q_OBJECT
public:
    static QString VAL_CURRENCY;
    static QString VAL_COUNTRY;
    static ManagerCompanyVatParams *instance();
    QString currency() const;
    QString countryNameCompany() const;
    QString countryCodeCompany() const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;

private:
    explicit ManagerCompanyVatParams(QObject *parent = nullptr);
    SortedMap<QString, QVariant> m_values;
    QString m_currency;
    QString m_country;
};

#endif // MANAGERCOMPANYVATPARAMS_H
