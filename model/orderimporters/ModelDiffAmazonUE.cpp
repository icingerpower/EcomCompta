#include <QtCore/qfile.h>
#include <QtGui/qbrush.h>
#include <QtGui/qcolor.h>
#include <QtCore/qfileinfo.h>

#include "ModelDiffAmazonUE.h"
#include "model/orderimporters/Shipment.h"
#include "model/SettingManager.h"
#include "model/orderimporters/Order.h"
#include "model/orderimporters/OrderManager.h"
#include "model/orderimporters/OrderImporterAmazonUE.h"

QString ModelDiffAmazonUE::COL_ORDER_ID = "TRANSACTION_EVENT_ID";
QString ModelDiffAmazonUE::COL_SHIPMENT_ID = "ACTIVITY_TRANSACTION_ID";
QString ModelDiffAmazonUE::COL_TRANSACTION_TYPE = "TRANSACTION_TYPE";
QString ModelDiffAmazonUE::COL_INVOICE_NUMBER = "VAT_INV_NUMBER";
//QString ModelDiffAmazonUE::COL_DATE = "TRANSACTION_DEPART_DATE";
QString ModelDiffAmazonUE::COL_DATE = "TRANSACTION_COMPLETE_DATE";
QString ModelDiffAmazonUE::COL_VAT_REGIME = "TAX_REPORTING_SCHEME";
QString ModelDiffAmazonUE::COL_MARKETPLACE = "MARKETPLACE";
QString ModelDiffAmazonUE::COL_COUNTRY_VAT = "VAT_CALCULATION_IMPUTATION_COUNTRY";
QString ModelDiffAmazonUE::COL_COUNTRY_DECLARATION = "TAXABLE_JURISDICTION";
QString ModelDiffAmazonUE::COL_VAT = "TOTAL_ACTIVITY_VALUE_VAT_AMT";
QString ModelDiffAmazonUE::COL_TOTAL = "TOTAL_ACTIVITY_VALUE_AMT_VAT_INCL";
QString ModelDiffAmazonUE::COL_CURRENCY = "TRANSACTION_CURRENCY_CODE";
QString ModelDiffAmazonUE::COL_BUYER_VAT_NUMBER = "BUYER_VAT_NUMBER";
//QString ModelDiffAmazonUE::COL_COUNTRY_TO = "ARRIVAL_COUNTRY";
QString ModelDiffAmazonUE::COL_COUNTRY_TO = "SALE_ARRIVAL_COUNTRY";
//QString ModelDiffAmazonUE::COL_COUNTRY_FROM = "DEPARTURE_COUNTRY";
QString ModelDiffAmazonUE::COL_COUNTRY_FROM = "SALE_DEPART_COUNTRY";
QString ModelDiffAmazonUE::COL_SKU = "SELLER_SKU";
QString ModelDiffAmazonUE::COL_QTY = "QTY";
QString ModelDiffAmazonUE::COL_REFUND_ID = "ACTIVITY_TRANSACTION_ID";
QString ModelDiffAmazonUE::COL_MERGED_SKUS = "MERGED_SKUS_QUANTITY";
QString ModelDiffAmazonUE::LOC_AMAZON_ONLY = QObject::tr("Seulement amazon");
QString ModelDiffAmazonUE::LOC_REPORTS_ONLY = QObject::tr("Manquante dans amazon");
QString ModelDiffAmazonUE::LOC_ALL = QObject::tr("OK (Tous les rapports)");

