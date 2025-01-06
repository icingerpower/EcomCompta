#include <QDate>
#include <QFile>
#include <QTextStream>

#include "model/orderimporters/VatOrdersModel.h"
#include "model/orderimporters/Shipment.h"
#include "model/orderimporters/Order.h"

#include "SalesLatestTable.h"

SalesLatestTable::SalesLatestTable(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_colInfos << ColInfo{
                  tr("SKU")
                  , [](const QString &sku) -> QVariant {
        return sku;
    }
                  , [](const QVariant &value1, const QVariant &value2) -> bool {
        return value1.toString() < value2.toString();
    }};
    m_colInfos << ColInfo{
                  tr("Title")
                  , [this](const QString &sku) -> QVariant {
        const auto &names = m_articleInfos[sku].names;
                     auto it = names.find("fr");
                     if (it == names.end())
                     {
                         return it.value();
                     }
                     it = names.find("en");
                     {
                         return it.value();
                     }
                     return names.begin().value();
    }
                  , [](const QVariant &value1, const QVariant &value2) -> bool {
        return value1.toString() < value2.toString();
    }};
    m_colInfos << ColInfo{
                  tr("Quantity")
                  , [this](const QString &sku) -> QVariant {
        return m_articleInfos[sku].quantity;
    }
                  , [](const QVariant &value1, const QVariant &value2) -> bool {
        return value1.toInt() < value2.toInt();
    }};
    m_colInfos << ColInfo{
                  tr("N orders")
                  , [this](const QString &sku) -> QVariant {
        return m_articleInfos[sku].nOrders;
    }
                  , [](const QVariant &value1, const QVariant &value2) -> bool {
        return value1.toInt() < value2.toInt();
    }};
}


QVariant SalesLatestTable::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Vertical)
        {
            return QString::number(section + 1);
        }
        else
        {
            return m_colInfos[section].colName;
        }
    }
    return QVariant{};
}

int SalesLatestTable::rowCount(const QModelIndex &) const
{
    return m_codesSorted.size();
}

int SalesLatestTable::columnCount(const QModelIndex &) const
{
    return m_colInfos.size();
}

QVariant SalesLatestTable::data(
        const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        return m_colInfos[index.column()].value(m_codesSorted[index.row()]);
    }
    return QVariant();
}

void SalesLatestTable::recordMovement(
        const QString &code,
        const QString &title,
        const QString &lang,
        int unit)
{
    if (!m_articleInfos.contains(code))
    {
        m_articleInfos[code].nOrders = 0;
        m_articleInfos[code].quantity = 0;
    }
    m_articleInfos[code].quantity += unit;
    ++m_articleInfos[code].nOrders;
    m_articleInfos[code].names[lang] = title;
}

void SalesLatestTable::compute(
        QSet<QString> keywordSkus,
        QSet<QString> subChannels,
        const QDate &dateFrom,
        const QDate &dateTo)
{
    Q_ASSERT(dateFrom < dateTo);
    _clear();
    int year = dateTo.year();
    VatOrdersModel::instance()->computeVat(
                year,
                [this, dateFrom, dateTo, subChannels, keywordSkus](
                const Shipment *shipment){
        if (!shipment->isRefund() && subChannels.contains(shipment->subchannel()))
        {
            QDate date = shipment->getOrder()->getDateTime().date();
            if (date >= dateFrom && date <= dateTo)
            {
                for (auto &article : shipment->getArticlesShipped()) {
                    recordMovement(
                                article->getSku(),
                                article->getName(),
                                shipment->getOrder()->getLangCode(),
                                article->getUnits());
                }
            }
        }
    }
    , QString{}
    , QString{}
    , [](const Shipment *shipment) -> bool {
        return shipment->getOrder() != nullptr;
    }
    );

    QSet<QString> codeToRemove;
    for (auto it = m_articleInfos.begin();
         it != m_articleInfos.end(); ++it)
    {
        bool ok = false;
        for (const auto &keywordOrSku : keywordSkus)
        {
            if (it.key().contains(keywordOrSku))
            {
                ok = true;
                break;
            }
            for (auto itTitle = it.value().names.begin();
                 itTitle != it.value().names.end(); ++itTitle)
            {
                if (itTitle.value().contains(keywordOrSku))
                {
                    ok = true;
                    break;
                }
            }
            if (ok)
            {
                break;
            }
        }
        if (ok)
        {
            m_codesSorted << it.key();
        }
        else
        {
            codeToRemove << it.key();
        }
    }
    for (const auto &code : qAsConst(codeToRemove))
    {
        m_articleInfos.remove(code);
    }
}

void SalesLatestTable::exportCsv(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QFile::WriteOnly))
    {
        QString colSep{"\n"};
        QTextStream stream{&file};
        int nRows = rowCount();
        int nColumns = columnCount();
        QStringList header;
        for (int i=0;i <nColumns; ++i)
        {
            header << headerData(i, Qt::Horizontal).toString();
        }
        stream << header.join(colSep);
        for (int i=0; i<nRows; ++i)
        {
            QStringList elements;
            for (int j=0;j <nColumns; ++j)
            {
                elements << index(i, j).data().toString();
            }
            stream << "\n" + elements.join(colSep);
        }
        file.close();
    }
}

void SalesLatestTable::sort(int column, Qt::SortOrder order)
{
    if (order == Qt::AscendingOrder)
    {
        std::sort(m_codesSorted.begin(),
                  m_codesSorted.end(),
                  [this, column](const QString &code1, const QString &code2){
            const auto &colInfo = m_colInfos[column];
            return colInfo.compareInf(colInfo.value(code1), colInfo.value(code2));
        });
    }
    else
    {
        std::sort(m_codesSorted.begin(),
                  m_codesSorted.end(),
                  [this, column](const QString &code1, const QString &code2){
            const auto &colInfo = m_colInfos[column];
            return !colInfo.compareInf(colInfo.value(code1), colInfo.value(code2));
        });
    }
    emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
}

void SalesLatestTable::_clear()
{
    bool remove = m_codesSorted.size() > 0;
    if (remove)
    {
        beginRemoveRows(QModelIndex{}, 0, m_codesSorted.size()-1);
    }
    m_codesSorted.clear();
    m_articleInfos.clear();
    if (remove)
    {
        endInsertRows();
    }
}

