#include <QtCore/qsettings.h>

#include "model/SettingManager.h"

#include "OrderImporterCustomParams.h"

QString OrderImporterCustomParams::ID_CSV_SEP = QObject::tr("Séparateur colonne");
QString OrderImporterCustomParams::ID_CSV_DEL = QObject::tr("Délimiteur du texte");
QString OrderImporterCustomParams::ID_CURRENCY = QObject::tr("Devise");
QString OrderImporterCustomParams::ID_ORDER = QObject::tr("Identifiant commande");
QString OrderImporterCustomParams::ID_DATE_TIME = QObject::tr("Date et heure");
QString OrderImporterCustomParams::ID_DATE_TIME_FORMAT = QObject::tr("Format date et heure");
QString OrderImporterCustomParams::ID_DATE = QObject::tr("Date");
QString OrderImporterCustomParams::ID_DATE_FORMAT = QObject::tr("Format date");
QString OrderImporterCustomParams::ID_TIME = QObject::tr("Heure");
QString OrderImporterCustomParams::ID_TIME_FORMAT = QObject::tr("Format heure");
QString OrderImporterCustomParams::ID_SKU = QObject::tr("SKU");
QString OrderImporterCustomParams::ID_ARTICLE_NAME = QObject::tr("Nom de l'article");
QString OrderImporterCustomParams::ID_QUANTITY = QObject::tr("Quantité");
QString OrderImporterCustomParams::ID_SUBCHANNEL = QObject::tr("Canal de vente");

QString OrderImporterCustomParams::ID_TOTAL_PAID_ORDER = QObject::tr("Total payé");

QString OrderImporterCustomParams::ID_ARTICLE_UNIT_PRICE = QObject::tr("Price unitaire article TTC");
QString OrderImporterCustomParams::ID_ARTICLE_SUM_PRICE = QObject::tr("Price total article TTC");
QString OrderImporterCustomParams::ID_ARTICLE_DISCOUNT = QObject::tr("Réduction article TTC");
QString OrderImporterCustomParams::ID_ARTICLE_DISCOUNT_TO_DEDUCT = QObject::tr("Réduction article TTC à soustraire");
QString OrderImporterCustomParams::ID_ARTICLE_SHIPPING = QObject::tr("Price expédition");

QString OrderImporterCustomParams::ID_TOTAL_REFUNDED = QObject::tr("Total remboursé");
QString OrderImporterCustomParams::ID_DATE_REFUNDED = QObject::tr("Date remboursement");
QString OrderImporterCustomParams::ID_DATE_TIME_REFUNDED = QObject::tr("Date heure remboursement");
QString OrderImporterCustomParams::ID_VALUES_REFUND_TO_DEDUCT = QObject::tr("Colonnes qui indiquent remboursement à déduire");

QString OrderImporterCustomParams::ID_BUSINESS_VAT_NUMBER = QObject::tr("Numéro de TVA");
QString OrderImporterCustomParams::ID_NAME = QObject::tr("Nom client livraison");
QString OrderImporterCustomParams::ID_STREET1 = QObject::tr("Rue 1 livraison");
QString OrderImporterCustomParams::ID_STREET2 = QObject::tr("Rue 2 livraison");
QString OrderImporterCustomParams::ID_POSTAL_CODE = QObject::tr("Code postal livraison");
QString OrderImporterCustomParams::ID_CITY = QObject::tr("Ville livraison");
QString OrderImporterCustomParams::ID_STATE = QObject::tr("État livraison");
QString OrderImporterCustomParams::ID_COUNTRY_CODE = QObject::tr("Code pays livraison");
QString OrderImporterCustomParams::ID_BILL_BUSINESS_NAME = QObject::tr("Nom société livraison");
QString OrderImporterCustomParams::ID_BILL_NAME = QObject::tr("Nom client facturation");
QString OrderImporterCustomParams::ID_BILL_STREET1 = QObject::tr("Rue 1 facturation");
QString OrderImporterCustomParams::ID_BILL_STREET2 = QObject::tr("Rue 2 facturation");
QString OrderImporterCustomParams::ID_BILL_POSTAL_CODE = QObject::tr("Code postal facturation");
QString OrderImporterCustomParams::ID_BILL_CITY = QObject::tr("Ville facturation");
QString OrderImporterCustomParams::ID_BILL_STATE = QObject::tr("État facturation");
QString OrderImporterCustomParams::ID_BILL_COUNTRY_CODE = QObject::tr("Code pays facturation");

