#include <QtCore/qhash.h>
#include <QtCore/qsettings.h>

#include "model/SettingManager.h"

#include "AmazonFulfillmentAddressModel.h"

//----------------------------------------------------------
FBACenterItem FBACenterItem::parentKnown;
FBACenterItem FBACenterItem::childKnown;
FBACenterItem FBACenterItem::parentAdded;
FBACenterItem FBACenterItem::childAdded;
//----------------------------------------------------------
AmazonFulfillmentAddressModel::AmazonFulfillmentAddressModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    _loadKnownCenters();
    loadFromSettings();
}
//----------------------------------------------------------
AmazonFulfillmentAddressModel *AmazonFulfillmentAddressModel::instance()
{
    static AmazonFulfillmentAddressModel instance;
    return &instance;
}
//----------------------------------------------------------
AmazonFulfillmentAddressModel::~AmazonFulfillmentAddressModel()
{
}
//----------------------------------------------------------
QStringList AmazonFulfillmentAddressModel::countriesUE()
{
    return {"FR", "DE", "IT", "ES", "PL", "CZ", "NL", "SE"}; //TODO add all countries
}
//----------------------------------------------------------
bool AmazonFulfillmentAddressModel::contains(const QString &name) const
{
    return m_centersByCode.contains(name) || m_addedCentersByCode.contains(name);
}
//----------------------------------------------------------
Address AmazonFulfillmentAddressModel::getAddress(
        const QString &centerName) const
{
    Address address;
    if (m_centersByCode.contains(centerName)) {
        address = m_centersByCode[centerName];
    } else if (m_centersByCode.contains(centerName)) {
        address = m_addedCentersByCode[centerName];
    }
    return address;
}
//----------------------------------------------------------
void AmazonFulfillmentAddressModel::add(const Address &address)
{
    QModelIndex parentIndex = createIndex(0, 0, &FBACenterItem::parentAdded);
    beginInsertRows(parentIndex, 0, 0);
    m_addedAddresses.insert(0, address);
    m_addedCentersByCode[address.fullName()] = address;
    saveInSettings();
    endInsertRows();
}
//----------------------------------------------------------
void AmazonFulfillmentAddressModel::remove(int index)
{
    QModelIndex parentIndex = createIndex(0, 0, &FBACenterItem::parentAdded);
    beginRemoveRows(parentIndex, index, index);
    m_addedCentersByCode.remove(m_addedAddresses[index].fullName());
    m_addedAddresses.removeAt(index);
    saveInSettings();
    endRemoveRows();
}
//----------------------------------------------------------
void AmazonFulfillmentAddressModel::saveInSettings() const
{
    QStringList elements;
    for (auto address : m_addedAddresses) {
        elements << address.toString();
    }
    QString centersString = elements.join(",,,");
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (m_addedAddresses.size() == 0) {
        if (settings.contains("fbaCenterAddresses")) {
            settings.remove("fbaCenterAddresses");
        }
    } else {
        settings.setValue("fbaCenterAddresses", centersString);
    }
}
//----------------------------------------------------------
void AmazonFulfillmentAddressModel::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains("fbaCenterAddresses")) {
        QString string = settings.value("fbaCenterAddresses").toString();
        if (!string.isEmpty()) {
            QStringList elements = string.split(",,,");
            QModelIndex parentIndex = createIndex(0, 0, &FBACenterItem::parentAdded);
            beginInsertRows(parentIndex, 0, elements.size()-1);
            for (auto addressString : elements) {
                Address address = Address::fromString(addressString);
                m_addedAddresses << address;
                m_addedCentersByCode[address.fullName()] = address;
            }
            endInsertRows();
        }
    }
}
//----------------------------------------------------------
QModelIndex AmazonFulfillmentAddressModel::parent(
        const QModelIndex &index) const
{
    QModelIndex parentIndex;
    if (index.isValid()) {
        FBACenterItem *item
                = static_cast<FBACenterItem *>(
                    index.internalPointer());
        if (item == &FBACenterItem::childAdded) {
            parentIndex = createIndex(0, 0, &FBACenterItem::parentAdded);
        } else if (item == &FBACenterItem::childKnown) {
            parentIndex = createIndex(1, 0, &FBACenterItem::parentKnown);
        }
    }
    return parentIndex;
}
//----------------------------------------------------------
QModelIndex AmazonFulfillmentAddressModel::index(
        int row, int column, const QModelIndex &parent) const
{
    /*
    auto pk = &FBACenterItem::parentKnown;
    auto ck = &FBACenterItem::childKnown;
    auto pa = &FBACenterItem::parentAdded;
    auto ca = &FBACenterItem::childAdded;
    if (pk || ck || pa || ca) {
    }
    int aa= 10;++aa;
    //*/
    QModelIndex index;
    if (hasIndex(row, column, parent)) {
        FBACenterItem *item = nullptr;
        if (parent.isValid()) {
            FBACenterItem *parentItem
                    = static_cast<FBACenterItem *>(
                        parent.internalPointer());
            item = parentItem == &FBACenterItem::parentAdded ?
                        &FBACenterItem::childAdded : &FBACenterItem::childKnown;
        } else {
            if (row == 0) {
                item = &FBACenterItem::parentAdded;
            } else {
                item = &FBACenterItem::parentKnown;
            }
        }
        index = createIndex(row, column, item);
    }
    return index;
}
//----------------------------------------------------------
Qt::ItemFlags AmazonFulfillmentAddressModel::flags(
        const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    QModelIndex parentIndex = index.parent();
    if (parentIndex.isValid()) {
        FBACenterItem *parentItem
                = static_cast<FBACenterItem *>(
                    parentIndex.internalPointer());
        if (parentItem == &FBACenterItem::parentAdded) {
            flags |= Qt::ItemIsSelectable;
            if (index.column() > 0) {
                flags |= Qt::ItemIsEditable;
            }
        }
    }
    return flags;
}
//----------------------------------------------------------
int AmazonFulfillmentAddressModel::rowCount(
        const QModelIndex &parent) const
{
    int count = 0;
    if (parent.isValid()) {
        FBACenterItem *parentItem
                = static_cast<FBACenterItem *>(
                    parent.internalPointer());
        if (parentItem == &FBACenterItem::parentAdded) {
            count = m_addedAddresses.length();
        } else if (parentItem == &FBACenterItem::parentKnown) {
            count = m_addresses.length();
        }
    } else {
        count = 2;
    }
    return count;
}
//----------------------------------------------------------
int AmazonFulfillmentAddressModel::columnCount(const QModelIndex &) const
{
    return 4; /// Name, country, postal code, city
}
//----------------------------------------------------------
QVariant AmazonFulfillmentAddressModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    static QHash<int, QString> header
            = {{0, "Code Du Centre FBA"}, {1,"Pays"}, {2,"Code postal"}, {3,"Ville"}};
    QVariant variant;
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            variant = header[section];
        }
    }
    return variant;
}
//----------------------------------------------------------
QVariant AmazonFulfillmentAddressModel::data(
        const QModelIndex &index, int role) const
{
    QVariant variant;
    if (role == Qt::DisplayRole) {
        FBACenterItem *item
                = static_cast<FBACenterItem *>(
                    index.internalPointer());
        if (item == &FBACenterItem::parentAdded
                && index.column() == 0) {
            variant = "Centres FBA ajoutés";
        } else if (item == &FBACenterItem::parentKnown
                   && index.column() == 0) {
            variant = "Centres FBA connus";
        } else if (item == &FBACenterItem::childAdded) {
            if (index.column() == 0) {
                variant = m_addedAddresses[index.row()].fullName();
            } else if (index.column() == 1) {
                variant = m_addedAddresses[index.row()].countryCode();
            } else if (index.column() == 2) {
                variant = m_addedAddresses[index.row()].postalCode();
            } else if (index.column() == 3) {
                variant = m_addedAddresses[index.row()].city();
            }
        } else if (item == &FBACenterItem::childKnown) {
            if (index.column() == 0) {
                variant = m_addresses[index.row()].fullName();
            } else if (index.column() == 1) {
                variant = m_addresses[index.row()].countryCode();
            } else if (index.column() == 2) {
                variant = m_addresses[index.row()].postalCode();
            } else if (index.column() == 3) {
                variant = m_addresses[index.row()].city();
            }
        }
    } else if (role == Qt::EditRole) {
        FBACenterItem *item
                = static_cast<FBACenterItem *>(
                    index.internalPointer());
        if (item == &FBACenterItem::childAdded) {
            if (index.column() == 1) {
                static QStringList allCountries = countriesUE();
                variant = allCountries;
                // TODO doesn't work…I need to look in createeditor as explained here https://wiki.qt.io/Combo_Boxes_in_Item_Views
            } else if (index.column() == 2) {
                variant = m_addedAddresses[index.row()].postalCode();
            } else if (index.column() == 3) {
                variant = m_addedAddresses[index.row()].city();
            }
        }
    }
    return variant;
}
//----------------------------------------------------------
bool AmazonFulfillmentAddressModel::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    bool change = false;
    if (role == Qt::EditRole) {
        FBACenterItem *item
                = static_cast<FBACenterItem *>(
                    index.internalPointer());
        if (item == &FBACenterItem::childAdded) {
            if (index.column() == 1) {
                m_addedAddresses[index.row()].setCountryCode(value.toString());
                change = true;
            } else if (index.column() == 2) {
                m_addedAddresses[index.row()].setPostalCode(value.toString());
                change = true;
            } else if (index.column() == 3) {
                m_addedAddresses[index.row()].setCity(value.toString());
                change = true;
            }
            if (change) {
                m_addedCentersByCode[m_addedAddresses[index.row()].fullName()]
                        = m_addedAddresses[index.row()];
                saveInSettings();
            }
        }
    }
    return change;
}
//----------------------------------------------------------
void AmazonFulfillmentAddressModel::_loadKnownCenters()
{
    // TODO get country of unavailable fulfillment centers from VAT report
    m_addresses << Address::fromFbaInfos("OVD1","ES","","");
    m_centersByCode["OVD1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("PAD2","DE","","");
    m_centersByCode["PAD2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("NCL1","UK","","");
    m_centersByCode["NCL1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("NCL2","UK","","");
    m_centersByCode["NCL2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BRS2","UK","","");
    m_centersByCode["BRS2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("DSA6","UK","","");
    m_centersByCode["DSA6"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("ERF1","DE","","");
    m_centersByCode["ERF1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BRQ2","CZ","","");
    m_centersByCode["BRQ2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BRE2","FR","","");
    m_centersByCode["BRE2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BCN4","ES","","");
    m_centersByCode["BCN4"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("PSR2","IT","","");
    m_centersByCode["PSR2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("SCN2","DE","","");
    m_centersByCode["SCN2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XFRS","FR","","");
    m_centersByCode["XFRS"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XFRO","FR","","");
    m_centersByCode["XFRO"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("VLC1","ES","","");
    m_centersByCode["VLC1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XPLE","DE","","");
    m_centersByCode["XPLE"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XPO1","DE","","");
    m_centersByCode["XPO1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LEJ5","DE","","");
    m_centersByCode["LEJ5"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("NUE1","DE","","");
    m_centersByCode["NUE1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XPLD","FR","","");
    m_centersByCode["XPLD"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("SES1","ES","","");
    m_centersByCode["SES1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("RMU1","ES","","");
    m_centersByCode["RMU1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BGY1","IT","","");
    m_centersByCode["BGY1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MXP6","IT","","");
    m_centersByCode["MXP6"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("POZ2","PL","Chociule","");
    m_centersByCode["POZ2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MAN1","UK","Manchester","");
    m_centersByCode["MAN1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BER3","DE","Brieselang","");
    m_centersByCode["BER3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BTS2","SK","Bratislava","");
    m_centersByCode["BTS2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BRE4","DE","Achim","");
    m_centersByCode["BRE4"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("CGN1","DE","Koblenz","");
    m_centersByCode["CGN1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("DTM1","DE","Werne","");
    m_centersByCode["DTM1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("DTM2","DE","Dortmund","");
    m_centersByCode["DTM2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("DTM3","DE","Dortmund","");
    m_centersByCode["DTM3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("DUS2","DE","Rheinberg","");
    m_centersByCode["DUS2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("DUS4","DE","Moenchengladbach","");
    m_centersByCode["DUS4"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("EDEA","DE","Dortmund","");
    m_centersByCode["EDEA"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("EDE4","DE","Werne","");
    m_centersByCode["EDE4"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("EDE5","DE","Werne","");
    m_centersByCode["EDE5"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("FRA1","DE","Bad Hersfeld","");
    m_centersByCode["FRA1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("FRA3","DE","Bad Hersfeld","");
    m_centersByCode["FRA3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("FRA7","DE","Frankenthal Pfalz","");
    m_centersByCode["FRA7"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("HAM2","DE","Winsen an der Luhe","");
    m_centersByCode["HAM2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("KTW1","PL","Sosnowiec","");
    m_centersByCode["KTW1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("KTW3","PL","Bojkowska","");
    m_centersByCode["KTW3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LCJ2","PL","Pawlikowice","");
    m_centersByCode["LCJ2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LCJ3","PL","Łódź","");
    m_centersByCode["LCJ3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LCJ4","PL","Łódź","");
    m_centersByCode["LCJ4"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LEJ1","DE","Leipzig","");
    m_centersByCode["LEJ1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LEJ3","DE","Suelzetal","");
    m_centersByCode["LEJ3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MUC3","DE","Graben","");
    m_centersByCode["MUC3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("PAD1","DE","Oelde","");
    m_centersByCode["PAD1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("POZ1","PL","Poznan","");
    m_centersByCode["POZ1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("PRG1","CZ","Dobroviz","");
    m_centersByCode["PRG1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("PRG2","CZ","Dobroviz","");
    m_centersByCode["PRG2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("STR1","DE","Pforzheim","");
    m_centersByCode["STR1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("SZZ1","PL","Kołbaskowo","");
    m_centersByCode["SZZ1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("WRO1","PL","Bielany Wroclawskie","");
    m_centersByCode["WRO1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("WRO2","PL","Bielany Wroclawskie","");
    m_centersByCode["WRO2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("WRO5","PL","Okmiany","");
    m_centersByCode["WRO5"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XDU1","DE","Malsfeld","");
    m_centersByCode["XDU1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XDET","DE","Malsfeld","");
    m_centersByCode["XDET"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XDU2","DE","Oberhausen","");
    m_centersByCode["XDU2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XDEZ","DE","Oberhausen","");
    m_centersByCode["XDEZ"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XFR1","DE","Hammersbach","");
    m_centersByCode["XFR1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XDEY","DE","Hammersbach","");
    m_centersByCode["XDEY"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XFR2","DE","Rennerod","");
    m_centersByCode["XFR2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XDEH","DE","Rennerod","");
    m_centersByCode["XDEH"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XFR3","DE","Michelstadt","");
    m_centersByCode["XFR3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XDEW","DE","Michelstadt","");
    m_centersByCode["XDEW"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XHA1","DE","Neu Wulmsdorf","");
    m_centersByCode["XHA1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XSC1","DE","Kaiserslautern","");
    m_centersByCode["XSC1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XDEQ","DE","Kaiserslautern","");
    m_centersByCode["XDEQ"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XWR1","PL","Krajków","");
    m_centersByCode["XWR1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XPLA","PL","Krajków","");
    m_centersByCode["XPLA"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("ORY1","FR","Saran","");
    m_centersByCode["ORY1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("ORY4","FR","Brétigny","");
    m_centersByCode["ORY4"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MRS1","FR","Montelimar","");
    m_centersByCode["MRS1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LYS1","FR","Sevrey","");
    m_centersByCode["LYS1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LIL1","FR","Lauwin","");
    m_centersByCode["LIL1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BVA1","FR","Boves","");
    m_centersByCode["BVA1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("CDG7","FR","Senlis","");
    m_centersByCode["CDG7"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XVA1","FR","Bussy-Lettree","");
    m_centersByCode["XVA1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XFRZ","FR","Bussy-Lettree","");
    m_centersByCode["XFRZ"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("ETZ2","FR","Augny","");
    m_centersByCode["ETZ2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("VESK","FR","Savigny Le Temple","");
    m_centersByCode["VESK"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XFRJ","FR","Savigny Le Temple","");
    m_centersByCode["XFRJ"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XOR6","FR","Lisses","");
    m_centersByCode["XOR6"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XFRK","FR","Lisses","");
    m_centersByCode["XFRK"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XOR2","FR","Satolas-et-Bonce","");
    m_centersByCode["XOR2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XFRE","FR","Satolas-et-Bonce","");
    m_centersByCode["XFRE"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XOS1","FR","Brebieres","");
    m_centersByCode["XOS1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XFRL","FR","Brebieres","");
    m_centersByCode["XFRL"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("TRN1","IT","Torrazza Piemonte","");
    m_centersByCode["TRN1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BLQ1","IT","San Bellino","");
    m_centersByCode["BLQ1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("FCO1","IT","Passo Corese","");
    m_centersByCode["FCO1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("FCO2","IT","Colleferro","");
    m_centersByCode["FCO2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MXP5","IT","Castel San Giovanni","");
    m_centersByCode["MXP5"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MXP3","IT","Vercelli","");
    m_centersByCode["MXP3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XITC","IT","Carpiano","");
    m_centersByCode["XITC"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XMP2","IT","Carpiano","");
    m_centersByCode["XMP2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XITD","IT","Rovigo","");
    m_centersByCode["XITD"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XMP1","IT","Rovigo","");
    m_centersByCode["XMP1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XITG","IT","Carpiano","");
    m_centersByCode["XITG"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XLI1","IT","Carpiano","");
    m_centersByCode["XLI1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XITI","IT","Marzano","");
    m_centersByCode["XITI"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XLI3","IT","Marzano","");
    m_centersByCode["XLI3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XITF","IT","Piacenza","");
    m_centersByCode["XITF"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("VEII ","IT","Piacenza","");
    m_centersByCode["VEII "] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MAD4","ES","Madrid","");
    m_centersByCode["MAD4"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MAD6","ES","Illescas","");
    m_centersByCode["MAD6"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MAD7","ES","Illescas","");
    m_centersByCode["MAD7"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MAD9","ES","Alcalá de Henares","");
    m_centersByCode["MAD9"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("PESA","ES","Toledo","");
    m_centersByCode["PESA"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BCN1","ES","Barcelona","");
    m_centersByCode["BCN1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BCN2","ES","Martorelles","");
    m_centersByCode["BCN2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BCN3","ES","Castellbisbal","");
    m_centersByCode["BCN3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XMA1","ES","XESA","");
    m_centersByCode["XMA1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XMA2","ES","XESE","");
    m_centersByCode["XMA2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XMA3","ES","XESF","");
    m_centersByCode["XMA3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XRE1","ES","XESC","");
    m_centersByCode["XRE1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("SVQ1","ES"," Sevilla","");
    m_centersByCode["SVQ1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XAR1","SE","Eskilstuna (SE)","");
    m_centersByCode["XAR1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LTN1","UK","Marston Gate","");
    m_centersByCode["LTN1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LTN2","UK","Hemel Hempstead","");
    m_centersByCode["LTN2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LTN4","UK","Dunstable","");
    m_centersByCode["LTN4"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LTN7","UK","Bedford","");
    m_centersByCode["LTN7"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LTN9","UK","Dunstable","");
    m_centersByCode["LTN9"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BHX1","UK","Rugeley","");
    m_centersByCode["BHX1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BHX2","UK","Coalville","");
    m_centersByCode["BHX2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BHX3","UK","Daventry","");
    m_centersByCode["BHX3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BHX4","UK","Coventry","");
    m_centersByCode["BHX4"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BHX5","UK","Rugby","");
    m_centersByCode["BHX5"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BHX7","UK","Hinckley","");
    m_centersByCode["BHX7"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("BRS1","UK","Bristol","");
    m_centersByCode["BRS1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("CWL1","UK","Swansea","");
    m_centersByCode["CWL1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("EDI4","UK","Dunfermline","");
    m_centersByCode["EDI4"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("EMA1","UK","Derby","");
    m_centersByCode["EMA1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("EMA2","UK","Mansfield","");
    m_centersByCode["EMA2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("EMA3","UK","Nottingham","");
    m_centersByCode["EMA3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("EUK5","UK","Peterborough","");
    m_centersByCode["EUK5"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("GLA1","UK","Gourock","");
    m_centersByCode["GLA1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LBA1","UK","Doncaster","");
    m_centersByCode["LBA1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LBA2","UK","Doncaster","");
    m_centersByCode["LBA2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LBA3","UK","Doncaster","");
    m_centersByCode["LBA3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LBA4","UK","Doncaster","");
    m_centersByCode["LBA4"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MAN1","UK","Manchester","");
    m_centersByCode["MAN1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MAN2","UK","Warrington","");
    m_centersByCode["MAN2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MAN3","UK","Bolton","");
    m_centersByCode["MAN3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MAN4","UK","Barlborough","");
    m_centersByCode["MAN4"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MME1","UK","Darlington","");
    m_centersByCode["MME1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("MME2","UK","Bowburn","");
    m_centersByCode["MME2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LCY2","UK","Tilbury","");
    m_centersByCode["LCY2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("LCY3","UK","Dartford","");
    m_centersByCode["LCY3"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XLT1","UK","Peterborough","");
    m_centersByCode["XLT1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("EUKA","UK","Peterborough","");
    m_centersByCode["EUKA"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XLT2","UK","Peterborough","");
    m_centersByCode["XLT2"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("EUKB","UK","Peterborough","");
    m_centersByCode["EUKB"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("Xpl1","Uk","Widnes","");
    m_centersByCode["Xpl1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("EUKD","Uk","Widnes","");
    m_centersByCode["EUKD"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XUKA","UK"," Runcorn","");
    m_centersByCode["XUKA"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XUKD","UK","Daventry","");
    m_centersByCode["XUKD"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XBH1","UK","Daventry","");
    m_centersByCode["XBH1"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XUKN","UK","Rugby","");
    m_centersByCode["XUKN"] = m_addresses.last();
    m_addresses << Address::fromFbaInfos("XBH2","UK","Rugby","");
    m_centersByCode["XBH2"] = m_addresses.last();
    std::sort(m_addresses.begin(), m_addresses.end());
}
//----------------------------------------------------------

