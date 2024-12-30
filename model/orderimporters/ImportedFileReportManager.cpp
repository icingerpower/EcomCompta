#include <QtCore/qsettings.h>
#include <QtCore/qdebug.h>
#include <QtCore/qtimer.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qset.h>

#include "ImportedFileReportManager.h"
#include "ImporterYearsManager.h"
#include "AbstractOrderImporter.h"
#include "model/SettingManager.h"
#include "model/CustomerManager.h"

//----------------------------------------------------------
ImportedFileReportManager::ImportedFileReportManager(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new ImpReportNode("");
    //m_fileSystemWatcher = new QFileSystemWatcher(this);
    QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
    if (!selectedCustomerId.isEmpty()) {
        onCustomerSelectedChanged(selectedCustomerId);
    }
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &ImportedFileReportManager::onCustomerSelectedChanged);
    QDir reportDir = SettingManager::instance()->reportDirectory();
    /*
    connect(m_fileSystemWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &ImportedFileReportManager::onFileChanged);
            //*/
}
/*
//----------------------------------------------------------
void ImportedFileReportManager::onFileChanged(const QString &filePath)
{
    if (QFile::exists(filePath)) { /// Added or renamed
        int idYear = 0;
        for (auto itemYear : m_rootItem->children()) {
            ++idYear;
            int idImporter = 0;
            QString yearDir = itemYear->value();
            for (auto itemImporter : itemYear->children()) {
                ++idImporter;
                QString importerName = itemImporter->value();
                int idReport = 0;
                for (auto itemReport : itemImporter->children()) {
                    ++idReport;
                    int idFile = 0;
                    QString reportName = itemReport->value();
                    for (auto itemFile : itemReport->children()) {
                        QDir reportDir
                                = SettingManager::instance()->reportDirectory(
                                    importerName, reportName, yearDir);
                        QString itemFilePath = reportDir.filePath(itemFile->value());
                        if (!QFile::exists(itemFilePath)) { /// Renamed
                            auto indexYear = index(idYear, 0, QModelIndex());
                            auto indexImporter = index(idImporter, 0, indexYear);
                            auto indexReport = index(idReport, 0, indexImporter);
                            auto indexFile = index(idFile, 0, indexReport);
                            QFileInfo fileInfoNew(filePath);
                            QString fileNameNew = fileInfoNew.fileName();
                            QString fileNameOld = itemFile->value();
                            itemFile->setValue(fileNameNew);
                            // TODO check if bug when removing several files in same time?
                            emit dataChanged(indexFile, indexFile, {Qt::DisplayRole});
                            emit fileRenamed(yearDir,
                                             importerName,
                                             reportName,
                                             itemFile->value(),
                                             fileNameNew);
                            return;
                        }
                        ++idFile;
                    }
                }
            }
        }
        /// If we reach here it means the file is new
        QStringList elements = filePath.split(QDir::separator());
        QString fileNameNew = elements.takeLast();
        QString yearDir = elements.takeLast();
        QString reportName = elements.takeLast();
        QString importerName = elements.takeLast();
        ImpReportNode *nodeYear;
        ImpReportNode *nodeImporter;
        ImpReportNode *nodeReport;
        if (!m_rootItem->contains(yearDir)) {
            nodeYear = new ImpReportNodeYear(yearDir, m_rootItem);
        } else {
            nodeYear = m_rootItem->child(yearDir) ;
        }
        if (!nodeYear->contains(importerName)) {
            nodeImporter = new ImpReportNodeImporter(importerName, nodeYear);
        } else {
            nodeImporter = nodeYear->child(importerName);
        }
        if (!nodeReport->contains(reportName)) {
            nodeReport = new ImpReportNodeReportType(reportName, nodeImporter);
        } else {
            nodeReport = nodeYear->child(reportName);
        }
        auto nodeFile = new ImpReportNodeFile(dateBegin, dateEnd, fileName, nodeReport);
    } else { /// file path deleted
        QStringList elements = filePath.split(QDir::separator());
        QString fileNameDeleted = elements.takeLast();
        QString yearDir = elements.takeLast();
        QString reportName = elements.takeLast();
        QString importerName = elements.takeLast();
        auto itemYear = m_rootItem->child(yearDir);
        if (itemYear->contains(importerName)) {
            auto itemImporter = itemYear->child(importerName);
            if (itemImporter->contains(reportName)) {
                auto itemReport = itemImporter->child(reportName);
                if (itemReport->contains(fileNameDeleted)) {
                    auto itemFile = itemReport->child(fileNameDeleted);
                    //QDir reportDir = SettingManager::instance()->reportDirectory(
                                //itemImporter->value(), itemReport->value());
                    //reportDir.cd(itemYear->value());
                    //if (reportDir.entryList(QDir::Files).size() == 0) {
                        //reportDir.removeRecursively();
                    //}
                    QModelIndex indexYear = index(itemYear->row(), 0, QModelIndex());
                    QModelIndex indexImporter = index(itemImporter->row(), 0, indexYear);
                    QModelIndex indexReport = index(itemReport->row(), 0, indexImporter);
                    beginRemoveRows(indexReport, itemFile->row(), itemFile->row());
                    itemReport->removeChild(fileNameDeleted);
                    endRemoveRows();
                }
            }
        }
    }
    saveInSettings();
}
//*/
//----------------------------------------------------------
void ImportedFileReportManager::onFileRenamed(
        const QString &path, const QString &oldName, const QString &newName)
{
    QStringList elements = path.split(QDir::separator());
    QString yearDir = elements.takeLast();
    QString report = elements.takeLast();
    QString importer = elements.takeLast();
    auto node = ImpReportNode::nodeFile(yearDir, importer, report, oldName);
    node->setValue(newName);
    QModelIndex indexYear = index(node->parent()->parent()->parent()->row(), 0, QModelIndex());
    QModelIndex indexImporter = index(node->parent()->parent()->row(), 0, indexYear);
    QModelIndex indexReport = index(node->parent()->row(), 0, indexImporter);
    QModelIndex indexFile = index(node->row(), 0, indexReport);
    saveInSettings();
    emit dataChanged(indexFile, indexFile, {Qt::DisplayRole});
}
//----------------------------------------------------------
    /*
void ImportedFileReportManager::onRowsRemoved(
        const QModelIndex &parent, int first, int last)
{
    //TODO remove what doesn't exist anymore
    if (parent.parent().parent().isValid()) {
        QString dirPath = parent.data().toString();
        QModelIndex parentRec = parent.parent();
        while (parentRec.isValid()) {
            dirPath = parentRec.data().toString() + QDir::separator();
            parentRec = parentRec.parent();
        }
        QDir reportDir = SettingManager::instance()->reportDirectory();
        if (dirPath.startsWith(reportDir.absolutePath())) {
            QString yearDir = parent.data().toString();
            QString report = parent.parent().data().toString();
            QString importer = parent.parent().parent().data().toString();
            auto node = ImpReportNode::nodeReport(yearDir, importer, report);
            for (int i=last; i>= first; --i) {
                node->removeChild(i);
            }
            saveInSettings();
        }
    }
}
    //*/