//----------------------------------------------------------
OrderImporterCustomParams::OrderImporterCustomParams(const QString &uniqueId, QObject *parent)
    : QAbstractTableModel(parent)
{
    m_indexUntilAndatory = 1;
    m_uniqueId = uniqueId;
    _generateValues();
    loadFromSettings();
}
//----------------------------------------------------------
void OrderImporterCustomParams::_generateValues()
{
    QStringList list = {
        OrderImporterCustomParams::ID_CSV_SEP
        , OrderImporterCustomParams::ID_CSV_DEL
        , OrderImporterCustomParams::ID_CURRENCY
        , OrderImporterCustomParams::ID_ORDER
        , OrderImporterCustomParams::ID_DATE_TIME
        , OrderImporterCustomParams::ID_DATE_TIME_FORMAT
        , OrderImporterCustomParams::ID_DATE
        , OrderImporterCustomParams::ID_DATE_FORMAT
        , OrderImporterCustomParams::ID_TIME
        , OrderImporterCustomParams::ID_TIME_FORMAT
        , OrderImporterCustomParams::ID_SKU
        , OrderImporterCustomParams::ID_ARTICLE_NAME
        , OrderImporterCustomParams::ID_QUANTITY
        , OrderImporterCustomParams::ID_SUBCHANNEL
        , OrderImporterCustomParams::ID_TOTAL_PAID_ORDER
        , OrderImporterCustomParams::ID_ARTICLE_UNIT_PRICE
        , OrderImporterCustomParams::ID_ARTICLE_SUM_PRICE
        , OrderImporterCustomParams::ID_ARTICLE_DISCOUNT
        //, OrderImporterCustomParams::ID_ARTICLE_DISCOUNT_TO_DEDUCT
        , OrderImporterCustomParams::ID_ARTICLE_SHIPPING
        , OrderImporterCustomParams::ID_TOTAL_REFUNDED
        , OrderImporterCustomParams::ID_DATE_REFUNDED
        , OrderImporterCustomParams::ID_DATE_TIME_REFUNDED
        , OrderImporterCustomParams::ID_VALUES_REFUND_TO_DEDUCT
        , OrderImporterCustomParams::ID_NAME
        , OrderImporterCustomParams::ID_STREET1
        , OrderImporterCustomParams::ID_STREET2
        , OrderImporterCustomParams::ID_POSTAL_CODE
        , OrderImporterCustomParams::ID_CITY
        , OrderImporterCustomParams::ID_STATE
        , OrderImporterCustomParams::ID_COUNTRY_CODE
        , OrderImporterCustomParams::ID_BUSINESS_VAT_NUMBER
        , OrderImporterCustomParams::ID_BILL_NAME
        , OrderImporterCustomParams::ID_BILL_BUSINESS_NAME
        , OrderImporterCustomParams::ID_BILL_STREET1
        , OrderImporterCustomParams::ID_BILL_STREET2
        , OrderImporterCustomParams::ID_BILL_POSTAL_CODE
        , OrderImporterCustomParams::ID_BILL_CITY
        , OrderImporterCustomParams::ID_BILL_STATE
        , OrderImporterCustomParams::ID_BILL_COUNTRY_CODE
    };
    int id=0;
    for (auto val : list) {
        m_values << QStringList({val, "", ""});
        m_titleToIndex[val] = id;
        ++id;
    }
}
//----------------------------------------------------------
void OrderImporterCustomParams::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (m_values.size() > 0) {
        QStringList lines;
        for (auto elements : m_values) {
            lines << elements.join(";;;");
        }
        settings.setValue(_settingKey(), lines.join(":::"));
    } else if (settings.contains(_settingKey())) {
        settings.remove(_settingKey());
    }
}
//----------------------------------------------------------
void OrderImporterCustomParams::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(_settingKey())) {
        QStringList lines = settings.value(_settingKey()).toString().split(":::");
        int i=0;
        for (auto line : lines) {
            QStringList elements = line.split(";;;");
            for (int j=0; j<m_values.size(); ++j) {
                if (m_values[j][0] == elements[0]) {
                    m_values[j] = elements;
                }
            }
            ++i;
        }
        emit dataChanged(index(0,0),
                         index(rowCount()-1, columnCount()-1));
    }
}
//----------------------------------------------------------
QStringList OrderImporterCustomParams::columns(
        const QString &id) const
{
    int index = m_titleToIndex[id];
    return m_values[index][1].split(",");
}
//----------------------------------------------------------
bool OrderImporterCustomParams::hasCol(const QString &id) const
{
    int index = m_titleToIndex[id];
    return !m_values[index][1].isEmpty();
}
//----------------------------------------------------------
bool OrderImporterCustomParams::hasDefaultValue(const QString &id) const
{
    int index = m_titleToIndex[id];
    return !m_values[index][2].isEmpty();
}
//----------------------------------------------------------
QString OrderImporterCustomParams::valueCol(const QString &id) const
{
    int index = m_titleToIndex[id];
    return m_values[index][1];
}
//----------------------------------------------------------
QString OrderImporterCustomParams::valueDefault(const QString &id) const
{
    int index = m_titleToIndex[id];
    return m_values[index][2];
}
//----------------------------------------------------------
QString OrderImporterCustomParams::valueAnyCol(const QString &id) const
{
    int index = m_titleToIndex[id];
    if (m_values[index][1].isEmpty()) {
        return m_values[index][2];
    }
    return m_values[index][1];
}
//----------------------------------------------------------
QVariant OrderImporterCustomParams::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        static QStringList values = {tr("Colonne"), tr("Nom"), tr("Valeur par défaut")};
        return values[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int OrderImporterCustomParams::rowCount(const QModelIndex &) const
{
    return m_values.size();
}
//----------------------------------------------------------
int OrderImporterCustomParams::columnCount(const QModelIndex &) const
{
    return 3;
}
//----------------------------------------------------------
QVariant OrderImporterCustomParams::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_values[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
bool OrderImporterCustomParams::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value && index.column() > 0) {
        m_values[index.row()][index.column()] = value.toString();
        m_titleToIndex[m_values[index.row()][0]] = index.row();
        saveInSettings();
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}
//----------------------------------------------------------
Qt::ItemFlags OrderImporterCustomParams::flags(const QModelIndex &index) const
{
    if (index.column() == 0) {
        return Qt::ItemIsEnabled;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsEditable;
}
//----------------------------------------------------------
QString OrderImporterCustomParams::_settingKey() const
{
    return "OrderImporterCustomParams-" + m_uniqueId;
}
//----------------------------------------------------------
QString OrderImporterCustomParams::uniqueId() const
{
    return m_uniqueId;
}
//----------------------------------------------------------
