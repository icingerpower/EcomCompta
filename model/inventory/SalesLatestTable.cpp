#include <QDate>
#include <QTextStream>
#include <QFile>
#include <QTextStream>

#include "model/orderimporters/VatOrdersModel.h"
#include "model/orderimporters/Shipment.h"
#include "model/orderimporters/Order.h"
#include "model/inventory/CsvOrderFolders.h"
#include "model/inventory/SaleColumnTree.h"

#include "SalesLatestTable.h"

SalesLatestTable::SalesLatestTable(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_indColSku = 0;
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
                     if (it != names.end())
                     {
                         return it.value();
                     }
                     it = names.find("en");
                     if (it != names.end())
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
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return m_colInfos[index.column()].value(m_codesSorted[index.row()]);
    }
    return QVariant();
}

Qt::ItemFlags SalesLatestTable::flags(const QModelIndex &) const
{
    return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
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
    int yearFrom = dateFrom.year();
    int yearTo = dateTo.year();
    for (int year = yearFrom; year <= yearTo; ++year)
    {
        VatOrdersModel::instance()->computeVat(
                    year,
                    [this](
                    const Shipment *shipment){
            for (auto &article : shipment->getArticlesShipped()) {
                recordMovement(
                            article->getSku(),
                            article->getName(),
                            shipment->getOrder()->getLangCode(),
                            article->getUnits());
            }
        }
        , QString{}
        , QString{}
        , [dateFrom, dateTo, subChannels, keywordSkus](
        const Shipment *shipment) -> bool {
            if (shipment->getOrder() != nullptr
                    && !shipment->isRefund()
                    && subChannels.contains(shipment->subchannel()))
            {
                QDate date = shipment->getOrder()->getDateTime().date();
                if (date >= dateFrom && date <= dateTo)
                {
                    return true;
                }
            }
            return false;
        }
        );
    }

    QSet<QString> codeToRemove;
    QStringList codesSorted;
    for (auto it = m_articleInfos.begin();
         it != m_articleInfos.end(); ++it)
    {
        bool ok = false;
        for (const auto &keywordOrSku : qAsConst(keywordSkus))
        {
            if (it.key().contains(keywordOrSku, Qt::CaseInsensitive))
            {
                ok = true;
                break;
            }
            for (auto itTitle = it.value().names.begin();
                 itTitle != it.value().names.end(); ++itTitle)
            {
                if (itTitle.value().contains(keywordOrSku, Qt::CaseInsensitive))
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
            codesSorted << it.key();
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
    beginInsertRows(QModelIndex(), 0, codesSorted.size() - 1);
    m_codesSorted = codesSorted;
    endInsertRows();
}

void SalesLatestTable::exportCsv(
    const QString &filePath,
    SaleColumnTree *saleColumnTree,
    const QString &dirEconomics,
    const QDate &minDate)
{
    QFile file(filePath);
    //QStringList colNames{"Country", "Manufacturer", "EC REP"};
    auto gsprData = CsvOrderFolders::instance()->getGsprData(saleColumnTree);
    if (file.open(QFile::WriteOnly))
    {
        QString colSep{"\t"};
        QTextStream stream{&file};
        int nRows = rowCount();
        int nColumns = columnCount();
        QStringList header;
        for (int i=0;i <nColumns; ++i)
        {
            header << headerData(i, Qt::Horizontal).toString();
        }
        //header << colNames;
        header << saleColumnTree->getHeader();
        int indColWeight = saleColumnTree->getColIndUnitWeight();
        int indColUnitPrice = saleColumnTree->getColIndUnitPrice();
        double shippingPricePerKilo = 3.;
        CsvOrderFolders::instance()->addEconomicsData(
            dirEconomics,
            minDate,
            indColUnitPrice,
            indColWeight,
            shippingPricePerKilo,
            header,
            gsprData);
        stream << header.join(colSep);
        for (int i=0; i<nRows; ++i)
        {
            QStringList elements;
            for (int j=0;j <nColumns; ++j)
            {
                elements << index(i, j).data().toString();
            }
            const QString &sku = elements[m_indColSku];
            if (gsprData.contains(sku))
            {
                elements << gsprData[sku];
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