//----------------------------------------------------------
ImportedFileReportManager *ImportedFileReportManager::instance()
{
    static ImportedFileReportManager instance;
    return &instance;
}
//----------------------------------------------------------
ImportedFileReportManager::~ImportedFileReportManager()
{
    delete m_rootItem;
}
//----------------------------------------------------------
QSharedPointer<OrdersMapping> ImportedFileReportManager::loadOrders(
        const QString &importerName,
        const QDate &dateMin,
        const QDate &dateMax) const
{
    QSharedPointer<OrdersMapping> allOrders;
    auto importer = AbstractOrderImporter::importer(importerName);
    QString yearMin = QString::number(dateMin.year());
    QString yearMax = QString::number(dateMax.year());
    for (auto nodeYear : m_rootItem->children()) {
        QString yearDir = nodeYear->value();
        if ((yearDir.contains(yearMax) || yearDir.contains(yearMin))
                && nodeYear->contains(importerName)) {
            auto nodeImporter = nodeYear->child(importerName);
            for (auto nodeReport : nodeImporter->children()) {
                QString reportName = nodeReport->value();
                QDir reportDir = SettingManager::instance()->reportDirectory(
                            importerName, reportName, yearDir);
                for (auto node : nodeReport->children()) {
                    ImpReportNodeFile *nodeFile
                            = static_cast<ImpReportNodeFile*>(node);
                    if (nodeFile->dateBegin() <= dateMax && nodeFile->dateEnd() >= dateMin) {
                        QString fileName = nodeFile->value();
                        QString filePath = reportDir.filePath(fileName);
                        auto newOrders = importer->loadReport(
                                    reportName, filePath, dateMax.year());
                        if (allOrders.isNull()) {
                            allOrders = newOrders;
                        } else {
                            allOrders->unite(*newOrders.data());
                        }
                    }
                }
            }
        }
    }
    return allOrders;
}
//----------------------------------------------------------
void ImportedFileReportManager::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!m_settingKey.isEmpty()) {
        if (m_rootItem->children().size() > 0) {
            QStringList lines;
            for (auto itemYear : m_rootItem->children()) {
                for (auto itemImporter : itemYear->children()) {
                    for (auto itemReport : itemImporter->children()) {
                        for (auto itemFile : itemReport->children()) {
                            auto itemFileCasted = static_cast<ImpReportNodeFile *>(itemFile);
                            QStringList lineElements
                                    = {itemYear->value(),
                                       itemImporter->value(),
                                       itemReport->value(),
                                       itemFile->value(),
                                       itemFileCasted->dateBegin().toString("yyyy-MM-dd"),
                                       itemFileCasted->dateEnd().toString("yyyy-MM-dd")};
                            lines << lineElements.join(";;;");
                        }
                    }
                }
            }
            settings.setValue(m_settingKey, lines.join(":::"));
        } else if (settings.contains(m_settingKey)) {
            settings.remove(m_settingKey);
        }
    }
}
//----------------------------------------------------------
void ImportedFileReportManager::loadFromSettings()
{
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    delete m_rootItem;
    m_rootItem = new ImpReportNode("");
    endRemoveRows();
    QSet<QString> filePathsDone;
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!m_settingKey.isEmpty() && settings.contains(m_settingKey)) {
        QString linesString = settings.value(m_settingKey).toString();
        QStringList lines = linesString.split(":::");
        int idYear = 0;
        int idImporter = 0;
        int idReport = 0;
        int idFile = 0;
        for (auto line : lines) {
            auto lineElements = line.split(";;;");
            QString year = lineElements[0];
            QString importer = lineElements[1];
            QString report = lineElements[2];
            QString fileName = lineElements[3];
            if (year.contains("-")) {
                QStringList years = year.split("-");
                for (auto yearInt : years) {
                    ImporterYearsManager::instance()->recordYear(yearInt.toInt());
                }
            } else {
                ImporterYearsManager::instance()->recordYear(year.toInt());
            }
            QDir reportDirWithYear
                    = SettingManager::instance()->reportDirectory(
                        importer, report, year);
            //Q_ASSERT(!fileNamesDone.contains(fileName));
            QString filePath = reportDirWithYear.filePath(fileName);
            if (reportDirWithYear.exists(fileName)
                && !filePathsDone.contains(filePath) ) { /// We only load the file if it exists
                filePathsDone << filePath;
                QDate dateBegin = QDate::fromString(lineElements[4], "yyyy-MM-dd");
                QDate dateEnd = QDate::fromString(lineElements[5], "yyyy-MM-dd");
                ImpReportNode *nodeYear = nullptr;
                ImpReportNode *nodeImporter = nullptr;
                ImpReportNode *nodeReport = nullptr;
                ImpReportNode *nodeFile = nullptr;
                if (!m_rootItem->contains(year)) {
                    nodeYear = new ImpReportNodeYear(year, m_rootItem);
                    idYear++;
                    idImporter = 0;
                    idReport = 0;
                    idFile = 0;
                } else {
                    nodeYear = m_rootItem->child(year);
                }
                if (!nodeYear->contains(importer)) {
                    nodeImporter = new ImpReportNodeImporter(importer, nodeYear);
                    idImporter++;
                    idReport = 0;
                    idFile = 0;
                } else {
                    nodeImporter = nodeYear->child(importer);
                }
                if (!nodeImporter->contains(report)) {
                    nodeReport = new ImpReportNodeReportType(report, nodeImporter);
                    idReport++;
                    idFile = 0;
                } else {
                    nodeReport = nodeImporter->child(report);
                }
                nodeFile = new ImpReportNodeFile(dateBegin, dateEnd, fileName, nodeReport);
                idFile++;
            }
        }
        beginInsertRows(QModelIndex(), 0, m_rootItem->rowCount()-1);
        endInsertRows();
    }
}
//----------------------------------------------------------
void ImportedFileReportManager::recordFile(
        const QString &fileName,
        const QString &importerName,
        const QString &reportType,
        const QDate &begin,
        const QDate &end)
{
    // TODO check if file exists. If exists, Q_ASSERT false
    Q_ASSERT(begin.isValid() && end.isValid());
    QString year = QString::number(begin.year());
    if (begin.year() != end.year()) {
        year += "-" + QString::number(end.year());
    }
    ImporterYearsManager::instance()->recordYear(begin.year());
    ImporterYearsManager::instance()->recordYear(end.year());
    ImpReportNode *nodeYear = nullptr;
    ImpReportNode *nodeImporter = nullptr;
    ImpReportNode *nodeReport = nullptr;
    bool insertingAsked = false;
    QModelIndex indexYear;
    QModelIndex indexImporter;
    QModelIndex indexReport;
    if (!m_rootItem->contains(year)) {
        beginInsertRows(QModelIndex(), m_rootItem->rowCount(), m_rootItem->rowCount());
        insertingAsked = true;
        nodeYear = new ImpReportNodeYear(
                    year, m_rootItem);
    } else {
        nodeYear = m_rootItem->child(year);
        indexYear = index(nodeYear->row(), 0, QModelIndex());
    }
    if (!nodeYear->contains(importerName)) {
        nodeImporter = new ImpReportNodeImporter(
                    importerName, nodeYear);
        if (!insertingAsked) {
            beginInsertRows(indexYear, nodeYear->rowCount(), nodeYear->rowCount());
            insertingAsked = true;
        }
    } else {
        nodeImporter = nodeYear->child(importerName);
        indexImporter = index(nodeImporter->row(), 0, indexYear);
    }
    if (!nodeImporter->contains(reportType)) {
        nodeReport = new ImpReportNodeReportType(
                    reportType, nodeImporter);
        if (!insertingAsked) {
            beginInsertRows(indexImporter, nodeImporter->rowCount(), nodeImporter->rowCount());
            insertingAsked = true;
        }
    } else {
        nodeReport = nodeImporter->child(reportType);
        indexReport = index(nodeReport->row(), 0, indexImporter);
    }
    if (!insertingAsked) {
        beginInsertRows(indexReport, nodeReport->rowCount(), nodeReport->rowCount());
    }
    QString fixedFileName = QFileInfo(fileName).fileName();
    ImpReportNode *nodeFile = new ImpReportNodeFile(
                begin, end, fixedFileName, nodeReport);
    (void)nodeFile; /// To avoid unused warning while we use it for debeging
    saveInSettings();
    endInsertRows();
}
//----------------------------------------------------------
void ImportedFileReportManager::removeFile(
        const QString &filePath,
        const QString &importerName,
        const QString &reportType)
{
    for (auto itemYear : m_rootItem->children()) {
        removeFile(filePath, itemYear->value(), importerName, reportType);
    }
}
//----------------------------------------------------------
void ImportedFileReportManager::removeFile(
        const QString &filePath,
        const QString &yearDir,
        const QString &importerName,
        const QString &reportType)
{
    QString fixedFileName = QFileInfo(filePath).fileName();
    auto itemYear = m_rootItem->child(yearDir);
    if (itemYear->contains(importerName)) {
        auto itemImporter = itemYear->child(importerName);
        if (itemImporter->contains(reportType)) {
            auto itemReport = itemImporter->child(reportType);
            if (itemReport->contains(fixedFileName)) {
                auto itemFile = itemReport->child(fixedFileName);
                QDir reportDir = SettingManager::instance()->reportDirectory(
                            itemImporter->value(), itemReport->value());
                reportDir.cd(itemYear->value());
                reportDir.remove(fixedFileName);
                if (reportDir.entryList(QDir::Files).size() == 0) {
                    reportDir.removeRecursively();
                }
                QModelIndex indexYear = index(itemYear->row(), 0, QModelIndex());
                QModelIndex indexImporter = index(itemImporter->row(), 0, indexYear);
                QModelIndex indexReport = index(itemReport->row(), 0, indexImporter);
                beginRemoveRows(indexReport, itemFile->row(), itemFile->row());
                itemReport->removeChild(fixedFileName);
                endRemoveRows();
                saveInSettings();
            }
        }
    }
}
//----------------------------------------------------------
QStringList ImportedFileReportManager::filePaths(
        const QString &importerName,
        const QString &reportType,
        int year,
        bool yearInLastOnly) const
{
    QStringList fileNames;
    for (auto childYear : m_rootItem->children()) {
        if ((yearInLastOnly && childYear->value().endsWith(QString::number(year)))
                || (!yearInLastOnly && childYear->value().contains(QString::number(year)))) {
            QString yearFolder = childYear->value();
            if (childYear->contains(importerName)) {
                auto childImporter = childYear->child(importerName);
                if (childImporter->contains(reportType)) {
                    auto childReport = childImporter->child(reportType);
                    QDir reportDir = SettingManager::instance()->reportDirectory(importerName, reportType, yearFolder);
                    for (auto childFile : childReport->children()) {
                        QString fileName = childFile->value();
                        fileNames << reportDir.filePath(fileName);
                    }
                }
            }
        }
    }
    return fileNames;
}
//----------------------------------------------------------
QStringList ImportedFileReportManager::filePaths(
        const QString &importerName,
        const QString &reportType,
        int year,
        int month,
        bool yearInLastOnly) const
{
    QStringList fileNames;
    for (auto childYear : m_rootItem->children()) {
        if ((yearInLastOnly && childYear->value().endsWith(QString::number(year)))
                || (!yearInLastOnly && childYear->value().contains(QString::number(year)))) {
            QString yearFolder = childYear->value();
            if (childYear->contains(importerName)) {
                auto childImporter = childYear->child(importerName);
                if (childImporter->contains(reportType)) {
                    auto childReport = childImporter->child(reportType);
                    QDir reportDir = SettingManager::instance()->reportDirectory(importerName, reportType, yearFolder);
                    for (auto childFile : childReport->children()) {
                        ImpReportNodeFile *nodeFile
                                = static_cast<ImpReportNodeFile *>(childFile);
                        auto begin = nodeFile->dateBegin();
                        auto end = nodeFile->dateEnd();
                        if (begin.month() <= month && end.month() >= month) {
                            QString fileName = childFile->value();
                            fileNames << reportDir.filePath(fileName);
                        }
                    }
                }
            }
        }
    }
    return fileNames;
}
//----------------------------------------------------------
QString ImportedFileReportManager::filePath(
        const QString &importerName,
        const QString &reportType,
        const QString &fileName,
        int year) const
{
    for (auto filePath : filePaths(importerName, reportType, year)) {
        QFileInfo fileInfo(filePath);
        if (fileInfo.fileName() == fileName) {
            return filePath;
        }
    }
    return "";
}
//----------------------------------------------------------
int ImportedFileReportManager::year(
        const QString &importerName,
        const QString &reportType,
        const QString &filePath)
{
    QStringList fileNames;
    for (auto childYear : m_rootItem->children()) {
        QString yearFolder = childYear->value();
        if (childYear->contains(importerName)) {
            auto childImporter = childYear->child(importerName);
            if (childImporter->contains(reportType)) {
                auto childReport = childImporter->child(reportType);
                QDir reportDir = SettingManager::instance()->reportDirectory(importerName, reportType, yearFolder);
                for (auto childFile : childReport->children()) {
                    if (childFile->value() == filePath) {
                        QString year = childYear->value();
                        if (year.contains("-")) {
                            year = year.split("-").last();
                            return year.toInt();
                        }
                    }
                }
            }
        }
    }
    return 0;
}
//----------------------------------------------------------
QPair<QDate, QDate> ImportedFileReportManager::dateMinMax(
        const QString &importerName,
        const QString &reportType,
        const QString &filePath)
{
    QPair<QDate, QDate> dateMinMax;
    for (auto childYear : m_rootItem->children()) {
        QString yearFolder = childYear->value();
        if (childYear->contains(importerName)) {
            auto childImporter = childYear->child(importerName);
            if (childImporter->contains(reportType)) {
                auto childReport = childImporter->child(reportType);
                QDir reportDir = SettingManager::instance()->reportDirectory(
                            importerName, reportType, yearFolder);
                for (auto childFile : childReport->children()) {
                    QString childFilePath = reportDir.filePath(childFile->value());
                    if (childFilePath == filePath) {
                        ImpReportNodeFile *nodeFile
                                = static_cast<ImpReportNodeFile *>(childFile);
                        dateMinMax.first = nodeFile->dateBegin();
                        dateMinMax.second = nodeFile->dateEnd();
                    }
                }
            }
        }
    }
    return dateMinMax;
}
//----------------------------------------------------------
/*
QList<int> ImportedFileReportManager::years() const
{
    QList<int> years;
    for (auto key : m_rootItem->keys()) {
        bool ok = true;
        int year = key.toInt(&ok);
        if (ok) {
            years << year;
        }
    }
    return years;
}
//*/
//----------------------------------------------------------
QVariant ImportedFileReportManager::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    QVariant value;
    static QStringList labels
            = {tr("Fichiers par année et type"),
               tr("Début"),
               tr("Fin")};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        value = labels[section];
    }
    return value;
}
//----------------------------------------------------------
Qt::ItemFlags ImportedFileReportManager::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    QString dirOrFileName = index.data().toString();
    if (dirOrFileName.endsWith(".txt") || dirOrFileName.endsWith(".csv")) {
        flags |= Qt::ItemIsSelectable;
        if (index.column() == 0) {
            flags |= Qt::ItemIsEditable;
        }
    }
    return flags;
}
//----------------------------------------------------------
QModelIndex ImportedFileReportManager::index(
        int row, int column, const QModelIndex &parent) const
{
    QModelIndex index;
    if (hasIndex(row, column, parent)) {
        ImpReportNode *item = nullptr;
        if (parent.isValid()) {
            ImpReportNode *itemParent
                    = static_cast<ImpReportNode *>(
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
QModelIndex ImportedFileReportManager::parent(
        const QModelIndex &index) const
{
    QModelIndex parentIndex;
    if (index.isValid()) {
        ImpReportNode *item
                = static_cast<ImpReportNode *>(
                    index.internalPointer());
        if (item->parent() != nullptr) {
            parentIndex = createIndex(item->parent()->row(), 0, item->parent());
        }
    }
    return parentIndex;
}
//----------------------------------------------------------
int ImportedFileReportManager::rowCount(const QModelIndex &parent) const
{
    ImpReportNode *itemParent = nullptr;
    if (parent.isValid()) {
        itemParent = static_cast<ImpReportNode *>(
                    parent.internalPointer());
    } else {
        itemParent = m_rootItem;
    }
    int count = itemParent->rowCount();
    return count;
}
//----------------------------------------------------------
int ImportedFileReportManager::columnCount(const QModelIndex &) const
{
    return 3;
}
//----------------------------------------------------------
QVariant ImportedFileReportManager::data(
        const QModelIndex &index, int role) const
{
    QVariant value;
    if ((role == Qt::DisplayRole || role == Qt::EditRole) && index.isValid()) {
        ImpReportNode *item
                = static_cast<ImpReportNode *>(
                    index.internalPointer());
        value = item->data(index.column(), role);
    }
    return value;
}
//----------------------------------------------------------
bool ImportedFileReportManager::setData(
        const QModelIndex &index,
        const QVariant &value,
        int role)
{
    bool edited = false;
    if (role == Qt::EditRole && index.column() == 0) {
        QString newFileName = value.toString();
        QString oldFileName = index.data().toString();
        QString currentFileNameExtention;
        if (oldFileName.contains(".")) {
            currentFileNameExtention = "." + oldFileName.split(".").last();
        }
        if (newFileName.endsWith(currentFileNameExtention)) {
            QString report = index.parent().data().toString();
            QString importer = index.parent().parent().data().toString();
            QString dirYear = index.parent().parent().parent().data().toString();
            auto reportDir = SettingManager::instance()->reportDirectory(
                        importer, report, dirYear);
            QString filePathOld = reportDir.filePath(oldFileName);
            QString filePathNew = reportDir.filePath(newFileName);
            QFile fileOld(filePathOld);
            qInfo() << "Creating " << newFileName << "from" << filePathOld << "?";
            qInfo() << QFile::copy(filePathOld, filePathNew);
            reportDir.remove(oldFileName);
            auto node = ImpReportNode::nodeFile(
                        dirYear, importer, report, oldFileName);
            node->setValue(newFileName);
            edited = true;
        }
    }
    return edited;

}
//----------------------------------------------------------
void ImportedFileReportManager::onCustomerSelectedChanged(
        const QString &customerId)
{
    if (customerId.isEmpty()) {
        m_settingKey = "";
        emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
    } else {
        m_settingKey = "ImportedFileReportManager-" + customerId;
        loadFromSettings();
    }
}
//----------------------------------------------------------
void ImportedFileReportManager::reorder()
{
    m_rootItem->sort(false);
    for (auto childYear : m_rootItem->children()) {
        for (auto childImporter : childYear->children()) {
            for (auto childReport : childImporter->children()) {
                childReport->sort();
            }
        }
    }
    saveInSettings();
    emit dataChanged(index(0, 0),
                     index(rowCount()-1, columnCount()-1));
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
ImpReportNode::ImpReportNode(const QString &value, ImpReportNode *parent)
{
    m_row = 0;
    m_value = value;
    m_parent = parent;
    if (parent != nullptr) {
        m_row = parent->rowCount();
        parent->m_children << this;
        parent->m_childrenMapping[m_value] = this;
    }
}
//----------------------------------------------------------
ImpReportNode::~ImpReportNode()
{
    qDeleteAll(m_children); // TODO here it can crash on application closing
}
//----------------------------------------------------------
QVariant ImpReportNode::data(int column, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole && column == 0) {
        value = m_value;
    }
    return value;
}
//----------------------------------------------------------
ImpReportNode *ImpReportNode::parent() const
{
    return m_parent;
}
//----------------------------------------------------------
bool ImpReportNode::contains(const QString &key) const
{
    return m_childrenMapping.contains(key);
}
//----------------------------------------------------------
ImpReportNode *ImpReportNode::child(const QString &key) const
{
    return m_childrenMapping[key];
}
//----------------------------------------------------------
ImpReportNode *ImpReportNode::child(int row) const
{
    return m_children[row];
}
//----------------------------------------------------------
QStringList ImpReportNode::keys() const
{
    QStringList keys;
    for (auto child : m_children) {
        keys << child->m_value;
    }
    return keys;
}
//----------------------------------------------------------
int ImpReportNode::rowCount() const
{
    return m_children.size();
}
//----------------------------------------------------------
int ImpReportNode::row() const
{
    return m_row;
}
//----------------------------------------------------------
void ImpReportNode::setRow(int row)
{
    m_row = row;
}
//----------------------------------------------------------
void ImpReportNode::removeChild(int row)
{
    auto child = m_children.takeAt(row);
    m_childrenMapping.remove(child->value());
    /// Workaround as Qt store it in QModelIndex and then try to get the parent
    /// after it was deleted to compare it to something
    static QList<QSharedPointer<ImpReportNode>> toDeleteLater;
    if (dynamic_cast<ImpReportNodeFile*>(child) != nullptr) {
        delete dynamic_cast<ImpReportNodeFile*>(child);
    } else {
        toDeleteLater << QSharedPointer<ImpReportNode>(child);
    }
    /*
    QTimer::singleShot(500,[child](){
        delete child;
    });
    //*/
    if (m_children.size() == 0) {
        removeRecursively();
    } else {
        for (int i=row; i<m_children.size(); ++i) {
            m_children[i]->setRow(i);
        }
    }
}
//----------------------------------------------------------
void ImpReportNode::removeChild(const QString &key)
{
    auto child = m_childrenMapping.take(key);
    m_children.removeAt(child->row());
    /*
    QTimer::singleShot(500,[child](){
        delete child;
    });
    //*/
    static QList<QSharedPointer<ImpReportNode>> toDeleteLater;
    if (dynamic_cast<ImpReportNodeFile*>(child) != nullptr) {
        delete dynamic_cast<ImpReportNodeFile*>(child);
    } else {
        toDeleteLater << QSharedPointer<ImpReportNode>(child);
    }
    /// Workaround as Qt store it in QModelIndex and then try to get the parent
    /// after it was deleted to compare it to something
    if (m_children.size() == 0) {
        removeRecursively();
    } else {
        for (int i=0; i<m_children.size(); ++i) {
            m_children[i]->setRow(i);
        }
    }
}
//----------------------------------------------------------
QList<ImpReportNode *> ImpReportNode::children() const
{
    return m_children;
}
//----------------------------------------------------------
ImpReportNode *ImpReportNode::nodeFile(
        const QString &dirYear,
        const QString &importerName,
        const QString &reportName,
        const QString &fileName)
{
    auto node = (*dicNodeFile())[dirYear][importerName][reportName][fileName];
    return node;
}
//----------------------------------------------------------
/*
ImpReportNode *ImpReportNode::nodeReport(
        const QString &dirYear, const QString &importerName, const QString &reportName)
{
    auto node = (*dicNodeReport())[dirYear][importerName][reportName];
    return node;
}
//*/
//----------------------------------------------------------
QHash<QString, QHash<QString, QHash<QString, QHash<QString, ImpReportNode *>>>> *
ImpReportNode::dicNodeFile()
{
    static QHash<QString, QHash<QString, QHash<QString, QHash<QString, ImpReportNode *>>>> instance;
    return &instance;
}
//----------------------------------------------------------
/*
QHash<QString, QHash<QString, QHash<QString, ImpReportNode *>>> *
ImpReportNode::dicNodeReport()
{
    static QHash<QString, QHash<QString, QHash<QString, ImpReportNode *>>> instance;
    return &instance;
}
//*/
//----------------------------------------------------------
void ImpReportNode::setValue(const QString &value)
{
    m_value = value;
}
//----------------------------------------------------------
QString ImpReportNode::value() const
{
    return m_value;
}
//----------------------------------------------------------
void ImpReportNode::removeRecursively()
{
    if (parent() != nullptr) {
        parent()->removeChild(row());
    }
}
//----------------------------------------------------------
void ImpReportNode::sort(bool alphaOrder)
{
    if (alphaOrder) {
        std::sort(m_children.begin(),
                  m_children.end(),
                  [](const ImpReportNode *node1, const ImpReportNode *node2) -> bool{
            return node1->value() < node2->value();
        });
    } else {
        std::sort(m_children.begin(),
                  m_children.end(),
                  [](const ImpReportNode *node1, const ImpReportNode *node2) -> bool{
            return node1->value() > node2->value();
        });
    }
    for (int i=0; i<m_children.size(); ++i) {
        m_children[i]->setRow(i);
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
ImpReportNodeYear::ImpReportNodeYear(const QString &value, ImpReportNode *parent)
    : ImpReportNode(value, parent)
{
}
//----------------------------------------------------------
ImpReportNodeImporter::ImpReportNodeImporter(const QString &value, ImpReportNode *parent)
    : ImpReportNode(value, parent)
{

}
//----------------------------------------------------------
ImpReportNodeReportType::ImpReportNodeReportType(const QString &value, ImpReportNode *parentNode)
    : ImpReportNode(value, parentNode)
{
    QString dirYear = parent()->parent()->value();
    QString dirImporter = parent()->value();
    /*
    if (!dicNodeReport()->contains(dirYear)) {
        dicNodeReport()->insert(
                    dirYear,
                    QHash<QString, QHash<QString, ImpReportNode *>>());
    }
    if (!(*dicNodeReport())[dirYear].contains(dirImporter)) {
        (*dicNodeReport())[dirYear][dirImporter]
                = QHash<QString, ImpReportNode *>();

    }
    (*dicNodeReport())[dirYear][dirImporter][value] = this;
    //*/
}
//----------------------------------------------------------
ImpReportNodeReportType::~ImpReportNodeReportType()
{
    /*
    QString dirYear = parent()->parent()->parent()->value();
    QString dirImporter = parent()->parent()->value();
    (*dicNodeReport())[dirYear][dirImporter].remove(value());
    if ((*dicNodeReport())[dirYear][dirImporter].isEmpty()) {
        (*dicNodeReport())[dirYear].remove(dirImporter);
        if ((*dicNodeReport())[dirYear].isEmpty()) {
            (*dicNodeReport()).remove(dirYear);
        }
    }
    //*/
}
//----------------------------------------------------------
ImpReportNodeFile::ImpReportNodeFile(const QDate &begin, const QDate &end, const QString &value, ImpReportNode *parentNode)
    : ImpReportNode(value, parentNode)
{
    m_dateBegin = begin;
    m_dateEnd = end;
    QString dirYear = parent()->parent()->parent()->value();
    QString dirImporter = parent()->parent()->value();
    QString dirReport = parent()->value();
    if (!dicNodeFile()->contains(dirYear)) {
        dicNodeFile()->insert(
                    dirYear,
                    QHash<QString, QHash<QString, QHash<QString, ImpReportNode *>>>());
    }
    if (!(*dicNodeFile())[dirYear].contains(dirImporter)) {
        (*dicNodeFile())[dirYear][dirImporter]
                = QHash<QString, QHash<QString, ImpReportNode *>>();

    }
    if (!(*dicNodeFile())[dirYear][dirImporter].contains(dirReport)) {
        (*dicNodeFile())[dirYear][dirImporter][dirReport] = QHash<QString, ImpReportNode *>();
    }
    (*dicNodeFile())[dirYear][dirImporter][dirReport][value] = this;
}
//----------------------------------------------------------
ImpReportNodeFile::~ImpReportNodeFile()
{
    QString dirYear = parent()->parent()->parent()->value();
    QString dirImporter = parent()->parent()->value();
    QString dirReport = parent()->value();
    (*dicNodeFile())[dirYear][dirImporter][dirReport].remove(value());
    if ((*dicNodeFile())[dirYear][dirImporter][dirReport].isEmpty()) {
        (*dicNodeFile())[dirYear][dirImporter].remove(dirReport);
        if ((*dicNodeFile())[dirYear][dirImporter].isEmpty()) {
            (*dicNodeFile())[dirYear].remove(dirImporter);
            if ((*dicNodeFile())[dirYear].isEmpty()) {
                (*dicNodeFile()).remove(dirYear);
            }
        }
    }
}
//----------------------------------------------------------
QDate ImpReportNodeFile::dateBegin() const
{
    return m_dateBegin;
}
//----------------------------------------------------------
QDate ImpReportNodeFile::dateEnd() const
{
    return m_dateEnd;
}
//----------------------------------------------------------
QStringList ImpReportNodeFile::years() const
{
    QSet<QString> years;
    years << QString::number(m_dateBegin.year());
    years << QString::number(m_dateEnd.year());
    return years.toList();
}
//----------------------------------------------------------
QVariant ImpReportNodeFile::data(int column, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole || role == Qt::EditRole){
        if (column == 0) {
            value = this->value();
        } else if (column == 1) {
            value = m_dateBegin.toString("yyyy-MM-dd");
        } else if (column == 2) {
            value = m_dateEnd.toString("yyyy-MM-dd");
        }
    }
    return value;
}
//----------------------------------------------------------
void ImpReportNodeFile::setValue(const QString &newValue)
{
    QString oldValue = value();
    QString dirYear = parent()->parent()->parent()->value();
    QString dirImporter = parent()->parent()->value();
    QString dirReport = parent()->value();
    (*dicNodeFile())[dirYear][dirImporter][dirReport].remove(oldValue);
    (*dicNodeFile())[dirYear][dirImporter][dirReport][newValue] = this;
    ImpReportNode::setValue(newValue);
}
//----------------------------------------------------------
