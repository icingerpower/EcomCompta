#include <QSettings>

#include "model/SettingManager.h"

#include "ManagerAccountsAmazon.h"

//----------------------------------------------------------
ManagerAccountsAmazon::ManagerAccountsAmazon(QObject *parent)
    : QAbstractTableModel(parent), UpdateToCustomer()
{
    _generateBasicAccounts();
    init();
}
//----------------------------------------------------------
void ManagerAccountsAmazon::_clear()
{
    beginRemoveRows(QModelIndex(), 0, m_values.size()-1);
    m_values.clear();
    endRemoveRows();
}
//----------------------------------------------------------
void ManagerAccountsAmazon::_generateBasicAccounts()
{
    /// adds account charges
#ifdef Q_OS_LINUX
    m_values["amazon.??"] = QStringList({"CCLIENTINCON", "FAMAZON", "275110", "467008", "622201"});
    m_values["amazon.fr"] = QStringList({"CCLIENTINCON", "FAMAZON", "275111", "467008", "622201"});
    m_values["amazon.de"] = QStringList({"CCLIENTINCON", "FAMAZON", "275112", "467008", "622201"});
    m_values["amazon.it"] = QStringList({"CCLIENTINCON", "FAMAZON", "275113", "467008", "622201"});
    m_values["amazon.es"] = QStringList({"CCLIENTINCON", "FAMAZON", "275114", "467008", "622201"});
    m_values["amazon.nl"] = QStringList({"CCLIENTINCON", "FAMAZON", "275115", "467008", "622201"});
    m_values["amazon.se"] = QStringList({"CCLIENTINCON", "FAMAZON", "275116", "467008", "622201"});
    m_values["amazon.pl"] = QStringList({"CCLIENTINCON", "FAMAZON", "275117", "467008", "622201"});
    m_values["amazon.com.be"] = QStringList({"CCLIENTINCON", "FAMAZON", "275118", "467008", "622201"});
    m_values["amazon.com.tr"] = QStringList({"CCLIENTINCON", "FAMAZON", "275180", "467010", "622201"});
    m_values["amazon.co.uk"] = QStringList({"CCLIENTINCON", "FAMAZON", "275145", "467010", "622201"});
    m_values["amazon.com"] = QStringList({"CCLIENTINCON", "FAMAZON", "275150", "467010", "622201"});
    m_values["amazon.ca"] = QStringList({"CCLIENTINCON", "FAMAZON", "275160", "467010", "622201"});
    m_values["amazon.com.mx"] = QStringList({"CCLIENTINCON", "FAMAZON", "275170", "467010", "622201"});
    m_values["cdiscount.fr"] = QStringList({"CCDISCOUNT", "FCDISCOUNT", "467008", "467008", "622201"});
    m_values["fnac.com"] = QStringList({"CFNAC", "FFNAC", "467010", "467010", "622201"});
#else
    m_values["amazon.??"] = QStringList({"CLIENTAMAZON", "FAMAZON", "467110", "CCLIENTAMAZONUECREANCE", "622201"});
    m_values["amazon.fr"] = QStringList({"CLIENTAMAZON", "FAMAZON", "467111", "CCLIENTAMAZONUECREANCE", "622201"});
    m_values["amazon.de"] = QStringList({"CLIENTAMAZON", "FAMAZON", "467112", "CCLIENTAMAZONUECREANCE", "622201"});
    m_values["amazon.it"] = QStringList({"CLIENTAMAZON", "FAMAZON", "467113", "CCLIENTAMAZONUECREANCE", "622201"});
    m_values["amazon.es"] = QStringList({"CLIENTAMAZON", "FAMAZON", "467114", "CCLIENTAMAZONUECREANCE", "622201"});
    m_values["amazon.nl"] = QStringList({"CLIENTAMAZON", "FAMAZON", "467115", "CCLIENTAMAZONUECREANCE", "622201"});
    m_values["amazon.se"] = QStringList({"CLIENTAMAZON", "FAMAZON", "467116", "CCLIENTAMAZONUECREANCE", "622201"});
    m_values["amazon.pl"] = QStringList({"CLIENTAMAZON", "FAMAZON", "467117", "CCLIENTAMAZONUECREANCE", "622201"});
    m_values["amazon.com.be"] = QStringList({"CLIENTAMAZON", "FAMAZON", "467118", "CCLIENTAMAZONUECREANCE", "622201"});
    m_values["amazon.com.tr"] = QStringList({"CLIENTAMAZON", "FAMAZON", "467150", "CCLIENTAMAZONCREANCEHORSUE", "622201"});
    m_values["amazon.co.uk"] = QStringList({"CLIENTAMAZON", "FAMAZON", "467150", "CCLIENTAMAZONCREANCEHORSUE", "622201"});
    m_values["amazon.com"] = QStringList({"CLIENTAMAZON", "FAMAZON", "467150", "CCLIENTAMAZONCREANCEHORSUE", "622201"});
    m_values["amazon.ca"] = QStringList({"CLIENTAMAZON", "FAMAZON", "467160", "CCLIENTAMAZONCREANCEHORSUE", "622201"});
    m_values["amazon.com.mx"] = QStringList({"CLIENTAMAZON", "FAMAZON", "467170", "CCLIENTAMAZONCREANCEHORSUE", "622201"});
    m_values["cdiscount.fr"] = QStringList({"CCDISCOUNT", "FCDISCOUNT", "467008", "CCDISCOUNTCREANCE", "622201"});
    m_values["fnac.com"] = QStringList({"CFNAC", "FFNAC", "467010", "CFNACCREANCE", "622201"});

#endif
    beginInsertRows(QModelIndex(), 0, m_values.size()-1);
    endInsertRows();
}
//----------------------------------------------------------
ManagerAccountsAmazon *ManagerAccountsAmazon::instance()
{
    static ManagerAccountsAmazon instance;
    return &instance;
}
//----------------------------------------------------------
void ManagerAccountsAmazon::onCustomerSelectedChanged(
        const QString &customerId)
{
    if (customerId.isEmpty()) {
        _clear();
    } else {
        loadFromSettings();
    }
}
//----------------------------------------------------------
AmazonAccounts ManagerAccountsAmazon::amazonAccount(const QString &amazon)
{
    AmazonAccounts accounts;
    QString amazonLower = amazon.toLower();
    if (m_values.contains(amazonLower)) {
        accounts.client = m_values[amazonLower][0];
        accounts.supplier = m_values[amazonLower][1];
        accounts.reserve = m_values[amazonLower][2];
        accounts.salesUnknown = m_values[amazonLower][3];
        accounts.fees = m_values[amazonLower][4];
    } else {
        /// TODO create exception
        Q_ASSERT(false);
    }
    return accounts;
}
//----------------------------------------------------------
void ManagerAccountsAmazon::addAmazon(const QString &amazon)
{
    addAmazon(amazon, "FAMAZON", "CC" + amazon, "4XX" + amazon, "4YY" + amazon, "6ZZ" + amazon);
}
//----------------------------------------------------------
void ManagerAccountsAmazon::addAmazon(
        const QString &amazon,
        const QString &accountCustomer,
        const QString &accountReserve,
        const QString &accountUnknownSales,
        const QString &fsupplier,
        const QString &accountCharges)
{
    m_values[amazon] = QStringList(
                {accountCustomer,
                 fsupplier,
                 accountReserve,
                 accountUnknownSales,
                 accountCharges});
    int index = m_values.keys().indexOf(amazon);
    beginInsertRows(QModelIndex(), index, index);
    saveInSettings();
    endInsertRows();
}
//----------------------------------------------------------
void ManagerAccountsAmazon::remove(
        const QModelIndex &index)
{
    beginRemoveRows(QModelIndex(), index.row(), index.row());
    QString amazon = index.siblingAtColumn(0).data().toString();
    m_values.remove(amazon);
    saveInSettings();
    endRemoveRows();
}
//----------------------------------------------------------
void ManagerAccountsAmazon::saveInSettings() const
{
    //return;
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!settingKey().isEmpty()) {
        QStringList lines;
        for (auto it=m_values.begin();
             it!=m_values.end();
             ++it) {
            QStringList elements;
            elements << it.key();
            for (int i=0; i<columnCount()-1; ++i) {
                elements << it.value()[i];
            }
            lines << elements.join(";;;");
        }
        settings.setValue(settingKey(), lines.join(":::"));
    }
}
//----------------------------------------------------------
void ManagerAccountsAmazon::loadFromSettings()
{
    //return; // TODO fix this
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!settingKey().isEmpty()) {
        if (settings.contains(settingKey())) {
            _clear();
            QString text = settings.value(settingKey()).toString();
            QStringList lines = text.split(":::");
            for (auto line:lines) {
                QStringList elements = line.split(";;;");
                QString amazon = elements.takeFirst();
                m_values[amazon] = elements;
            }
        }
    }
    if (m_values.size() > 0) {
        beginInsertRows(QModelIndex(), 0, m_values.size()-1);
        endInsertRows();
    }
}
//----------------------------------------------------------
QString ManagerAccountsAmazon::uniqueId() const
{
    return "ManagerAccountsAmazon";
}
//----------------------------------------------------------
QSet<QString> ManagerAccountsAmazon::allBalanceAccounts() const
{
    QSet<QString> accounts;
    for (auto it = m_values.begin(); it != m_values.end(); ++it) {
        accounts << it.value()[2];
    }
    return accounts;
}
//----------------------------------------------------------
QSet<QString> ManagerAccountsAmazon::allSupplierAccounts() const
{
    QSet<QString> accounts;
    for (auto it = m_values.begin(); it != m_values.end(); ++it) {
        accounts << it.value()[0];
    }
    return accounts;
}
//----------------------------------------------------------
QSet<QString> ManagerAccountsAmazon::allCustomerAccounts() const
{
    QSet<QString> accounts;
    for (auto it = m_values.begin(); it != m_values.end(); ++it) {
        accounts << it.value()[1];
    }
    return accounts;
}
//----------------------------------------------------------
QVariant ManagerAccountsAmazon::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList values
                = {"Amazon", tr("Client"), tr("Fournisseur"), tr("Balance"), tr("Ventes non identifiées"), tr("Frais")};
        return values[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int ManagerAccountsAmazon::rowCount(const QModelIndex &) const
{
    return m_values.size();
}
//----------------------------------------------------------
int ManagerAccountsAmazon::columnCount(const QModelIndex &) const
{
    return 6;
}
//----------------------------------------------------------
QVariant ManagerAccountsAmazon::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        QString amazon = m_values.keys()[index.row()];
        if (index.column() == 0) {
            return amazon;
        }
        return m_values[amazon][index.column()-1];
    }
    return QVariant();
}
//----------------------------------------------------------
Qt::ItemFlags ManagerAccountsAmazon::flags(const QModelIndex &index) const
{
    if (index.column() > 0) {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
//----------------------------------------------------------
bool ManagerAccountsAmazon::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        QString amazon = m_values.keys()[index.row()];
        m_values[amazon][index.column()-1] = value.toString();
        saveInSettings();
        return true;
    }
    return false;
}
//----------------------------------------------------------
