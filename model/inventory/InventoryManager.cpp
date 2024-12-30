#include <QtCore/qdatetime.h>
#include <QtCore/QTextStream>

#include "../common/utils/CsvReader.h"
#include "../common/currencies/CurrencyRateManager.h"

#include "model/SettingManager.h"
#include "InventoryManager.h"
#include "ManagerBundle.h"
#include "ManagerInventoryIssues.h"
#include "model/CustomerManager.h"

QString InventoryManager::COL_UNIT_PRICE = QObject::tr("Prix unitaire");
QString InventoryManager::COL_WEIGHT_GR = QObject::tr("Poids en gr");
QString InventoryManager::COL_UNIT_LEFT = QObject::tr("Unité restante");
QString InventoryManager::COL_UNIT_TOTAL = QObject::tr("Unité restante total");
QString InventoryManager::COL_UNIT_BEGIN = QObject::tr("Unité début");
QString InventoryManager::COL_TITLES = QObject::tr("Titres");
QString InventoryManager::COL_CODE = QObject::tr("Code");
QString InventoryManager::COL_CODES_OTHER = QObject::tr("Autres codes");
QString InventoryManager::COL_CODE_PARENT = QObject::tr("Parent");
QString InventoryManager::COL_DATE = QObject::tr("Date");
QString InventoryManager::COL_ORDER = QObject::tr("Commande");
QString InventoryManager::COL_LEFT_MONTHS = QObject::tr("Mois restant");
QString InventoryManager::COL_SALES_365J = QObject::tr("Ventes 365j");
QString InventoryManager::COL_CURRENCY = QObject::tr("Monnaie");
//----------------------------------------------------------
InventoryManager::InventoryManager(QObject *parent)
    : QAbstractTableModel(parent), UpdateToCustomer ()
{
}
//----------------------------------------------------------
void InventoryManager::load(int year)
{
    _clear();
    for (auto fileInfo : SettingManager::instance()
         ->dirInventoryMergedCodes().entryInfoList(
             QStringList() << "*.csv", QDir::Files)) {
        _addMergingCodeFile(fileInfo.absoluteFilePath());
    }
    for (auto fileInfo : SettingManager::instance()
         ->dirInventoryBegin(year).entryInfoList(
             QStringList() << "*.csv", QDir::Files)) {
        _addInventoryBeginFile(fileInfo.absoluteFilePath(), year);
    }
    for (auto fileInfo : SettingManager::instance()
         ->dirInventoryPurchase(year).entryInfoList(
             QStringList() << "*.csv", QDir::Files)) {
        QString dateString = fileInfo.fileName().split("__")[0];
        QDate date = QDate::fromString(dateString, "yyyy-MM-dd");
        _addPurchaseOrInventoryFile(fileInfo.absoluteFilePath(), date);
    }
    for (auto fileInfo : SettingManager::instance()
         ->dirInventoryAmzReturns(year).entryInfoList(
             QStringList() << "*.csv", QDir::Files)) {
        QString dateString = fileInfo.fileName().split("__")[0];
        QDate date = QDate::fromString(dateString, "yyyy-MM-dd");
        _addAmazonReturnFile(fileInfo.absoluteFilePath(), date);
    }
}
//----------------------------------------------------------
void InventoryManager::_clear()
{
    if (m_valuesTable.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_valuesTable.size()-1);
        m_valuesTable.clear();
        m_parentToMainCodes.clear();
        m_mainCodeToOtherInfos.clear();
        m_otherCodeToMain.clear();
        m_totals.clear();
        m_inventoryBeginYear.clear();
        m_purchases.clear();
        endRemoveRows();
    }
}
//----------------------------------------------------------
void InventoryManager::_addMergingCodeFile(const QString &filePath)
{
    CsvReader csvReader(filePath, "\t");
    QList<QString> mainCodes;
    QList<QStringList> allOtherCodes;
    if (csvReader.readAll()) {
        auto dataRode = csvReader.dataRode();
        if (dataRode->header.contains("Merchant SKU")
                && dataRode->header.contains("ASIN")
                && dataRode->header.contains("FNSKU")) { /// FBA shipment sku list
            int posSKU = dataRode->header.pos("Merchant SKU");
            int posASIN = dataRode->header.pos("ASIN");
            int posFNSKU = dataRode->header.pos("FNSKU");
            for (auto line: dataRode->lines) {
                QString sku = line[posSKU].trimmed();
                QString asin = line[posASIN].trimmed();
                QString fnsku = line[posFNSKU].trimmed();
                mainCodes << sku;
                allOtherCodes << QStringList({asin, fnsku});
            }
        } else if (dataRode->header.contains("(Parent) ASIN")
                   && dataRode->header.contains("(Child) ASIN")
                   && dataRode->header.contains("SKU")) { /// Business report
            int posParent = dataRode->header.pos("(Parent) ASIN");
            int posAsin = dataRode->header.pos("(Child) ASIN");
            int posSKU = dataRode->header.pos("SKU");
            for (auto line: dataRode->lines) {
                QString parent = line[posParent].trimmed();
                QString asin = line[posAsin].trimmed();
                QString sku = line[posSKU].trimmed();
                mainCodes << sku;
                allOtherCodes << QStringList({asin});
                if (!m_parentToMainCodes.contains(parent)) {
                    m_parentToMainCodes[parent] = QSet<QString>();
                }
                m_parentToMainCodes[parent] << sku;
            }
        } else if ((dataRode->header.contains("SKU") || dataRode->header.contains("sku"))
                   && (dataRode->header.contains("CODE1") || dataRode->header.contains("code1"))) {
            int posSKU = dataRode->header.pos({"SKU", "sku"});
            QList<int> posOtherCodes;
            for (auto header : dataRode->header.getHeaderElements()) {
                if (header.toLower().startsWith("code")) {
                    posOtherCodes << dataRode->header.pos(header);
                }
            }
            for (auto elements : dataRode->lines) {
                QString sku = elements[posSKU].trimmed();
                mainCodes << sku;
                QStringList codes;
                for (auto ind : posOtherCodes) {
                    auto otherCode = elements[ind].trimmed();
                    if (!otherCode.isEmpty()) {
                        codes << otherCode;
                    }
                }
                allOtherCodes << codes;
            }
        }
        for (int i=0; i<mainCodes.size(); ++i) {
            mergeCodes(mainCodes[i], allOtherCodes[i]);
        }
    }
}
//----------------------------------------------------------
void InventoryManager::_addInventoryBeginFile(
        const QString &filePath, int year)
{
    QDate date(year, 1, 1);
    _addPurchaseOrInventoryFile(filePath, date, false);
}
//----------------------------------------------------------
void InventoryManager::_addPurchaseFile(const QString &filePath, const QDate &date)
{
    _addPurchaseOrInventoryFile(filePath, date, true);
}
//----------------------------------------------------------
void InventoryManager::_addPurchaseOrInventoryFile(
        const QString &filePath, const QDate &date, bool purchase)
{
    QString fileName = QFileInfo(filePath).fileName();
    int year = date.year();
    if (!m_totals.contains(year)) {
        m_totals[year] = QHash<QString, InfoTotals>();
    }
    CsvReader csvReader(filePath, "\t");
    if (csvReader.readAll()) {
        if (!m_inventoryBeginYear.contains(year)) {
            m_inventoryBeginYear[year] = QMap<QString, QList<Infos>>();
        }
        auto dataRode = csvReader.dataRode();
        int posOrderName = -1;
        if (purchase || dataRode->header.contains("order-number")) {
            posOrderName = dataRode->header.pos("order-number");
        }
        int posSku = dataRode->header.pos({"sku", "SKU", InventoryManager::COL_CODE});
        int posProductName = dataRode->header.pos({"product-name", InventoryManager::COL_TITLES});
        int posQuantity = dataRode->header.pos({"quantity", InventoryManager::COL_UNIT_LEFT}); //TODO use translate code names
        int posWeight = -1;
        if (dataRode->header.contains("Unit-weight")
                || dataRode->header.contains("unit-weight")
                || dataRode->header.contains(InventoryManager::COL_WEIGHT_GR)
                ) {
            posWeight = dataRode->header.pos({"Unit-weight", "unit-weight", InventoryManager::COL_WEIGHT_GR});
        }
        int posPriceUnit = dataRode->header.pos({"price", InventoryManager::COL_UNIT_PRICE});
        int posCurrency = dataRode->header.pos({"currency", InventoryManager::COL_CURRENCY});
        bool codeEmptyFound = false;
        for (auto elements : dataRode->lines) {
            QString code = elements[posSku].trimmed();
            QString title = elements[posProductName].trimmed();
            Q_ASSERT(!code.isEmpty() || !title.isEmpty());
            if (codeEmptyFound) {
                Q_ASSERT(code.isEmpty()); /// Once we found an empty code, all other codes should be empty
            }
            if (code.isEmpty()) {
                codeEmptyFound = true;
            } else {
                QString mainCode1 = m_otherCodeToMain.value(code, code);
                if (mainCode1 == "GLAZ-CC-SETA") {
                    int TEMP=10;++TEMP;
                }
                QStringList mainCodes = {};
                QList<int> codesUnits = {};
                int totalBundleUnits = 0;
                //if (mainCode1.contains("747150648088")) {
                if (ManagerBundle::instance()->isBundle(mainCode1)) {
                    auto codeUnits = ManagerBundle::instance()->codesBase(mainCode1);
                    for (auto codeUnit : qAsConst(codeUnits)) {
                        totalBundleUnits += codeUnit.second;
                        mainCodes << m_otherCodeToMain.value(
                                         codeUnit.first, codeUnit.first);
                        codesUnits << codeUnit.second;
                    }
                } else {
                    totalBundleUnits = 1;
                    m_mainCodeToOtherInfos[mainCode1].titles << title;
                    mainCodes.append(mainCode1);
                    codesUnits.append(1);
                }
                Q_ASSERT(!mainCode1.isEmpty());
                int origUnit = elements[posQuantity].toInt();
                int origWeightGr = 0;
                if (posWeight >= 0) {
                    origWeightGr = elements[posWeight].toInt();
                }
                double origUnitPrice = elements[posPriceUnit].toDouble();
                QString order;
                if (purchase || posOrderName > -1) {
                    order = elements[posOrderName];
                } else {
                    order = "INVENTORY-" + QString::number(year) + "-01-01-ALL";
                }
                QString currency = elements[posCurrency];
                if (currency != CustomerManager::instance()->getSelectedCustomerCurrency()) {
                    double rate = CurrencyRateManager::instance()->rate(
                                currency,
                                CustomerManager::instance()->getSelectedCustomerCurrency(),
                                date);
                    origUnitPrice *= rate;
                }
                for (int i=0; i<mainCodes.size(); ++i) {
                    // TODO handle bundle
                    QString mainCode = mainCodes[i].trimmed();
                    if (mainCode.isEmpty() || mainCode.toLower() != "shipping") {
                        int unitInBundle = codesUnits[i];
                        Infos infos;
                        infos.unitsPurchased.units = origUnit * unitInBundle;
                        infos.unitsPurchased.weightGr = origWeightGr / totalBundleUnits;
                        infos.unitsPurchased.priceUnit = origUnitPrice / totalBundleUnits;
                        Q_ASSERT(infos.unitsPurchased.priceUnit < 100.);
                        //Q_ASSERT(mainCode.trimmed().toLower() != "shipping");
                        infos.orderName = order;
                        if (!m_totals[year].contains(mainCode)) {
                            m_totals[year][mainCode] = InfoTotals();
                        }
                        if (purchase) {
                            if (!m_purchases[year].contains(mainCode)) {
                                m_purchases[year][mainCode] = QMultiMap<QDate, Infos>();
                            }
                            m_purchases[year][mainCode].insert(date, infos);
                        } else {
                            if (!m_inventoryBeginYear[year].contains(mainCode)) {
                                m_inventoryBeginYear[year][mainCode] = QList<Infos>();
                            }
                            m_inventoryBeginYear[year][mainCode] << infos;
                        }
                        m_totals[year][mainCode].unitsPurchased += infos.unitsPurchased.units;
                    }
                }
            }
        }
    }
}
//----------------------------------------------------------
void InventoryManager::_addAmazonReturnFile(
        const QString &filePath, const QDate &date)
{
    CsvReader csvReader(filePath, ",", "\"");
    if (csvReader.readAll()) {
        int year = date.year();
        auto dataRode = csvReader.dataRode();
        int posSku = dataRode->header.pos("MSKU");
        int posTitle = dataRode->header.pos("Title");
        //int posTitle = dataRode->header.pos("Title");
        int posReturns = dataRode->header.pos("Customer Returns");
        for (auto elements : dataRode->lines) {
            QString code = elements[posSku];
            QString title = elements[posTitle];
            QString mainCode1 = m_otherCodeToMain.value(code, code);
            int unitReturned = elements[posReturns].toInt();
            QStringList mainCodes = {};
            QList<int> codesUnits = {};
            int totalBundleUnits = 0;
            if (ManagerBundle::instance()->isBundle(mainCode1)) {
                auto codeUnits = ManagerBundle::instance()->codesBase(mainCode1);
                for (auto codeUnit : qAsConst(codeUnits)) {
                    totalBundleUnits += codeUnit.second;
                    mainCodes << m_otherCodeToMain.value(
                                     codeUnit.first, codeUnit.first);
                    codesUnits << codeUnit.second;
                }
            } else {
                totalBundleUnits = 1;
                m_mainCodeToOtherInfos[mainCode1].titles << title;
                mainCodes.append(mainCode1);
                codesUnits.append(1);
            }
            if (m_inventoryBeginYear.contains(year)) {
                for (int i=0; i<mainCodes.size(); ++i) {
                    int unitReturnedBundle = unitReturned * codesUnits[i];
                    QString mainCode = mainCodes[i];
                    m_totals[year][mainCode].unitsReturned += unitReturnedBundle;
                    if (m_inventoryBeginYear[year].contains(mainCode)) {
                        auto itList = m_inventoryBeginYear[year][mainCode].begin();
                        itList->unitsReturned.insert(date, unitReturned);
                    } else if (m_purchases.contains(year)
                               && m_purchases[year].contains(mainCode)) {
                        auto itInfo = m_purchases[year][mainCode].begin();
                        itInfo.value().unitsReturned.insert(date, unitReturnedBundle);
                    } else {
                        ManagerInventoryIssues::instance()->record(
                                    mainCode, "", ManagerInventoryIssues::UNKWOWN, unitReturnedBundle);
                        //Q_ASSERT(false);
                    }
                }
            }
        }
    }
}
//----------------------------------------------------------
QList<InventoryManager::ColInfo> *InventoryManager::_colInfos() const
{
    //static QStringList titles = {tr("Parent"), tr("Code"), tr("Autre codes"),
    //tr("titres"), tr("Unité"), tr("prix moyen"), tr("Unité total"),
    //tr("Ventes 365 jours"), tr("Ventes 3 mois")};
    static QList<InventoryManager::ColInfo> colInfos
            = {{COL_CODE_PARENT, [](
                const InventoryManager *manager, const QString &code, const QDate &,
                const Infos &, const InfoTotals &) -> QString {
                    return manager->m_mainCodeToOtherInfos.value(code, InfoTitles()).parent;
                }}
               ,{COL_CODE, [](
                const InventoryManager *, const QString &code, const QDate &,
                const Infos &, const InfoTotals &) -> QString{
                     return code;
                }}
               ,{COL_CODES_OTHER, [](
                const InventoryManager *manager, const QString &code, const QDate &,
                const Infos &, const InfoTotals &) -> QString{
                    return manager->m_mainCodeToOtherInfos.value(
                     code, InfoTitles()).otherCodes.values().join(" ");
                }}
               ,{COL_TITLES, [](
                const InventoryManager *manager, const QString &code, const QDate &,
                const Infos &, const InfoTotals &) -> QString{
                    return manager->m_mainCodeToOtherInfos.value(
                     code, InfoTitles()).titles.values().join(" ");
                }}
               ,{COL_DATE, [](
                const InventoryManager *, const QString &, const QDate &date,
                const Infos &, const InfoTotals &) -> QString{
                     return date.toString("yyyy-MM-dd");
                }}
               ,{COL_UNIT_TOTAL, [](
                const InventoryManager *manager, const QString &code, const QDate &date,
                const Infos &, const InfoTotals &) -> QString{
                     if (manager->m_totals.contains(date.year())) {
                         int left = manager->m_totals[date.year()].value(
                         code, InfoTotals()).left();
                         return QString::number(left);
                     }
                     return QString();
                }}
               ,{COL_UNIT_BEGIN, [](
                const InventoryManager *, const QString &, const QDate &,
                const Infos &infos, const InfoTotals &) -> QString{
                     return QString::number(infos.unitsPurchased.units);
                }}
               ,{COL_UNIT_PRICE, [](
                const InventoryManager *, const QString &, const QDate &,
                const Infos &infos, const InfoTotals &) -> QString{
                     return QString::number(infos.unitsPurchased.priceUnit);
                }}
               ,{COL_UNIT_LEFT, [](
                //const InventoryManager *manager, const QString &code, const QDate &date,
                //const Infos &infos, const InfoTotals &totals) -> QString{
                const InventoryManager *, const QString &, const QDate &,
                const Infos &infos, const InfoTotals &) -> QString{
                     return QString::number(infos.left());
                }}
               ,{COL_LEFT_MONTHS, [](
                const InventoryManager *manager, const QString &code, const QDate &date,
                const Infos &, const InfoTotals &) -> QString{
                     int sale365 = manager->salesCount(code, 365);
                     int sale30 = manager->salesCount(code, 30);
                     int sale90 = manager->salesCount(code, 90);
                     int left = 0;
                     if (manager->m_totals.contains(date.year())) {
                         left = manager->m_totals[date.year()].value(
                                        code, InfoTotals()).left();
                     }
                     QList<double> ratios;
                     ratios << (sale30 == 0 ? 12. :left / (sale30 / 30.) / 30.);
                     ratios << (sale90 == 0 ? 12. :left / (sale90 / 90.) / 30.);
                     ratios << (sale365 == 0 ? 12. :left / (sale365 / 365.) / 30.);
                     std::sort(ratios.begin(), ratios.end());
                     return QString::number(ratios[0], 'f', 2);
                }}
               ,{tr("Ventes 30j"), [](
                const InventoryManager *manager, const QString &code, const QDate &,
                const Infos &, const InfoTotals &) -> QString{
                     return QString::number(manager->salesCount(code, 30));
                }}
               ,{tr("Ventes 30j parent"), [](
                const InventoryManager *manager, const QString &code, const QDate &,
                const Infos &, const InfoTotals &) -> QString{
                     QString parent =  manager->m_mainCodeToOtherInfos.value(
                     code, InfoTitles()).parent;
                     if (!parent.isEmpty()) {
                         return QString::number(manager->salesCountParent(parent, 30));
                     }
                     return QString();
                }}
               ,{tr("Ventes 90j"), [](
                const InventoryManager *manager, const QString &code, const QDate &,
                const Infos &, const InfoTotals &) -> QString{
                     return QString::number(manager->salesCount(code, 90));
                }}
               ,{tr("Ventes 90j parent"), [](
                const InventoryManager *manager, const QString &code, const QDate &,
                const Infos &, const InfoTotals &) -> QString{
                     QString parent =  manager->m_mainCodeToOtherInfos.value(
                     code, InfoTitles()).parent;
                     if (!parent.isEmpty()) {
                         return QString::number(manager->salesCountParent(parent, 90));
                     }
                     return QString();
                }}
               ,{COL_SALES_365J, [](
                const InventoryManager *manager, const QString &code, const QDate &,
                const Infos &, const InfoTotals &) -> QString{
                     return QString::number(manager->salesCount(code, 365));
                }}
               ,{tr("Ventes 365j parent"), [](
                const InventoryManager *manager, const QString &code, const QDate &,
                const Infos &, const InfoTotals &) -> QString{
                     QString parent =  manager->m_mainCodeToOtherInfos.value(
                     code, InfoTitles()).parent;
                     if (!parent.isEmpty()) {
                         return QString::number(manager->salesCountParent(parent, 365));
                     }
                     return QString();
                }}
               ,{COL_WEIGHT_GR, [](
                const InventoryManager *, const QString &, const QDate &,
                const Infos &infos, const InfoTotals &) -> QString{
                     return QString::number(infos.unitsPurchased.weightGr);
                }}
               ,{COL_ORDER, [](
                const InventoryManager *, const QString &, const QDate &,
                const Infos &infos, const InfoTotals &) -> QString{
                     return infos.orderName;
                }}
              };
    return &colInfos;
}
//----------------------------------------------------------
InventoryManager *InventoryManager::instance()
{
    static InventoryManager instance;
    return &instance;
}
//----------------------------------------------------------
InventoryManager::~InventoryManager()
{
}
//----------------------------------------------------------
QStringList InventoryManager::getInventoryFilePaths(const QList<int> &years) const
{
    QStringList filePaths;
    for (const auto &year : years)
    {
        auto dirPurchase = SettingManager::instance()
                               ->dirInventoryPurchase(year);
        filePaths << dirPurchase.entryList(
            QStringList() << "*.csv", QDir::Files);
    }
    return filePaths;
}
//----------------------------------------------------------
QString InventoryManager::uniqueId() const
{
    return "InventoryManager";
}
//----------------------------------------------------------
void InventoryManager::onCustomerSelectedChanged(
        const QString &)
{
    //loadFromSettings();
    _clear();
}
//----------------------------------------------------------
double InventoryManager::inventoryValue() const
{
    double total = 0.;
    // TODO for colInfo with titles as static to avoid duplicate while translating
    int indUnitPrice = indexColumn(COL_UNIT_PRICE);
    int indQuantity = indexColumn(COL_UNIT_LEFT);
    for (auto it = m_valuesTable.begin();
         it != m_valuesTable.end(); ++it) {
        int units = it->value(indQuantity).toInt();
        double unitPrice = it->value(indUnitPrice).toDouble();
        total += unitPrice * units;
    }
    return total;
}
//----------------------------------------------------------
QString InventoryManager::mainCode(const QString &code) const
{
    QString mainCode = m_otherCodeToMain.value(code, code);
    return mainCode;
}
//----------------------------------------------------------
int InventoryManager::indexColumn(const QString &title) const
{
    int i = 0;
    for (auto info : *_colInfos()) {
        if (info.name == title) {
            return i;
        }
        ++i;
    }
    return -1;
}
//----------------------------------------------------------
void InventoryManager::exportInventoryUnsold(const QString &filePath)
{
    int indDate = indexColumn(COL_DATE);
    int indOrder = indexColumn(COL_ORDER);
    int indCode = indexColumn(COL_CODE);
    int indCodesOThers = indexColumn(COL_CODES_OTHER);
    int indQuantityLeft = indexColumn(COL_UNIT_LEFT);
    int indQuantityBegin = indexColumn(COL_UNIT_BEGIN);
    int indUnitPrice = indexColumn(COL_UNIT_PRICE);
    int indTitles = indexColumn(COL_TITLES);
    int indWeight = indexColumn(COL_WEIGHT_GR);
    QStringList linesToSave;
    linesToSave << QStringList({
                                   "order-number"
                                   , "product-name"
                                   , "sku"
                                   , "quantity"
                                   , "unit-weight"
                                   , "price"
                                   , "total-price"
                                   , "currency"}).join("\t");
    QList<QStringList> linesElements;
    QSet<QString> codesSold;
    for (auto elements : m_valuesTable) {
        int unitsLeft = elements[indQuantityLeft].toInt();
        if (unitsLeft > 0) {
            int unitBegin = elements[indQuantityBegin].toInt();
            QString dateString = elements[indDate];
            QString code = elements[indCode];
            if (dateString.endsWith("01-01")
                    && unitsLeft == unitBegin) {
                //int unitSold = unitBegin - unitsLeft;
                QString unitPrice = elements[indUnitPrice];
                Q_ASSERT(unitPrice.toDouble() < 50.);
                double totalValue = unitsLeft * unitPrice.toDouble();
                //QDate date = QDate::fromString(elements[indDate], "yyyy-MM-dd");
                QString titles = elements[indTitles];
                QString order = elements[indOrder];
                QString codesOthers = elements[indCodesOThers];
                QString weight = elements[indWeight];
                QStringList line;
                line << "";
                line << titles;
                line << code;
                line << elements[indQuantityBegin];
                line << weight;
                line << unitPrice;
                line << QString::number(totalValue, 'f', 2);
                line << "EUR";
                linesElements << line;
                //linesToSave << line.join("\t");
            } else {
                codesSold << code;
            }
        }
    }
    for (auto itElements = linesElements.begin();
         itElements != linesElements.end(); ++itElements) {
        QString code = itElements->value(2);
        if (!codesSold.contains(code)) {
            linesToSave << itElements->join("\t");
        }
    }
    QFile file(filePath);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream << linesToSave.join("\n");
        file.close();
    }
}
//----------------------------------------------------------
void InventoryManager::exportInventory(
        const QString &filePath)
{
    int indDate = indexColumn(COL_DATE);
    int indOrder = indexColumn(COL_ORDER);
    int indCode = indexColumn(COL_CODE);
    int indCodesOThers = indexColumn(COL_CODES_OTHER);
    int indQuantityLeft = indexColumn(COL_UNIT_LEFT);
    int indQuantityBegin = indexColumn(COL_UNIT_BEGIN);
    int indUnitPrice = indexColumn(COL_UNIT_PRICE);
    int indTitles = indexColumn(COL_TITLES);
    int indWeight = indexColumn(COL_WEIGHT_GR);
    QMultiMap<QString, QString> linesOrdered;
    QStringList elementsHeader
            = {COL_CODE, COL_DATE, COL_UNIT_PRICE
              , COL_UNIT_BEGIN, COL_UNIT_LEFT
              , tr("Valeur total")
              , COL_ORDER
              , COL_TITLES
              , COL_CODES_OTHER
              , COL_WEIGHT_GR
              , COL_CURRENCY
              };
    linesOrdered.insert("", elementsHeader.join("\t"));
    QString currencyApp = CustomerManager::instance()->getSelectedCustomerCurrency();
    for (auto elements : m_valuesTable) {
        int unitsLeft = elements[indQuantityLeft].toInt();
        if (unitsLeft > 0) {
            int unitBegin = elements[indQuantityBegin].toInt();
            //int unitSold = unitBegin - unitsLeft;
            QString unitPrice = elements[indUnitPrice];
            Q_ASSERT(unitPrice.toDouble() < 50.);
            double totalValue = unitsLeft * unitPrice.toDouble();
            QString dateString = elements[indDate];
            //QDate date = QDate::fromString(elements[indDate], "yyyy-MM-dd");
            QString code = elements[indCode];
            QString titles = elements[indTitles];
            QString order = elements[indOrder];
            QString codesOthers = elements[indCodesOThers];
            QString weight = elements[indWeight];
            QStringList elementsLines
                    = {code
                       , dateString
                       , unitPrice
                       , QString::number(unitBegin)
                       , QString::number(unitsLeft)
                       , QString::number(totalValue, 'f', 2)
                       , order
                       , titles
                       , codesOthers
                       , weight
                       , currencyApp
                      };
            linesOrdered.insert(code, elementsLines.join("\t"));
        }
    }
    QFile file(filePath);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream << linesOrdered.values().join("\n");
        file.close();
    }
}
//----------------------------------------------------------
void InventoryManager::addMergingCodeFile(const QString filePath)
{
    QDir dir = SettingManager::instance()->dirInventoryMergedCodes();
    QFileInfo fileInfo(filePath);
    QString newFilePath = dir.filePath(fileInfo.fileName());
    QFile::copy(filePath, newFilePath);
    _addMergingCodeFile(newFilePath);
}
//----------------------------------------------------------
void InventoryManager::addInventoryBeginFile(
        const QString filePath, int year)
{
    QDir dir = SettingManager::instance()->dirInventoryBegin(year);
    QFileInfo fileInfo(filePath);
    _addInventoryBeginFile(filePath, year);
    QString newFilePath = dir.filePath(fileInfo.fileName());
    QFile::copy(filePath, newFilePath);
}
//----------------------------------------------------------
void InventoryManager::addPurchaseFile(
        const QString filePath, const QDate &date)
{
    QDir dir = SettingManager::instance()->dirInventoryPurchase(
                date.year());
    QFileInfo fileInfo(filePath);
    QString currentFileName = fileInfo.fileName();
    QString dateString = date.toString("yyyy-MM-dd__");
    if (!currentFileName.startsWith(dateString)) {
        currentFileName = dateString + currentFileName;
    }
    QString newFilePath = dir.filePath(currentFileName);
    QFile::copy(filePath, newFilePath);
    _addPurchaseOrInventoryFile(newFilePath, date);
}
//----------------------------------------------------------
void InventoryManager::addAmazonReturnFile(
        const QString filePath, const QDate &date)
{
    QDir dir = SettingManager::instance()->dirInventoryAmzReturns(
                date.year());
    QFileInfo fileInfo(filePath);
    QString currentFileName = fileInfo.fileName();
    QString dateString = date.toString("yyyy-MM-dd__");
    if (!currentFileName.startsWith(dateString)) {
        currentFileName = dateString + currentFileName;
    }
    QString newFilePath = dir.filePath(currentFileName);
    QFile::copy(filePath, newFilePath);
    _addAmazonReturnFile(newFilePath, date);
}
//----------------------------------------------------------
void InventoryManager::recordMovement(
        const QString &code, const QString &title, int unit, const QDate &date)
{
    if (code == "SHOE-INSERTS-BEIGE-X5") {
        int TEMP=10;++TEMP;
    }
    QString mainCode = m_otherCodeToMain.value(code, code);
    //if (!m_mainCodeToOtherInfos.contains(mainCode) && m_otherCodeToMain.contains(mainCode)) {
        //recordMovement(m_otherCodeToMain[mainCode], title, unit, date);
    //} else if (ManagerBundle::instance()->isBundle(mainCode)) {
    if (ManagerBundle::instance()->isBundle(mainCode)) {
        auto codeUnits = ManagerBundle::instance()->codesBase(mainCode);
        for (auto codeUnit : codeUnits) {
            recordMovement(codeUnit.first, title, codeUnit.second, date);
        }
    } else {
        // TODO rec or change code if needed
        int year = date.year();
        bool codefound = false;
        if (m_totals.contains(year)
                && m_totals[year].contains(mainCode)) {
            m_totals[year][mainCode].unitsSold += unit;
            codefound = true;
        }
        m_mainCodeToOtherInfos[mainCode].titles << title;
        if (m_inventoryBeginYear.contains(year)) {
            if (m_inventoryBeginYear[year].contains(mainCode)) {
                codefound = true;
                for(auto itList = m_inventoryBeginYear[year][mainCode].begin();
                    itList != m_inventoryBeginYear[year][mainCode].end();
                    ++itList) {
                    if (itList->left() > 0) {
                        itList->unitsSold.insert(date, unit);
                        return;
                    }
                }
            }
        }
        if (m_purchases.contains(year)
                && m_purchases[year].contains(mainCode)) {
            codefound = true;
            for(auto itInfo = m_purchases[year][mainCode].begin();
                itInfo != m_purchases[year][mainCode].end();
                ++itInfo) {
                if (itInfo.value().left() > 0) {
                    itInfo.value().unitsSold.insert(date, unit);
                    return;
                }
            }
        }
        QString type = ManagerInventoryIssues::UNKWOWN;
        if (codefound) {
            type = ManagerInventoryIssues::UNAVAILABLE;
        }
        ManagerInventoryIssues::instance()->record( mainCode, title, type, unit);
    }
}
//----------------------------------------------------------
double InventoryManager::valueUnitAverage(const QString &sku) const
{
    QList<QtyPrice> totalUnitPrices;
    QString mainCode = m_otherCodeToMain.value(sku, sku);
    for (auto itYear = m_inventoryBeginYear.begin();
         itYear != m_inventoryBeginYear.end(); ++itYear) {
        if (itYear.value().contains(mainCode)) {
            for (auto info : itYear.value()[mainCode]) {
                totalUnitPrices << info.unitsPurchased;
            }
        }
    }
    for (auto itYear = m_purchases.begin();
         itYear != m_purchases.end(); ++itYear) {
        if (itYear.value().contains(mainCode)) {
            for (auto info : itYear.value()[mainCode].values()) {
                totalUnitPrices << info.unitsPurchased;
            }
        }
    }
    double totalValue = 0.;
    int totalUnits = 0;
    for (auto info : totalUnitPrices) {
        totalUnits += info.units;
        totalValue += info.priceUnit * info.units;
    }
    if (totalUnits == 0) {
        return 0.;
    }
    double avgUnitValue = totalValue / totalUnits;
    return avgUnitValue;
}
//----------------------------------------------------------
void InventoryManager::clearMovementsBeforeRecording()
{
    QMultiMap<QString, Infos> infos;
    for (auto itYear = m_inventoryBeginYear.begin();
         itYear != m_inventoryBeginYear.end(); ++itYear) {
        for (auto itCode = itYear.value().begin();
             itCode != itYear.value().end(); ++itCode) {
            if (m_totals[itYear.key()].contains(itCode.key())) {
                m_totals[itYear.key()][itCode.key()].unitsSold = 0;
            }
            for (auto itList = itCode.value().begin();
                 itList != itCode.value().end(); ++itList) {
                itList->unitsSold.clear();
            }
        }
    }
    for (auto itYear = m_purchases.begin();
         itYear != m_purchases.end(); ++itYear) {
        for (auto itCode = itYear.value().begin();
             itCode != itYear.value().end(); ++itCode) {
            if (m_totals[itYear.key()].contains(itCode.key())) {
                m_totals[itYear.key()][itCode.key()].unitsSold = 0;
            }
            for (auto itInfo = itCode.value().begin();
                 itInfo != itCode.value().end(); ++itInfo) {
                itInfo.value().unitsSold.clear();
            }
        }
    }
}
//----------------------------------------------------------
void InventoryManager::refresh(int year)
{
    if (m_valuesTable.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_valuesTable.size() - 1);
        m_valuesTable.clear();
        endRemoveRows();
    }
    auto colInfos = _colInfos();
    int nCols = colInfos->size();
    bool purchasesDone = m_purchases.contains(year);
    QSet<QString> codesDone;
    if (m_inventoryBeginYear.contains(year)) {
        for (auto itInv = m_inventoryBeginYear[year].begin();
             itInv != m_inventoryBeginYear[year].end(); ++itInv) {
            QDate dateBegin(year, 1, 1);
            QString code = itInv.key();
            auto infoTotals = m_totals[year][code];
            for (auto itList = itInv.value().begin();
                 itList != itInv.value().end(); ++itList) {
                QStringList elements;
                for (int col = 0; col < nCols; ++col) {
                    elements << colInfos->value(col).getValue(
                                this, code, dateBegin, *itList, infoTotals);
                }
                m_valuesTable << elements;
            }
            if (purchasesDone && m_purchases[year].contains(code)) {
                for (auto itPur = m_purchases[year][code].begin();
                     itPur != m_purchases[year][code].end(); ++itPur) {
                    QStringList elements;
                    for (int col = 0; col < nCols; ++col) {
                        elements << colInfos->value(col).getValue(
                                        this, code, itPur.key(), itPur.value(), infoTotals);
                    }
                    m_valuesTable << elements;
                }
            }
            codesDone << code;
        }
    }
    if (m_purchases.contains(year)) {
        for (auto itInv = m_purchases[year].begin();
             itInv != m_purchases[year].end(); ++itInv) {
            QString code = itInv.key();
            if (!codesDone.contains(code)) {
                auto infoTotals = m_totals[year][code];
                for (auto itPur = m_purchases[year][code].begin();
                     itPur != m_purchases[year][code].end(); ++itPur) {
                    QStringList elements;
                    for (int col = 0; col < nCols; ++col) {
                        elements << colInfos->value(col).getValue(
                                        this, code, itPur.key(), itPur.value(), infoTotals);
                    }
                    m_valuesTable << elements;
                }
            }
        }
    }
    if (m_valuesTable.size() > 0) {
        beginInsertRows(QModelIndex(), 0, m_valuesTable.size() - 1);
        endInsertRows();
    }
}
//----------------------------------------------------------
int InventoryManager::salesCount(const QString &code, int nDays) const
{
    QDate today = QDate::currentDate(); //TODO end in previous month
    //QDate today = QDate(2021,12,31);
    int nSold = 0;
    for (auto itYear = m_inventoryBeginYear.begin();
         itYear != m_inventoryBeginYear.end(); ++itYear) {
        if (itYear.value().contains(code)) {
            for (auto itList = itYear.value()[code].begin();
                 itList != itYear.value()[code].end(); ++itList) {
                for (auto itSold = itList->unitsSold.begin(); // TODO I should deduct returned units
                     itSold != itList->unitsSold.end();
                     ++itSold) {
                    if (itSold.key().daysTo(today) <= nDays) {
                        nSold += itSold.value();
                    }
                }
            }
        }
    }
    for (auto itYear = m_purchases.begin();
         itYear != m_purchases.end(); ++itYear) {
        if (itYear.value().contains(code)) {
            for (auto itInfo = itYear.value()[code].begin();
                 itInfo != itYear.value()[code].end(); ++itInfo) {
                for (auto itSold = itInfo.value().unitsSold.begin();
                     itSold != itInfo.value().unitsSold.end();
                     ++itSold) {
                    if (itSold.key().daysTo(today) <= nDays) {
                        nSold += itSold.value();
                    }
                }
            }
        }
    }
    return nSold;
}
//----------------------------------------------------------
int InventoryManager::salesCountParent(const QString &codeParent, int nDays) const
{
    int nSales = 0;
    if (m_parentToMainCodes.contains(codeParent)) {
        for (auto code = m_parentToMainCodes[codeParent].begin();
             code != m_parentToMainCodes[codeParent].end(); ++code) {
            int currentSales = salesCount(*code, nDays);
            nSales += currentSales;
        }
    }
    return nSales;
}
//----------------------------------------------------------
void InventoryManager::mergeCodes(const QModelIndexList &indexes)
{
    QStringList otherCodes;
    for (int i=0; i<indexes.size(); ++i) {
        otherCodes << m_valuesTable[indexes[i].row()][1];
    }
    QString mainCode = otherCodes.takeFirst();
    mergeCodes(mainCode, otherCodes);
}
//----------------------------------------------------------
void InventoryManager::mergeCodes(
        const QString &mainCode, const QStringList &otherCodes)
{
    Q_ASSERT(!mainCode.isNull());
    if (!m_mainCodeToOtherInfos.contains(mainCode)) {
        m_mainCodeToOtherInfos[mainCode] = InfoTitles();
    }
    QStringList otherCodesInMain;
    for (auto code : qAsConst(otherCodes)) {
        if (m_mainCodeToOtherInfos.contains(code)) {
            otherCodesInMain << code;
        }
    }
    if (otherCodesInMain.size() > 0) {
        for (auto otherCode : otherCodesInMain) {
            QString parent = m_mainCodeToOtherInfos[otherCode].parent;
            if (!parent.isEmpty()) {
                m_parentToMainCodes[parent].remove(otherCode);
                m_parentToMainCodes[parent] << mainCode;
            }
            m_mainCodeToOtherInfos.remove(otherCode);
            for (auto itYear = m_totals.begin(); itYear != m_totals.end(); ++itYear) {
                if (itYear.value().contains(otherCode)) {
                    if (!m_totals[itYear.key()].contains(mainCode)) {
                        m_totals[itYear.key()][mainCode].unitsSold = 0;
                        m_totals[itYear.key()][mainCode].unitsReturned = 0;
                        m_totals[itYear.key()][mainCode].unitsPurchased = 0;
                    }
                    m_totals[itYear.key()][mainCode].unitsSold
                            += itYear.value()[otherCode].unitsSold;
                    m_totals[itYear.key()][mainCode].unitsReturned
                            += itYear.value()[otherCode].unitsReturned;
                    m_totals[itYear.key()][mainCode].unitsPurchased
                            += itYear.value()[otherCode].unitsPurchased;
                }
            }
            for (auto itYear = m_inventoryBeginYear.begin();
                 itYear != m_inventoryBeginYear.end(); ++itYear) {
                if (itYear.value().contains(otherCode)) {
                    if (!m_inventoryBeginYear[itYear.key()].contains(mainCode)) {
                        m_inventoryBeginYear[itYear.key()][mainCode] = QList<Infos>();
                    }
                    m_inventoryBeginYear[itYear.key()][mainCode]
                            << m_inventoryBeginYear[itYear.key()][otherCode];
                    m_inventoryBeginYear[itYear.key()].remove(otherCode);
                }
            }
            for (auto itYear = m_purchases.begin();
                 itYear != m_purchases.end(); ++itYear) {
                if (itYear.value().contains(otherCode)) {
                    if (!m_purchases[itYear.key()].contains(mainCode)) {
                        m_purchases[itYear.key()][mainCode] = QMultiMap<QDate, Infos>();
                    }
                    for (auto it = m_purchases[itYear.key()][otherCode].begin();
                         it != m_purchases[itYear.key()][otherCode].end(); ++it) {
                        m_purchases[itYear.key()][mainCode].insert(
                                    it.key(), it.value());
                    }
                    m_purchases[itYear.key()].remove(otherCode);
                }
            }
        }
    }
    for (auto otherCode : qAsConst(otherCodes)) {
        m_mainCodeToOtherInfos[mainCode].otherCodes << otherCode;
        if (m_otherCodeToMain.contains(mainCode)) {
            m_otherCodeToMain.remove(mainCode);
        }
        m_otherCodeToMain[otherCode] = mainCode;
    }
}
//----------------------------------------------------------
void InventoryManager::sort(
        int column, Qt::SortOrder order)
{
    if (order == Qt::AscendingOrder) {
        std::sort(m_valuesTable.begin(), m_valuesTable.end(),
                  [column](
                  const QStringList &l1, const QStringList &l2){
            //bool isDouble = l1[column].size() < 6;
            bool isDouble = false;
            double d1 = l1[column].toDouble(&isDouble);
            if (isDouble) {
                double d2 = l2[column].toDouble(&isDouble);
                if (isDouble) {
                    if (isDouble) {
                        return d1 > d2;
                    }
                }
            }
            return l1[column] > l2[column];
        });
    } else {
        std::sort(m_valuesTable.begin(), m_valuesTable.end(),
                  [column](
                  const QStringList &l1, const QStringList &l2){
            bool isDouble = l1[column].size() < 6;
            if (isDouble) {
                double d1 = l1[column].toDouble(&isDouble);
                if (isDouble) {
                    double d2 = l2[column].toDouble(&isDouble);
                    if (isDouble) {
                        return d1 < d2;
                    }
                }
            }
            return l1[column] < l2[column];
        });
    }
}
//----------------------------------------------------------
QVariant InventoryManager::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList titles = [this]() -> QStringList {
                QStringList values;
                for (auto info : *_colInfos()) {
                    values << info.name;
                }
                return values;
    }();
        return titles[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int InventoryManager::rowCount(const QModelIndex &) const
{
    return m_valuesTable.size();
}
//----------------------------------------------------------
int InventoryManager::columnCount(const QModelIndex &) const
{
    return _colInfos()->size();
}
//----------------------------------------------------------
Qt::ItemFlags InventoryManager::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}
//----------------------------------------------------------
QVariant InventoryManager::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_valuesTable[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
//----------------------------------------------------------
int InventoryManager::InfoTotals::left() const
{
    return qMax(0, unitsPurchased + unitsReturned - unitsSold);
}
//----------------------------------------------------------
//----------------------------------------------------------
int InventoryManager::Infos::left() const
{
    int left = unitsPurchased.units;
    for (auto val : unitsSold) {
        left -= val;
    }
    for (auto val : unitsReturned) {
        left += val;
    }
    return left;
}
//----------------------------------------------------------
//----------------------------------------------------------
