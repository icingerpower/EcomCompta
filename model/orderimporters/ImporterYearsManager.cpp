#include "ImporterYearsManager.h"
#include "model/CustomerManager.h"

//----------------------------------------------------------
ImporterYearsManager::ImporterYearsManager(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &ImporterYearsManager::onCustomerSelectedChanged);
}
//----------------------------------------------------------
void ImporterYearsManager::onCustomerSelectedChanged(
        const QString &)
{
    if (m_years.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_years.size()-1);
        m_years.clear();
        m_yearsSet.clear();
        endRemoveRows();
    }
}
//----------------------------------------------------------
ImporterYearsManager *ImporterYearsManager::instance()
{
    static ImporterYearsManager instance;
    return &instance;
}
//----------------------------------------------------------
QList<int> ImporterYearsManager::years() const
{
    return m_years;
}
//----------------------------------------------------------
void ImporterYearsManager::recordYear(int year)
{
    //Q_ASSERT(year != 2000); //TODO fix
    //Q_ASSERT(year != 0);
    if (!m_yearsSet.contains(year)) {
        m_yearsSet << year;
        if (m_years.size() == 0 || year > m_years[0]) {
            beginInsertRows(QModelIndex(), 0, 0);
            m_years.insert(0, year);
            endInsertRows();
        } else if (year < m_years.last()) {
            beginInsertRows(QModelIndex(), m_years.size(), m_years.size());
            m_years << year;
            endInsertRows();
        } else {
            m_years << year;
            std::sort(m_years.begin(), m_years.end(),
                      [](int left, int right) -> bool {return left > right;});
            int index = m_years.indexOf(year);
            beginInsertRows(QModelIndex(), index, index);
            endInsertRows();
        }
    }
}
//----------------------------------------------------------
int ImporterYearsManager::rowCount(
        const QModelIndex &) const
{
    return m_years.size();
}
//----------------------------------------------------------
int ImporterYearsManager::columnCount(
        const QModelIndex &) const
{
    return 1;
}
//----------------------------------------------------------
QVariant ImporterYearsManager::data(
        const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        return m_years[index.row()];
    }
    return QVariant();
}
//----------------------------------------------------------
