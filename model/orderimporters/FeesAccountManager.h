#ifndef FEESACCOUNTMANAGER_H
#define FEESACCOUNTMANAGER_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qmap.h>

class FeesAccountManager : public QAbstractTableModel
{
    Q_OBJECT

public:
    static FeesAccountManager *instance();
    void saveInSettings() const;
    void loadFromSettings();
    void addAccount(const QString &number, const QString &label);
    void removeAccount(int position);
    QStringList toList() const;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;

signals:
    void accountUpdated(const QString &previousAccount, const QString newAccount);

private:
    explicit FeesAccountManager(QObject *parent = nullptr);
    QMap<QString, QString> m_accounts;
    QStringList m_accountList;
};

#endif // FEESACCOUNTMANAGER_H
