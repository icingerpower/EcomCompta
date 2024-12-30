#ifndef MANAGERBUNDLE_H
#define MANAGERBUNDLE_H

#include <QAbstractTableModel>

#include "model/UpdateToCustomer.h"

class ManagerBundle : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT

public:
    static ManagerBundle *instance();
    ~ManagerBundle() override;
    QString uniqueId() const override;
    void onCustomerSelectedChanged(
            const QString &customerId) override;
    QList<QPair<QString, int>> codesBase(const QString &mainCode) const;
    bool isBundle(const QString &code) const;
    void addBundleFile(const QString filePath);

    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    explicit ManagerBundle(QObject *parent = nullptr);
    void loadFromSettings();
    void saveInSettings();
    void _clear();
    void _addBundleFile(const QString filePath);
    QStringList m_codeByOrders;
    QHash<QString, QList<QPair<QString, int>>> m_mainCodeToSeverals;
    QHash<QString, int> m_mainCodeToFirstIndex;
};

#endif // MANAGERBUNDLE_H
