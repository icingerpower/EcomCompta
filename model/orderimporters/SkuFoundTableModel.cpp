#include "../common/utils/CsvReader.h"
#include "../common/currencies/CurrencyRateManager.h"

#include "model/orderimporters/Shipment.h"
#include "model/CustomerManager.h"
#include "model/inventory/InventoryManager.h"
#include "model/orderimporters/Order.h"

#include "SkuFoundTableModel.h"

//----------------------------------------------------------
QString SkuFoundTableModel::ID_PARENT_SKU{"parent-sku"};
QString SkuFoundTableModel::ID_SKU{"sku"};
QString SkuFoundTableModel::ID_ASIN{"ASIN"};
QString SkuFoundTableModel::ID_UNIT_COST{"unit-cost"};
QString SkuFoundTableModel::ID_CURRENCY{"currency"};
QString SkuFoundTableModel::ID_MOQ{"MOQ"};
QString SkuFoundTableModel::ID_MONTH_INVENTORY{"month-inventory"};
QString SkuFoundTableModel::ID_SALE_ADS_COST{"sale-ads-cost"};
QString SkuFoundTableModel::ID_PROFIT_PERC{"profit-perc"};
QString SkuFoundTableModel::ID_PROFIT_PERC_NO_ADS{"profit-perc-no-ads"};
QString SkuFoundTableModel::ID_CAPITAL_BLOCKED{"capital-blocked"};
QString SkuFoundTableModel::ID_PROFIT_PERC_MONTH{"profit-perc-month"};
QString SkuFoundTableModel::ID_CAPITAL_MONTH_RETURN{"capital-month-return"};
//----------------------------------------------------------
SkuFoundTableModel *SkuFoundTableModel::instance()
{
    static SkuFoundTableModel instance;
    return &instance;
}
//----------------------------------------------------------
SkuFoundTableModel::SkuFoundTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    /*
    m_colNames << "SKU parent";
    m_colNames << "SKU";
    m_colNames << "ASIN";
    m_colNames << "Unit cost";
    m_colNames << "Currency";
    m_colNames << "MOQ";
    m_colNames << "Month inventory";
    m_colNames << "Sale ads cost";
    m_colNames << "Profit %";
    m_colNames << "Profit % no ads";
    m_colNames << "Capital blocked";
    m_colNames << "Capital profit %";
    m_colNames << "Month for capital return";
    QStringList domains{
        "amazon.fr"
        , "amazon.com"
        , "amazon.ca"
        , "amazon.de"
        , "amazon.co.uk"
    };
//*/
    _retrieveUnitCost();
    _initColInfos();
    /*
    for (auto it = domains.begin();
         it != domains.end(); ++it) {
        m_domains << *it;
    }
    for (int i=0; i<m_colNames.size(); ++i) {
        m_colNameToColIndex[i] = m_colNames[i];
    }
//*/
}
//----------------------------------------------------------
void SkuFoundTableModel::_retrieveUnitCost()
{
    QList<int> years;
    auto invFilePaths = InventoryManager::instance()->getInventoryFilePaths(years);
    std::sort(invFilePaths.begin(), invFilePaths.end());
    for (const auto &invFilePath : invFilePaths)
    {
        CsvReader csvReader(invFilePath, "\t");
        if (csvReader.readAll())
        {
            QString dateString = QFileInfo{invFilePath}.fileName().split("__")[0];
            QDate date = QDate::fromString(dateString, "yyyy-MM-dd");
            auto dataRode = csvReader.dataRode();
            int posSku = dataRode->header.pos({"sku", "SKU", InventoryManager::COL_CODE});
            int posPriceUnit = dataRode->header.pos({"price", InventoryManager::COL_UNIT_PRICE});
            int posCurrency = dataRode->header.pos({"currency", InventoryManager::COL_CURRENCY});
            for (auto elements : dataRode->lines)
            {
                QString sku = elements[posSku].trimmed();
                if (!m_sku_unitCostLast.contains(sku))
                {
                    double unitPrice = elements[posPriceUnit].trimmed().replace(",", ".").toDouble();
                    QString currency = elements[posCurrency].trimmed();
                    if (currency != CustomerManager::instance()->getSelectedCustomerCurrency())
                    {
                        double rate = CurrencyRateManager::instance()->rate(
                            currency,
                            CustomerManager::instance()->getSelectedCustomerCurrency(),
                            date);
                        unitPrice *= rate;
                    }
                    m_sku_unitCostLast[sku] = unitPrice;
                }
            }
        }
    }
}
//----------------------------------------------------------
void SkuFoundTableModel::_initColInfos()
{
    m_colInfos << ColInfo{ID_PARENT_SKU,
                          "SKU Parent",
                          [this](const QString &sku, const QSet<const Shipment *> &) -> QVariant{
                              auto itSku = m_sku_col_variant.find(sku);
                              if (itSku != m_sku_col_variant.end())
                              {
                                  return itSku.value().value(ID_PARENT_SKU, QVariant{});
                              }
                              return QVariant{};
                              }};
    m_colInfos << ColInfo{ID_SKU,
                          "SKU",
                          [](const QString &sku, const QSet<const Shipment *> &) -> QVariant{
                              return sku;
                              }};
    m_colInfos << ColInfo{ID_ASIN,
                          "ASIN",
                          [this](const QString &sku, const QSet<const Shipment *> &) -> QVariant{
                              auto itSku = m_sku_col_variant.find(sku);
                              if (itSku != m_sku_col_variant.end())
                              {
                                  return itSku.value().value(ID_ASIN, QVariant{});
                              }
                              return QVariant{};
                              }};
    m_colInfos << ColInfo{ID_UNIT_COST,
                          "Unit cost",
                          [this](const QString &sku, const QSet<const Shipment *> &) -> QVariant{
                              return m_sku_unitCostLast.value(sku, 0.);
                              }};
    m_colInfos << ColInfo{ID_SKU,
                          "Currency",
                          [](const QString &, const QSet<const Shipment *> &) -> QVariant{
                              return CustomerManager::instance()->getSelectedCustomerCurrency();
                              }};
    m_colInfos << ColInfo{ID_MOQ,
                          "MOQ",
                          [this](const QString &sku, const QSet<const Shipment *> &) -> QVariant{
                              auto itSku = m_sku_col_variant.find(sku);
                              if (itSku != m_sku_col_variant.end())
                              {
                                  return itSku.value().value(ID_MOQ, QVariant{});
                              }
                              return QVariant{};
                              }};
    m_colInfos << ColInfo{ID_MONTH_INVENTORY,
                          "Month inventory",
                          [this](const QString &sku, const QSet<const Shipment *> &) -> QVariant{
                              auto itSku = m_sku_col_variant.find(sku);
                              if (itSku != m_sku_col_variant.end())
                              {
                                  return itSku.value().value(ID_MONTH_INVENTORY, 2);
                              }
                              return 2;
                              }};
    m_colInfos << ColInfo{ID_SALE_ADS_COST,
                          "Sale ads cost",
                          [this](const QString &sku, const QSet<const Shipment *> &) -> QVariant{
                              return 0; //TODO analyse ads report
                              auto itSku = m_sku_col_variant.find(sku);
                              if (itSku != m_sku_col_variant.end())
                              {
                                  return itSku.value().value(ID_SALE_ADS_COST, 0);
                              }
                              return 0;
                              }};
    m_colInfos << ColInfo{ID_PROFIT_PERC,
                          "Profit (%)",
                          [this](const QString &sku, const QSet<const Shipment *> &shipments) -> QVariant{

                              auto itSku = m_sku_col_variant.find(sku);
                              if (itSku != m_sku_col_variant.end())
                              {
                                  static ProfitFunction adsCostFunction = _getProfitFunction(ID_SALE_ADS_COST);
                                  //TODO continue from here
                                  return itSku.value().value(ID_PROFIT_PERC, QVariant{});
                              }
                              return QVariant{};
                              }};
    m_colInfos << ColInfo{ID_PROFIT_PERC_NO_ADS,
                          "Profit (%) No Ads",
                          [this](const QString &sku, const QSet<const Shipment *> &shipments) -> QVariant{
                              auto itSku = m_sku_col_variant.find(sku);
                              if (itSku != m_sku_col_variant.end())
                              {
                                  return itSku.value().value(ID_PROFIT_PERC_NO_ADS, QVariant{});
                              }
                              return QVariant{};
                              }};
    m_colInfos << ColInfo{ID_CAPITAL_BLOCKED,
                          "Capital blocked",
                          [this](const QString &sku, const QSet<const Shipment *> &shipments) -> QVariant{
                              auto itSku = m_sku_col_variant.find(sku);
                              if (itSku != m_sku_col_variant.end())
                              {
                                  return itSku.value().value(ID_CAPITAL_BLOCKED, QVariant{});
                              }
                              return QVariant{};
                              }};
    m_colInfos << ColInfo{ID_PROFIT_PERC_MONTH,
                          "Monthly profit (%)",
                          [this](const QString &sku, const QSet<const Shipment *> &shipments) -> QVariant{
                              auto itSku = m_sku_col_variant.find(sku);
                              if (itSku != m_sku_col_variant.end())
                              { //TODO find out wht will be computed and compute it
                                  return itSku.value().value(ID_PROFIT_PERC_MONTH, QVariant{});
                              }
                              return QVariant{};
                              }};
    m_colInfos << ColInfo{ID_CAPITAL_MONTH_RETURN,
                          "Month for capital return",
                          [this](const QString &sku, const QSet<const Shipment *> &shipments) -> QVariant{
                              auto itSku = m_sku_col_variant.find(sku);
                              if (itSku != m_sku_col_variant.end())
                              {
                                  return itSku.value().value(ID_CAPITAL_MONTH_RETURN, QVariant{});
                              }
                              return QVariant{};
                              }};
}
//----------------------------------------------------------
ProfitFunction SkuFoundTableModel::_getProfitFunction(const QString &id) const
{
    for (auto colInfo : m_colInfos)
    {
        if (colInfo.id == id)
        {
            return colInfo.function;
        }
    }
    ProfitFunction adsCostFunction;
    return adsCostFunction;
}
//----------------------------------------------------------
void SkuFoundTableModel::addShipmentOrRefund(
        const Shipment *shipmentOrRefund)
{
    auto articlesShipped = shipmentOrRefund->getArticlesShipped();
    for (auto itArt = articlesShipped.begin();
         itArt != articlesShipped.end(); ++itArt) {
        //addSku(itArt.value()->getSku(),
               //shipmentOrRefund->getOrder()->getSubchannel().toLower(),
               //itArt.value()->getName());
        m_skuToShipmentRefunds[itArt.value()->getSku()] << shipmentOrRefund;
    }
}
//----------------------------------------------------------
void SkuFoundTableModel::compute()
{
    QMultiMap<int, QString> countToSku;
    for (auto it = m_skuToShipmentRefunds.begin();
         it != m_skuToShipmentRefunds.end(); ++it)
    {
        int count = 0;
        const QString &sku = it.key();
        for (const auto &shipmentOrRefund : it.value())
        {
            if (!shipmentOrRefund->isRefund())
            {
                count += shipmentOrRefund->getArticleShipped(sku)->getUnits();
            }
        }
        countToSku.insert(count, sku);
    }
    if (m_skus.size() > 0)
    {
        beginRemoveRows(QModelIndex(), 0, m_skus.size()-1);
        m_skus.clear();
        endRemoveRows();
    }
    auto tempSkus = m_skus;
    const auto &countKeys = countToSku.uniqueKeys();
    for (auto count : countKeys)
    {
        const auto &skus = countToSku.values(count);
        for (const auto &sku : skus)
        {
            tempSkus << sku;
        }
        //TODO I stop here, I need to associate col name to a function that compute QVariant from the list of shipments
        // I need to add export in CSV
        // I need to save date typed in hand
    }
    if (tempSkus.size() > 0)
    {
        beginInsertRows(QModelIndex(), 0, tempSkus.size()-1);
        m_skus = std::move(tempSkus);
        endInsertRows();
    }
}
//----------------------------------------------------------
bool SkuFoundTableModel::containsText(
        int rowIndex, const QString &text) const
{
    const QString &sku = m_skus[rowIndex];
    if (sku.contains(text, Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}
//----------------------------------------------------------
void SkuFoundTableModel::fillChartData(
        QMap<QDate, int> &chartSales,
        QMap<QDate, int> &chartRefunds,
        const QStringList &skus) const
{
    for (auto itSku = skus.begin();
         itSku != skus.end(); ++itSku) {
        auto itShipmentAndRefunds = m_skuToShipmentRefunds.constFind(*itSku);
        if (itShipmentAndRefunds != m_skuToShipmentRefunds.constEnd()) {
            for (auto itShipmentOrRefund = itShipmentAndRefunds->cbegin();
                 itShipmentOrRefund != itShipmentAndRefunds->cend(); ++itShipmentOrRefund) {
                auto shipmentOrRefund = *itShipmentOrRefund;
                auto date = shipmentOrRefund->getDateTime().date();
                if (!chartSales.contains(date)) {
                    chartSales[date] = 0;
                    chartRefunds[date] = 0;
                }
                int count = shipmentOrRefund->getUnitCounts();
                if (shipmentOrRefund->isRefund()) {
                    chartRefunds[date] += count;
                } else {
                    chartSales[date] += count;
                }
            }
        }
    }
}
//----------------------------------------------------------
void SkuFoundTableModel::fillTable(
        QStringList &headerVert,
        QStringList &headerHoriz,
        QList<QList<QVariant>> &elements,
        const QStringList &skus,
        double unitPrice,
        int MOQ,
        int minNumberMonth,
        double adsCostPerSale)
{
    headerHoriz << QObject::tr("Total ou année");
    headerHoriz << QObject::tr("Unitaire");
    int totalUnits = 0;
    int totalUnitsRefund = 0;
    double totalFeesCom = 0.;
    double totalFeesFba = 0.;
    double totalSaleTaxed = 0.;
    double totalSaleUntaxed = 0.;
    QMap<QDate, int> salePerMonth;
    for (auto itSku = skus.begin();
         itSku != skus.end(); ++itSku) {
        auto itShipmentAndRefunds = m_skuToShipmentRefunds.constFind(*itSku);
        if (itShipmentAndRefunds != m_skuToShipmentRefunds.constEnd()) {
            for (auto itShipmentOrRefund = itShipmentAndRefunds->begin();
                 itShipmentOrRefund != itShipmentAndRefunds->end(); ++itShipmentOrRefund) {
                auto shipmentOrRefund = *itShipmentOrRefund;
                auto date = shipmentOrRefund->getDateTime().date();
                date.setDate(date.year(), date.month(), 1);
                if (!salePerMonth.contains(date)) {
                    salePerMonth[date] = 0;
                }
                int unitCount = shipmentOrRefund->getUnitCounts();
                if (shipmentOrRefund->isRefund()) {
                    totalUnitsRefund += unitCount;
                    unitCount *= -1;
                }
                salePerMonth[date] += unitCount;
                totalUnits += unitCount;
                totalSaleTaxed += shipmentOrRefund->getTotalPriceTaxedConverted();
                totalSaleUntaxed += shipmentOrRefund->getTotalPriceUntaxedConverted();
                auto fees = shipmentOrRefund->getChargedFeesBySKU();
                auto itFees = fees.constFind(*itSku);
                if (itFees != fees.constEnd()) {
                    for (auto itFeeValue = itFees->begin();
                         itFeeValue != itFees->end(); ++itFeeValue) {
                        const QString &feeName = itFeeValue.key();
                        double feesTotal = itFeeValue.value().amountTotal;
                        if (feeName.contains("ommission")) {
                            totalFeesCom += feesTotal;
                        } else if (feeName.contains("FBA")) {
                            totalFeesFba += feesTotal;
                        }
                    }
                }
            }
        }
    }
    QList<int> monthlySales = salePerMonth.values();
    if (monthlySales.size() > 0) {
        std::sort(monthlySales.begin(), monthlySales.end());
        int estimationSaleMonth = 10;
        int saleMedian = 0;
        if (monthlySales.size() > 0) {
            if (monthlySales.size() % 2 == 0) {
                int indexUpper = monthlySales.size() / 2;
                int indexLower = indexUpper - 1;
                saleMedian = (monthlySales[indexLower] + monthlySales[indexUpper]) / 2;
            } else {
                saleMedian = monthlySales[monthlySales.size()/2];
            }
            if (monthlySales.size() < 6) {
                estimationSaleMonth = monthlySales[monthlySales.size()-1];
            } else {
                estimationSaleMonth = monthlySales[monthlySales.size()-3];
            }
        }
        int minNumberOfUnitsNeeded = qMin(MOQ, estimationSaleMonth*minNumberMonth);
        double totalFees = totalFeesCom + totalFeesFba;
        double profit = totalSaleUntaxed + totalFees - unitPrice * totalUnits;
        double profitAds = totalSaleUntaxed + totalFees - (unitPrice + adsCostPerSale) * totalUnits;
        double profitUnit = profit / totalUnits;
        double profitUnitAds = profitAds / totalUnits;
        double profitRel = profit / totalSaleUntaxed;
        double profitRelAds = profitAds / totalSaleUntaxed;
        double capitalNeeded = qMax(estimationSaleMonth, minNumberOfUnitsNeeded) * unitPrice * minNumberMonth;
        headerVert << QObject::tr("Unités vendues");
        elements << QList<QVariant>({totalUnits, 1});
        headerVert << QObject::tr("Unités remboursés (%)");
        elements << QList<QVariant>({100. * totalUnitsRefund / (totalUnits + totalUnitsRefund), 1});
        headerVert << QObject::tr("Ventes mensuelles median / max");
        elements << QList<QVariant>({saleMedian, monthlySales.last()});
        headerVert << QObject::tr("Ventes TTC");
        elements << QList<QVariant>({totalSaleTaxed, totalSaleTaxed / totalUnits});
        headerVert << QObject::tr("Ventes HT");
        elements << QList<QVariant>({totalSaleUntaxed, totalSaleUntaxed / totalUnits});
        headerVert << QObject::tr("Commissions");
        elements << QList<QVariant>({totalFeesCom, totalFeesCom / totalUnits});
        headerVert << QObject::tr("Frais FBA");
        elements << QList<QVariant>({totalFeesFba, totalFeesFba / totalUnits});
        headerVert << QObject::tr("Frais FBA (%)");
        elements << QList<QVariant>({totalFeesFba / totalSaleUntaxed * 100., QVariant()});
        headerVert << QObject::tr("Frais totaux");
        elements << QList<QVariant>({totalFees, totalFees / totalUnits});
        headerVert << QObject::tr("Marge");
        elements << QList<QVariant>({profit, profit / totalUnits});
        headerVert << QObject::tr("Marge (%)");
        elements << QList<QVariant>({profitRel * 100, QVariant()});
        if (adsCostPerSale > 0.001) {
            headerVert << QObject::tr("Marge Pub");
            elements << QList<QVariant>({profitAds, profitAds / totalUnits});
            headerVert << QObject::tr("Marge Pub (%)");
            elements << QList<QVariant>({profitRelAds * 100, QVariant()});
        }
        headerVert << QObject::tr("Marge per année / mois");
        elements << QList<QVariant>({profitUnit * estimationSaleMonth * 12, profitUnit * estimationSaleMonth});
        headerVert << QObject::tr("Unités par année / mois");
        elements << QList<QVariant>({estimationSaleMonth * 12, estimationSaleMonth});
        headerVert << QObject::tr("Capital requis");
        elements << QList<QVariant>({capitalNeeded, QVariant()});
        headerVert << QObject::tr("Marge / capital");
        elements << QList<QVariant>({(profitUnit * estimationSaleMonth) / capitalNeeded, QVariant()});
        if (adsCostPerSale > 0.001) {
            headerVert << QObject::tr("Marge / capital Pub");
            elements << QList<QVariant>({(profitUnitAds * estimationSaleMonth) / capitalNeeded, QVariant()});
        }
        headerVert << QObject::tr("Monthly capital one year / month");
        elements << QList<QVariant>({estimationSaleMonth * unitPrice * 12, estimationSaleMonth * unitPrice});
    }
}
/*
//----------------------------------------------------------
void SkuFoundTableModel::addSku(
        const QString &sku,
        const QString &domain,
        const QString &title)
{
    int origRowCount = m_skusDomainsTitles.size();
    int origColsSku = m_skusDomainsTitles[sku].size();
    m_skusDomainsTitles[sku][domain] = title;
    if (m_skusDomainsTitles.size() > origRowCount) {
        beginInsertRows(QModelIndex(), 0, 0);
        m_skus.insert(0, sku);
        endInsertRows();
    }
    int nCols = m_colNames.size();
    if (!m_domains.contains(domain)) {
        beginInsertColumns(QModelIndex(), nCols, nCols);
        m_domains << domain;
        m_colNameToColIndex[nCols] = domain;
        m_colNames << domain;
        ++nCols;
        endInsertColumns();
    } else if (m_skusDomainsTitles[sku].size() > origColsSku) {
        emit dataChanged(index(0, 1),
                         index(m_skusDomainsTitles.size()-1,
                               nCols-1));
    }
}
//*/
//----------------------------------------------------------
QVariant SkuFoundTableModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return m_colInfos[section].colName;
    }
    return QVariant();
}
//----------------------------------------------------------
int SkuFoundTableModel::rowCount(
        const QModelIndex &) const
{
    return m_skus.size();
}
//----------------------------------------------------------
int SkuFoundTableModel::columnCount(
        const QModelIndex &) const
{
    return m_colInfos.size();
}
//----------------------------------------------------------
QVariant SkuFoundTableModel::data(
        const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const QString &sku = m_skus[index.row()];
        return m_colInfos[index.column()].function(sku, m_skuToShipmentRefunds[sku]);
        /*
        if (index.column() == 0) {
            return m_skus[index.row()];
        }
        //return m_skusDomainsTitles[m_skus[index.row()]][m_colNameToColIndex[index.column()]];
        return m_listOfVariantList[index.row()][index.column()];
//*/
    }
    return QVariant();
}
//----------------------------------------------------------
