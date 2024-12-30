#ifndef IMPORTERYEARSMANAGER_H
#define IMPORTERYEARSMANAGER_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qset.h>
#include <QtCore/qlist.h>

class ImporterYearsManager : public QAbstractListModel
{
    Q_OBJECT

public:
    static ImporterYearsManager *instance();
    QList<int> years() const;
    void recordYear(int year);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void onCustomerSelectedChanged(const QString &customerId);

private:
    explicit ImporterYearsManager(QObject *parent = nullptr);
    QSet<int> m_yearsSet;
    QList<int> m_years;
};

#endif // IMPORTERYEARSMANAGER_H
