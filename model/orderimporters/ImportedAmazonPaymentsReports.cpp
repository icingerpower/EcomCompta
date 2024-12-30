#include <QtCore/qsettings.h>
#include <QtCore/qtextstream.h>

#include "ImportedAmazonPaymentsReports.h"
#include "model/CustomerManager.h"
#include "model/SettingManager.h"
#include "model/orderimporters/ImportedFileReportManager.h"
#include "model/orderimporters/ImporterYearsManager.h"
#include "model/orderimporters/OrderImporterAmazonUE.h"

//----------------------------------------------------------
ImportedAmazonPaymentsReports *ImportedAmazonPaymentsReports::instance()
{
    static ImportedAmazonPaymentsReports instance;
    return &instance;
}
//----------------------------------------------------------
ImportedAmazonPaymentsReports::~ImportedAmazonPaymentsReports()
{
    delete m_rootItem;
}
//----------------------------------------------------------
ImportedAmazonPaymentsReports::ImportedAmazonPaymentsReports(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new ImpAmPaymentNode("");
    m_fileSystemWatcher = new QFileSystemWatcher(this);
    QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
    if (!selectedCustomerId.isEmpty()) {
        onCustomerSelectedChanged(selectedCustomerId);
    }
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &ImportedAmazonPaymentsReports::onCustomerSelectedChanged);
    connect(m_fileSystemWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &ImportedAmazonPaymentsReports::onFileChanged);
}
//----------------------------------------------------------
void ImportedAmazonPaymentsReports::_findSubChannel(const QString &filePath)
{
    QString amazon = "amazon.??";
    CsvReader reader = OrderImporterAmazonUE().createAmazonReader(filePath);
    if (reader.readSomeLines(2)) {
        reader.removeFirstLine();
        auto dataRode = reader.dataRode();
        int indexAmazon = dataRode->header.pos("marketplace-name");
        QString value;
        while (value.isEmpty() && dataRode->lines.size() > 0) {
            auto elements = reader.takeFirstLine();
            value = elements[indexAmazon];
            if (!value.isEmpty()) {
                amazon = value;
                break;
            }
        }
    }
    m_filePathToAmazon[filePath] = amazon;
    // TODO create a system that will ask which amazon is it when can't guess
}
//----------------------------------------------------------
void ImportedAmazonPaymentsReports::onCustomerSelectedChanged(const QString &customerId)
{
    if (customerId.isEmpty()) {
        m_settingKey = "";
        emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
    } else {
        m_settingKey = "ImportedAmazonPaymentsReports-" + customerId;
        loadFromSettings();
        m_fileSystemWatcher->removePaths(m_fileSystemWatcher->directories());
        QString importer = OrderImporterAmazonUE().name();
        QString reportType = OrderImporterAmazonUE::REPORT_ORDERS_PAYMENTS;
        QDir paymentDir = SettingManager::instance()->reportDirectory(
                    importer, reportType);
        m_fileSystemWatcher->addPath(paymentDir.absolutePath());
    }
}
//----------------------------------------------------------
void ImportedAmazonPaymentsReports::onFileChanged(const QString &filePath)
{
    QStringList elements = filePath.split(QDir::separator());
    auto fileName = elements.takeLast();
    QString yearDir = elements.takeLast();
    QString reportName = elements.takeLast();
    QString importerName = elements.takeLast();
    if (reportName == OrderImporterAmazonUE::REPORT_ORDERS_PAYMENTS
            && importerName == OrderImporterAmazonUE().name()) {
        loadFromSettings();
    }
    /*
    if (QFile::exists(filePath)) { /// Added or renamed
        int idYear = 0;
        for (auto itemYear : m_rootItem->children()) {
            auto indexYear = index(idYear, 0, QModelIndex());
            ++idYear;
            int idAmazon = 0;
            for (auto itemAmazon : itemYear->children()) {
                auto indexAmazon = index(idAmazon, 0, indexYear);
                ++idAmazon;
                int idFile = 0;
                for (auto itemFile : itemAmazon->children()) {
                    auto indexFile = index(idFile, 0, indexAmazon);
                    if (!QFile::exists(itemFile->title())) { // TODO work from rel dirs
                        /// Renamed
                        m_filePathToAmazon[filePath]
                                = m_filePathToAmazon[itemFile->title()];
                        m_filePathToAmazon.remove(itemFile->title());
                        itemFile->setTitle(filePath);
                        saveInSettings();
                        emit dataChanged(indexFile, indexFile, {Qt::DisplayRole});
                        return;
                    }
                    ++idFile;
                }
            }
        }
        /// If we reach here it means the file is new
        _findSubChannel(filePath);
        ImportedFileReportManager::instance()->year()
    } else { /// file path deleted
        if (m_filePathToAmazon.contains(filePath)) {
            m_filePathToAmazon.remove(filePath);
        }
    }
    //*/
}
//----------------------------------------------------------
void ImportedAmazonPaymentsReports::saveInSettings() const
{
    /// Only save subchannel which is a missing information in ImportedFileReportManager
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!m_settingKey.isEmpty()) {
        if(m_filePathToAmazon.size() > 0) {
            QStringList values;
            for (auto it = m_filePathToAmazon.begin();
                 it != m_filePathToAmazon.end();
                 ++it) {
                if (QFile::exists(it.key())) {
                    values << it.key() + ":::" + it.value();
                }
            }
            settings.setValue(m_settingKey, values.join(";;;"));
        } else if (settings.contains(m_settingKey)) {
            settings.remove(m_settingKey);
        }
    }
}
//----------------------------------------------------------
void ImportedAmazonPaymentsReports::loadFromSettings()
{
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    delete m_rootItem;
    m_rootItem = new ImpAmPaymentNode("");
    m_filePathToAmazon.clear();
    endRemoveRows();
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(m_settingKey)) {
        QStringList values = settings.value(m_settingKey).toString().split(";;;");
        for (auto value : values) {
            QStringList elements = value.split(":::");
            m_filePathToAmazon[elements[0]] = elements[1];
        }
    }
    _generateTree();
}
//----------------------------------------------------------
void ImportedAmazonPaymentsReports::_generateTree()
{
    QString importer = OrderImporterAmazonUE().name();
    QString reportType = OrderImporterAmazonUE::REPORT_ORDERS_PAYMENTS;
    bool updatedFilePathToAmazon = false;
    for (auto year : ImporterYearsManager::instance()->years()) {
        QStringList filePaths
                = ImportedFileReportManager::instance()->filePaths(
                    importer,
                    reportType,
                    year);
        auto itemYear = new ImpAmPaymentNode(QString::number(year), m_rootItem);
        QHash<QString, ImpAmPaymentNodeAmazon*> itemsAmazon;
        for (auto filePath : filePaths) {
            if (!m_filePathToAmazon.contains(filePath)) {
                updatedFilePathToAmazon = true;
                _findSubChannel(filePath);
            }
            QString amazon = m_filePathToAmazon[filePath];
            if (!itemsAmazon.contains(amazon)) {
                itemsAmazon[amazon]
                        = new ImpAmPaymentNodeAmazon(
                            amazon, itemYear);
            }
            auto itemAmazon = itemsAmazon[amazon];
            QString fileName = QFileInfo(filePath).fileName();
            auto itemFile = new ImpAmPaymentNodeFile(
                        fileName, itemAmazon);
            (void) itemFile;
        }
    }
    if (updatedFilePathToAmazon) {
        saveInSettings();
    }
}
//----------------------------------------------------------
QVariant ImportedAmazonPaymentsReports::headerData(
        int, Qt::Orientation, int) const
{
    /*
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList values = {tr("Titre", "The differents fees and receipts in amazon payments"), tr("Valeur")};
        return values[section];
    }
    //*/
    return tr("Fichier", "amazon payment report file name");
}
//----------------------------------------------------------
Qt::ItemFlags ImportedAmazonPaymentsReports::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if (index.isValid()) {
        ImpAmPaymentNode *item = static_cast<ImpAmPaymentNode *>(index.internalPointer());
        if (dynamic_cast<ImpAmPaymentNodeFile *>(item) != nullptr) {
            flags |= Qt::ItemIsSelectable;
        }
    }
    return flags;
}
//----------------------------------------------------------
QModelIndex ImportedAmazonPaymentsReports::index(
        int row, int column, const QModelIndex &parent) const
{
    QModelIndex index;
    if (hasIndex(row, column, parent)) {
        ImpAmPaymentNode *item = nullptr;
        if (parent.isValid()) {
            ImpAmPaymentNode *itemParent
                    = static_cast<ImpAmPaymentNode *>(
                        parent.internalPointer());
            item = itemParent->child(row);;
        } else {
            item = m_rootItem->child(row);
        }
        index = createIndex(row, column, item);
    }
    return index;
}
//----------------------------------------------------------
QModelIndex ImportedAmazonPaymentsReports::parent(const QModelIndex &index) const
{
    QModelIndex parentIndex;
    if (index.isValid()) {
        ImpAmPaymentNode *item
                = static_cast<ImpAmPaymentNode *>(
                    index.internalPointer());
        if (item->parent() != nullptr) {
            parentIndex = createIndex(item->parent()->row(), 0, item->parent());
        }
    }
    return parentIndex;
}
//----------------------------------------------------------
int ImportedAmazonPaymentsReports::rowCount(const QModelIndex &parent) const
{
    ImpAmPaymentNode *itemParent = nullptr;
    if (parent.isValid()) {
        itemParent = static_cast<ImpAmPaymentNode *>(
                    parent.internalPointer());
    } else {
        itemParent = m_rootItem;
    }
    int count = itemParent->rowCount();
    return count;
}
//----------------------------------------------------------
int ImportedAmazonPaymentsReports::columnCount(const QModelIndex &) const
{
    return 1;
}
//----------------------------------------------------------
QVariant ImportedAmazonPaymentsReports::data(const QModelIndex &index, int role) const
{
    QVariant value;
    if ((role == Qt::DisplayRole || role == Qt::EditRole) && index.isValid()) {
        ImpAmPaymentNode *item
                = static_cast<ImpAmPaymentNode *>(
                    index.internalPointer());
        value = item->data(index.column(), role);
    }
    return value;
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
ImpAmPaymentNode::ImpAmPaymentNode(const QString &title, ImpAmPaymentNode *parent)
{
    m_row = 0;
    m_parent = parent;
    m_title = title;
    if (parent != nullptr) {
        m_row = parent->m_children.size();
        parent->m_children << this;
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
ImpAmPaymentNode::~ImpAmPaymentNode()
{
    qDeleteAll(m_children);
}
//----------------------------------------------------------
//----------------------------------------------------------
QVariant ImpAmPaymentNode::data(int column, int role) const
{
    if (role == Qt::DisplayRole && column == 0) {
            return m_title;
    }
    return QVariant();
}
//----------------------------------------------------------
//----------------------------------------------------------
ImpAmPaymentNode *ImpAmPaymentNode::child(int row) const
{
    return m_children[row];
}
//----------------------------------------------------------
//----------------------------------------------------------
int ImpAmPaymentNode::rowCount() const
{
    return m_children.size();
}
//----------------------------------------------------------
//----------------------------------------------------------
void ImpAmPaymentNode::_setRow(int row)
{
    m_row = row;
}
//----------------------------------------------------------
//----------------------------------------------------------
void ImpAmPaymentNode::removeChild(int row)
{
    auto child = m_children.takeAt(row);
    for (int i=row; i<m_children.size(); ++i) {
        m_children[i]->_setRow(i);
    }
    delete child;
}
//----------------------------------------------------------
//----------------------------------------------------------
int ImpAmPaymentNode::row() const
{
    return m_row;
}
//----------------------------------------------------------
//----------------------------------------------------------
ImpAmPaymentNode *ImpAmPaymentNode::parent() const
{
    return m_parent;
}
//----------------------------------------------------------
//----------------------------------------------------------
QList<ImpAmPaymentNode *> ImpAmPaymentNode::children() const
{
    return m_children;
}
//----------------------------------------------------------
//----------------------------------------------------------
QString ImpAmPaymentNode::title() const
{
    return m_title;
}
//----------------------------------------------------------
//----------------------------------------------------------
void ImpAmPaymentNode::setTitle(const QString &title)
{
    m_title = title;
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
ImpAmPaymentNodeYear::ImpAmPaymentNodeYear(
        const QString &title, ImpAmPaymentNode *parent)
    : ImpAmPaymentNode(title, parent)
{
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
ImpAmPaymentNodeAmazon::ImpAmPaymentNodeAmazon(
        const QString &title, ImpAmPaymentNode *parent)
    : ImpAmPaymentNode(title, parent)
{
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
ImpAmPaymentNodeFile::ImpAmPaymentNodeFile(
        const QString &title, ImpAmPaymentNode *parent)
    : ImpAmPaymentNode(title, parent)
{
}
/*
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
ImpAmPaymentNodeLine::ImpAmPaymentNodeLine(
        const QString &title,
        const QString &account,
        double value,
        ImpAmPaymentNode *parent)
    : ImpAmPaymentNode(title, parent)
{
    m_account = account;
    m_value = value;
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
QVariant ImpAmPaymentNodeLine::data(int column, int role) const
{
    if (role == Qt::DisplayRole) {
        if (column == 1) {
            return m_account;
        } else if (column == 2) {
            return valueString();
        }
    }
    return ImpAmPaymentNode::data(column, role);
}
//----------------------------------------------------------
//----------------------------------------------------------
QString ImpAmPaymentNodeLine::account() const
{
    return m_account;
}
//----------------------------------------------------------
//----------------------------------------------------------
void ImpAmPaymentNodeLine::setAccount(const QString &account)
{
    m_account = account;
}
//----------------------------------------------------------
//----------------------------------------------------------
double ImpAmPaymentNodeLine::value() const
{
    return m_value;
}
//----------------------------------------------------------
//----------------------------------------------------------
QString ImpAmPaymentNodeLine::valueString() const
{
    return QString::number(m_value, 'f', 2);
}
//----------------------------------------------------------
//----------------------------------------------------------
void ImpAmPaymentNodeLine::setValue(double value)
{
    m_value = value;
}
//----------------------------------------------------------
//----------------------------------------------------------
ImpAmPaymentNodeDetails::ImpAmPaymentNodeDetails(
        const QString &title,
        const QString &account,
        double value,
        ImpAmPaymentNode *parent)
{
}
//----------------------------------------------------------
//----------------------------------------------------------
//*/