//----------------------------------------------------------
ModelDiffAmazonUE::ModelDiffAmazonUE(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_totalShipmentAnalyzed = 0;
}
//----------------------------------------------------------
void ModelDiffAmazonUE::compute(const OrderManager *orderManager, int year)
{
    clear();
    computeAmazonVatData(year);
    QSet<QString> amazonShipmentIdsLeft = m_valuesAmazon.keys().toSet();
    QMultiMap<QDateTime, Shipment *> shipmentsBydate;
    for (auto shipment : orderManager->m_ordersByChannel[OrderImporterAmazonUE().name()]
         .shipmentByDate[year]) {
        shipmentsBydate.insert(shipment->getDateTime(), shipment.data());
    }
    for (auto refund : orderManager->m_ordersByChannel[OrderImporterAmazonUE().name()]
         .refundByDate[year]) {
        shipmentsBydate.insert(refund->getDateTime(), refund.data());
    }
    for (auto shipment : shipmentsBydate) {
        ++m_totalShipmentAnalyzed;
        QString shipmentId = shipment->getId();
        QVector<QVariant> values = QVector<QVariant>(colInfos()->size());
        QHash<QString, QString> amazonValues;
        bool amazonValuesFound = m_valuesAmazon.contains(shipmentId);
        if (amazonValuesFound) {
            amazonShipmentIdsLeft.remove(shipmentId);
            amazonValues = m_valuesAmazon[shipmentId];
        }
        int pos = 0;
        for (auto function : *colInfos()) {
            values[pos] = function.getValue(shipment, amazonValues);
            ++pos;
        }
        if (amazonValuesFound) {
            static QMultiHash<QString, QString> amazonVatRegimeMapping
                    = {{"UNION-OSS",Shipment::VAT_REGIME_OSS}
                       ,{"UNION-IOSS",Shipment::VAT_REGIME_IOSS}  /// Check
                       ,{"REGULAR",Shipment::VAT_REGIME_NORMAL}
                       ,{"REGULAR",Shipment::VAT_REGIME_NORMAL_EXPORT}
                       ,{"REGULAR",Shipment::VAT_REGIME_NONE}
                       ,{"UK_VOEC-DOMESTIC",Shipment::VAT_REGIME_NONE}
                       ,{"",Shipment::VAT_REGIME_NONE} /// Check
                      };
            //bool diffForWatch = amazonValues[COL_COUNTRY_FROM] == "PL"
                    //&& amazonValues[COL_COUNTRY_TO] == "IT";
            bool diffRegime = !amazonVatRegimeMapping.values(
                        amazonValues[COL_VAT_REGIME])
                    .contains(shipment->getRegimeVat());
            bool diffCountryVat = amazonValues[COL_COUNTRY_VAT] != shipment->getCountryCodeVat();
            if (shipment->orderId() == "408-2754977-3751548") {
                int TEMP=10;++TEMP;
            }
            if (shipment->getId() == "DVgYgfTG1") {
                int TEMP=10;++TEMP;
            }
            if (diffCountryVat) {
                QString countryInvoice = shipment->getInvoiceNameMarketPlace();
                if (!countryInvoice.isEmpty()) {
                    QStringList elements = countryInvoice.split("-");
                    elements.takeLast();
                    elements.takeLast();
                    elements.takeLast();
                    QString countryInfo = elements.join("-");
                    diffCountryVat = !countryInfo.contains(shipment->getCountryCodeVat());
                }
            }
            bool diffCountryDeclaration = amazonValues[COL_COUNTRY_DECLARATION] != shipment->getCountrySaleDeclaration();
            bool diffDate = QDate::fromString(amazonValues[COL_DATE], "dd-MM-yyyy") != shipment->getDateTime().date();
            bool diffVat = qAbs(amazonValues[COL_VAT].toDouble() - shipment->getTotalPriceTaxes()) > 0.009999;
            //bool diffCurrencyConverted // I forgot the purpose
                    //= amazonValues[COL_CURRENCY] != SettingManager::instance()->currency()
                    //&& qAbs(amazonValues[COL_COUNTRY_VAT].toDouble()
                            //- shipment->getTotalPriceTaxes()) > 0.01;
            //|| amazonValues[COL_ORDER_ID] == "171-1896960-6665907";
            if (diffRegime || diffCountryVat || diffCountryDeclaration || diffDate || diffVat) {
            //if (diffForWatch) {
                m_values << values;
            }
        } else {
            m_values << values;
        }
    }
    m_totalShipmentAnalyzed += amazonShipmentIdsLeft.size();
    for (auto shipmentId : amazonShipmentIdsLeft) {
        QVector<QVariant> values = QVector<QVariant>(colInfos()->size());
        int pos = 0;
        for (auto function : *colInfos()) {
            values[pos] = function.getValue(nullptr, m_valuesAmazon[shipmentId]);
            ++pos;
        }
        m_values.insert(0, values);
    }
    if (m_values.size() > 0) {
        beginInsertRows(QModelIndex(), 0, m_values.size()-1);
        endInsertRows();
    }
}
//----------------------------------------------------------
void ModelDiffAmazonUE::clear()
{
    if (m_values.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_values.size()-1);
        m_values.clear();
        m_valuesAmazon.clear();
        endRemoveRows();
    }
}
//----------------------------------------------------------
QVariant ModelDiffAmazonUE::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return (*colInfos())[section].name;
    } else if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        return section + 1;
    }
    return QVariant();
}
//----------------------------------------------------------
QString ModelDiffAmazonUE::amazonCountryToCode(const QString &country) const
{
    static QHash<QString, QString> mapping
            = {{"FRANCE", "FR"}
               ,{"UNITED KINGDOM", "GB"}
               ,{"GERMANY", "DE"}
               ,{"SPAIN", "ES"}
               ,{"ITALY", "IT"}
               ,{"POLAND", "PL"}
               ,{"CZECH REPUBLIC", "CZ"}
               ,{"NETHERLANDS", "NL"}
               ,{"SWEDEN", "SE"}
               ,{"BELGIUM", "BE"}
               ,{"BULGARIA", "BG"}
               ,{"DENMARK", "DK"}
               ,{"ESTONIA", "EE"}
               ,{"IRELAND", "IE"}
               ,{"CROATIA", "HR"}
               ,{"CYPRUS", "CY"}
               ,{"LETTONIA", "LV"}
               ,{"LITUANIA", "LT"}
               ,{"LITHUANIA", "LT"}
               ,{"GREECE", "GR"}
               ,{"FINLAND", "FI"}
               ,{"LUXEMBOURG", "LU"}
               ,{"HUNGARY", "HU"}
               ,{"MALTA", "MT"}
               ,{"AUSTRIA", "AT"}
               ,{"PORTUGAL", "PT"}
               ,{"ROMANIA", "RO"}
               ,{"SLOVENIA", "SI"}
               ,{"SLOVAKIA", "SK"}
               ,{"LATVIA", "LV"}
               ,{"", ""}
               };
    QString upperCountry = country.toUpper();
    Q_ASSERT(mapping.contains(upperCountry));
    return mapping[upperCountry];
}
//----------------------------------------------------------
int ModelDiffAmazonUE::rowCount(const QModelIndex &) const
{
    return m_values.size();
}
//----------------------------------------------------------
int ModelDiffAmazonUE::columnCount(const QModelIndex &) const
{
    static int count = colInfos()->size();
    return count;
}
//----------------------------------------------------------
Qt::ItemFlags ModelDiffAmazonUE::flags(const QModelIndex &index) const
{
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}
//----------------------------------------------------------
QVariant ModelDiffAmazonUE::data(const QModelIndex &index, int role) const
{
    static QVariant colorGreen = SettingManager::instance()->brushGreen();
    static QVariant colorRed = SettingManager::instance()->brushRed();
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_values[index.row()][index.column()];
    } else if (role == Qt::BackgroundRole) {
        if (index.column() == 0) {
            if (m_values[index.row()][0] == LOC_ALL) {
                return colorGreen;
            } else if (m_values[index.row()][0] == LOC_AMAZON_ONLY) {
                return colorRed;
            } else if (m_values[index.row()][0] == LOC_REPORTS_ONLY) {
                return colorRed;
            }
        } else {
            if (m_values[index.row()][0] == LOC_ALL) {
                if ((*colInfos())[index.column()].compareWithPrevious) {
                    if (m_values[index.row()][index.column()-1] != m_values[index.row()][index.column()]) {
                        return colorRed;
                    } else {
                        return colorGreen;
                    }
                }
                if (index.column() + 1 < columnCount()
                        && (*colInfos())[index.column()+1].compareWithPrevious) {
                    if (m_values[index.row()][index.column()] != m_values[index.row()][index.column()+1]) {
                        return colorRed;
                    } else {
                        return colorGreen;
                    }
                }
            }
        }
    }
    return QVariant();
}
//----------------------------------------------------------
void ModelDiffAmazonUE::computeAmazonVatData(int year)
{
    QDir reportDir = SettingManager::instance()->reportDirectory(
                OrderImporterAmazonUE().name(),
                OrderImporterAmazonUE::REPORT_VAT_SHORT,
                QString::number(year));
    QStringList relImportedFileNames = reportDir.entryList(QDir::Files);
    for (auto relImportedFileName : relImportedFileNames) {
        QString importedFilePath = reportDir.filePath(relImportedFileName);
        CsvReader reader = OrderImporterAmazonUE().createAmazonReader(importedFilePath);
        reader.readAll();
        const DataFromCsv *dataRode = reader.dataRode();
        for (auto elements : dataRode->lines) {
            int indTransactionType = dataRode->header.pos(ModelDiffAmazonUE::COL_TRANSACTION_TYPE);
            QString transactionType = elements[indTransactionType];
            if (transactionType == "SALE" || transactionType == "REFUND") {
                int indShipmentId = dataRode->header.pos(ModelDiffAmazonUE::COL_SHIPMENT_ID);
                QString shipmentId = elements[indShipmentId];
                if (transactionType == "SALE") {
                    m_valuesAmazon[shipmentId][COL_TRANSACTION_TYPE] = tr("Vente");
                } else {
                    m_valuesAmazon[shipmentId][COL_TRANSACTION_TYPE] = tr("Remboursement");
                }
                if (!m_valuesAmazon.contains(shipmentId)) {
                    m_valuesAmazon[shipmentId] = QHash<QString, QString>();
                }
                m_valuesAmazon[shipmentId][ModelDiffAmazonUE::COL_SHIPMENT_ID] = shipmentId;
                int indAmazonOrderId = dataRode->header.pos(COL_ORDER_ID);
                m_valuesAmazon[shipmentId][COL_ORDER_ID] = elements[indAmazonOrderId];
                m_valuesAmazon[shipmentId][COL_DATE] = elements[dataRode->header.pos(COL_DATE)];
                m_valuesAmazon[shipmentId][COL_BUYER_VAT_NUMBER] = elements[dataRode->header.pos(COL_BUYER_VAT_NUMBER)];
                m_valuesAmazon[shipmentId][COL_VAT_REGIME] = elements[dataRode->header.pos(COL_VAT_REGIME)];
                m_valuesAmazon[shipmentId][COL_COUNTRY_VAT] = elements[dataRode->header.pos(COL_COUNTRY_VAT)];
                m_valuesAmazon[shipmentId][COL_COUNTRY_DECLARATION] = amazonCountryToCode(elements[dataRode->header.pos(COL_COUNTRY_DECLARATION)]);
                m_valuesAmazon[shipmentId][COL_CURRENCY] = elements[dataRode->header.pos(COL_CURRENCY)];
                m_valuesAmazon[shipmentId][COL_COUNTRY_TO] = elements[dataRode->header.pos(COL_COUNTRY_TO)];
                m_valuesAmazon[shipmentId][COL_COUNTRY_FROM] = elements[dataRode->header.pos(COL_COUNTRY_FROM)];
                m_valuesAmazon[shipmentId][COL_INVOICE_NUMBER] = elements[dataRode->header.pos(COL_INVOICE_NUMBER)];
                QString sku = elements[dataRode->header.pos(COL_SKU)];
                QString quantity = elements[dataRode->header.pos(COL_QTY)];
                QString skuQuantity = sku + "=" + quantity;
                if (!m_valuesAmazon[shipmentId].contains(COL_MERGED_SKUS)) {
                    m_valuesAmazon[shipmentId][COL_MERGED_SKUS] = skuQuantity;
                    m_valuesAmazon[shipmentId][COL_VAT] = elements[dataRode->header.pos(COL_VAT)];
                    m_valuesAmazon[shipmentId][COL_TOTAL] = elements[dataRode->header.pos(COL_TOTAL)];
                } else {
                    m_valuesAmazon[shipmentId][COL_MERGED_SKUS] += " | " + skuQuantity;
                    m_valuesAmazon[shipmentId][COL_VAT] = QString::number(
                                m_valuesAmazon[shipmentId][COL_VAT].toDouble()
                                + elements[dataRode->header.pos(COL_VAT)].toDouble());
                    m_valuesAmazon[shipmentId][COL_TOTAL] = QString::number(
                                m_valuesAmazon[shipmentId][COL_TOTAL].toDouble()
                                + elements[dataRode->header.pos(COL_TOTAL)].toDouble());
                }
            }
        }
    }
}
//----------------------------------------------------------
int ModelDiffAmazonUE::totalShipmentAnalyzed() const
{
    return m_totalShipmentAnalyzed;
}
//----------------------------------------------------------
QList<ModelDiffAmazonUE::ColInfo> *ModelDiffAmazonUE::colInfos() const
{
     static QList<ModelDiffAmazonUE::ColInfo> colInfos
            = {{tr("Localisation"), [](const Shipment *shipment,
                             const QHash<QString, QString> &amazonValues) -> QVariant {
                    if (shipment == nullptr) {
                        return LOC_AMAZON_ONLY;
                    } else if (amazonValues.size() == 0) {
                        return LOC_REPORTS_ONLY;
                    } else {
                        return LOC_ALL;
                    }
                }, false},
               {tr("Type"), [](const Shipment *shipment,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    if (shipment == nullptr) {
                        if (dynamic_cast<const Refund *>(shipment) != nullptr) {
                            return tr("Remboursement");
                        } else {
                            return tr("Vente");
                        }
                    }
                    return amazonValues[COL_TRANSACTION_TYPE];
                }, false},
               {tr("Commande"), [](const Shipment *shipment,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    if (shipment == nullptr) {
                        return amazonValues[COL_ORDER_ID];
                    }
                    return shipment->getOrder()->getId();
                }, false},
               {tr("Expédition"), [](const Shipment *shipment,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    if (shipment == nullptr) {
                        return amazonValues[COL_SHIPMENT_ID];
                    }
                    return shipment->getId();
                }, false},
               {tr("Date Commande"), [](const Shipment *shipment,
                             const QHash<QString, QString> &) -> QVariant{
                    if (shipment == nullptr) {
                        return "";
                    }
                    return shipment->getOrder()->getDateTime().date().toString("dd-MM-yyyy");
                }, false},
               {tr("Date Expédition"), [](const Shipment *shipment,
                             const QHash<QString, QString> &) -> QVariant{
                    if (shipment == nullptr) {
                        return "";
                    }
                    return shipment->getDateTime().date().toString("dd-MM-yyyy");
                }, false},
               {tr("Date Amazon"), [](const Shipment *,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    return amazonValues.value(COL_DATE, "");
                }, true},
               {tr("Régime"), [](const Shipment *shipment,
                             const QHash<QString, QString> &) -> QVariant{
                    if (shipment == nullptr) {
                        return "";
                    }
                    return shipment->getRegimeVat();
                }, false},
               {tr("Régime Amazon"), [](const Shipment *,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    return amazonValues.value(COL_VAT_REGIME, "");
                }, false},
               {tr("Amazon"), [](const Shipment *shipment,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    if (shipment == nullptr) {
                        return amazonValues.value(COL_MARKETPLACE, "");
                    }
                    return shipment->getOrder()->getSubchannel();
                }, false},
               {tr("Numéro de facture"), [](const Shipment *,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    return amazonValues.value(COL_INVOICE_NUMBER, "");
                }, false},
               {tr("Client pro"), [](const Shipment *shipment,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    if (shipment == nullptr) {
                        if (amazonValues.value(COL_BUYER_VAT_NUMBER, "").isEmpty()) {
                            return tr("NON");
                        } else {
                            return tr("OUI");
                        }
                    }
                    if (shipment->getOrder()->isBusinessCustomer()) {
                        return tr("OUI");
                    }
                    return tr("NON");
                }, false},
               {tr("Pays TVA"), [](const Shipment *shipment,
                             const QHash<QString, QString> &) -> QVariant{
                    if (shipment == nullptr) {
                        return "";
                    }
                    return shipment->getCountryCodeVat();
                }, false},
               {tr("Pays TVA Amazon"), [](const Shipment *,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    return amazonValues.value(COL_COUNTRY_VAT, "");
                }, true},
               {tr("Pays Déclaration"), [](const Shipment *shipment,
                             const QHash<QString, QString> &) -> QVariant{
                    if (shipment == nullptr) {
                        return "";
                    }
                    return shipment->getCountrySaleDeclaration();
                }, false},
               {tr("Pays Déclaration Amazon"), [](const Shipment *,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    return amazonValues.value(COL_COUNTRY_DECLARATION, "");
                }, true},
               {tr("État"), [](const Shipment *shipment,
                             const QHash<QString, QString> &) -> QVariant{
                    if (shipment == nullptr || shipment->getOrder() == nullptr) {
                        return "";
                    }
                    return shipment->getOrder()->getAddressTo().state();
                }, false},
               {tr("Total converti", "The total price converted in right currency"), [](const Shipment *shipment,
                             const QHash<QString, QString> &) -> QVariant{
                    if (shipment == nullptr) {
                        return "";
                    }
                    return QString::number(shipment->getTotalPriceTaxedConverted(), 'f', 2);
                }, false},
               {tr("Total"), [](const Shipment *shipment,
                             const QHash<QString, QString> &) -> QVariant{
                    if (shipment == nullptr) {
                        return "";
                    }
                    return QString::number(shipment->getTotalPriceTaxed(), 'f', 2);
                }, false},
               {tr("Total Amazon"), [](const Shipment *,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    return QString::number(amazonValues.value(COL_TOTAL, "").toDouble(), 'f', 2);
                }, true},
               {tr("TVA converti", "The total vat converted in right currency"), [](const Shipment *shipment,
                             const QHash<QString, QString> &) -> QVariant{
                    if (shipment == nullptr) {
                        return "";
                    }
                    return QString::number(shipment->getTotalPriceTaxesConverted(), 'f', 2);
                }, false},
               {tr("TVA"), [](const Shipment *shipment,
                             const QHash<QString, QString> &) -> QVariant{
                    if (shipment == nullptr) {
                        return "";
                    }
                    return QString::number(shipment->getTotalPriceTaxes(), 'f', 2);
                }, false},
               {tr("TVA Amazon"), [](const Shipment *,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    return QString::number(amazonValues.value(COL_VAT, "").toDouble(), 'f', 2);
                }, true},
               {tr("Monnaie"), [](const Shipment *shipment,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    if (shipment == nullptr) {
                        return amazonValues.value(COL_CURRENCY, "");
                    }
                    return shipment->getOrder()->getCurrency();
                }, false},
               {tr("Numéro TVA acheteur"), [](const Shipment *,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    return amazonValues.value(COL_BUYER_VAT_NUMBER, "");
                }, false},
               {tr("Pays expédition"), [](const Shipment *,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    return amazonValues.value(COL_COUNTRY_FROM, "");
                }, false},
               {tr("Pays destination"), [](const Shipment *,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    return amazonValues.value(COL_COUNTRY_TO, "");
                }, false},
               {tr("SKUs commandés"), [](const Shipment *,
                             const QHash<QString, QString> &amazonValues) -> QVariant{
                    return amazonValues.value(COL_MERGED_SKUS, "");
                }, false}
               };
    return &colInfos;
}
//----------------------------------------------------------
