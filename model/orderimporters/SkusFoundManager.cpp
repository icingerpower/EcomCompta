#include "SkusFoundManager.h"

//----------------------------------------------------------
SkusFoundManager *SkusFoundManager::instance()
{
    static SkusFoundManager instance;
    return &instance;
}
//----------------------------------------------------------
SkusFoundManager::SkusFoundManager(QObject *object)
    : QAbstractListModel(object)
{
}
//----------------------------------------------------------
void SkusFoundManager::add(const QString &sku)
{
    int initialSize = m_skus.size();
    m_skus << sku;
    if (m_skus.size() > initialSize) {
        if (m_filter.isEmpty() || sku.contains(m_filter)) {
            beginInsertRows(QModelIndex(), 0, 0);
            m_filteredSkus.insert(0, sku);
            endInsertRows();
        }
    }
}
//----------------------------------------------------------
void SkusFoundManager::select(const QString &sku)
{
    m_selSkus << sku;
    int index = m_filteredSkus.indexOf(sku);
    beginInsertRows(QModelIndex(), index, index);
    m_filteredSkus.removeAt(index);
    endRemoveRows();
}
//----------------------------------------------------------
void SkusFoundManager::unselect(const QString &sku)
{
    m_selSkus.remove(sku);
    if (sku.contains(m_filter)) {
        beginInsertRows(QModelIndex(), 0, 0);
        m_filteredSkus.insert(0, sku);
        endInsertRows();
    }
}
//----------------------------------------------------------
int SkusFoundManager::rowCount(const QModelIndex &) const
{
    int length = m_filteredSkus.length();
    return length;
}
//----------------------------------------------------------
int SkusFoundManager::columnCount(const QModelIndex &) const
{
    return 1;
}
//----------------------------------------------------------
QVariant SkusFoundManager::data(
        const QModelIndex &index, int role) const
{
    QVariant variant;
    if (role == Qt::DisplayRole) {
        variant = m_filteredSkus[index.row()];
    }
    return variant;
}
//----------------------------------------------------------
void SkusFoundManager::filters(const QString &string)
{
    beginRemoveRows(QModelIndex(), 0, m_filteredSkus.length()-1);
    m_filter = string;
    m_filteredSkus.clear();
    endRemoveRows();
    for (auto val : m_skus) {
        if (val.contains(string) && !m_selSkus.contains(string)) {
            m_filteredSkus << string;
        }
    }
    beginInsertRows(QModelIndex(), 0, m_filteredSkus.length()-1);
    std::sort(m_filteredSkus.begin(), m_filteredSkus.end());
    endInsertRows();
}
//----------------------------------------------------------
void SkusFoundManager::clear()
{
    beginRemoveRows(QModelIndex(), 0, m_filteredSkus.length());
    m_filteredSkus.clear();
    m_skus.clear();
    m_filter.clear();
    endRemoveRows();
}
//----------------------------------------------------------
