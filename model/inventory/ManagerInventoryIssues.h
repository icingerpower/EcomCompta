#ifndef MANAGERINVENTORYISSUES_H
#define MANAGERINVENTORYISSUES_H

#include <QAbstractTableModel>

#include "model/UpdateToCustomer.h"

class ManagerInventoryIssues : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT

public:
    static QString UNKWOWN;
    static QString UNAVAILABLE;
    static ManagerInventoryIssues *instance();
    QString uniqueId() const override;
    void onCustomerSelectedChanged(
            const QString &customerId) override;
    void clear();
    void record(const QString &code,
                     const QString &title,
                     const QString &type,
                     int unit);
    void exportUnknown(const QString &filePath);
    void exportAll(const QString &filePath);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    explicit ManagerInventoryIssues(QObject *parent = nullptr);
    QList<QStringList> m_values;
    QHash<QString, int> m_codeToIndex;
};

#endif // MANAGERINVENTORYISSUES_H
