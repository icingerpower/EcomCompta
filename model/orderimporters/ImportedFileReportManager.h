#ifndef IMPORTEDFILEREPORTMANAGER_H
#define IMPORTEDFILEREPORTMANAGER_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qsharedpointer.h>
//#include <QtCore/qfilesystemwatcher.h>
#include <QtCore/qdatetime.h>
#include "OrderMapping.h"

class ImpReportNode;
class ImportedFileReportManager : public QAbstractItemModel
{
    Q_OBJECT

public:
    static ImportedFileReportManager *instance();
    ~ImportedFileReportManager() override;

    QSharedPointer<OrdersMapping> loadOrders(
            const QString &importerName,
            const QDate &dateMin,
            const QDate &dateMax) const;
    void saveInSettings() const;
    void loadFromSettings();
    void recordFile(const QString &fileName,
                    const QString &importerName,
                    const QString &reportType,
                    const QDate &begin,
                    const QDate &end);
    void removeFile(const QString &filePath,
                    const QString &importerName,
                    const QString &reportType);
    void removeFile(const QString &filePath,
                    const QString &yearDir,
                    const QString &importerName,
                    const QString &reportType);
    QStringList filePaths(const QString &importerName,
                          const QString &reportType,
                          int year,
                          bool yearInLastOnly = false) const;
    QStringList filePaths(const QString &importerName,
                          const QString &reportType,
                          int year,
                          int month,
                          bool yearInLastOnly = false) const;
    QString filePath(const QString &importerName,
                     const QString &reportType,
                     const QString &fileName,
                     int year) const;
    int year(const QString &importerName,
             const QString &reportType,
             const QString &filePath);
    QPair<QDate, QDate> dateMinMax(
            const QString &importerName,
             const QString &reportType,
             const QString &filePath);

    //QList<int> years() const;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;

signals:
    void fileRenamed(const QString &yearDir, const QString &importer, const QString &report, const QString &oldFileName, const QString &newFileName);
    void fileRemoved(const QString &yearDir, const QString &importer, const QString &report, const QString &fileName);
    void fileAdded(const QString &yearDir, const QString &importer, const QString &report, const QString &fileName);

public slots:
    void onCustomerSelectedChanged(const QString &customerId);
    void reorder();
    //void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void onFileRenamed(const QString &path, const QString &oldName, const QString &newName);
    //void onRowsRemoved(const QModelIndex &parent, int first, int last);
    //void onFileChanged(const QString &filePath);

private:
    explicit ImportedFileReportManager(QObject *parent = nullptr);
    QString m_settingKey;
    ImpReportNode *m_rootItem;
    //QFileSystemWatcher *m_fileSystemWatcher;
};

class ImpReportNode {
public:
    ImpReportNode(const QString &value, ImpReportNode *parent = nullptr);
    virtual ~ImpReportNode();
    virtual QVariant data(int column, int role) const;
    ImpReportNode *parent() const;
    bool contains(const QString &key) const;
    ImpReportNode *child(const QString &key) const;
    ImpReportNode *child(int row) const;
    QStringList keys() const;
    int rowCount() const;
    int row() const;
    void setRow(int row);
    void removeChild(int row);
    void removeChild(const QString &key);
    QList<ImpReportNode *> children() const;
    QString value() const;
    void removeRecursively();
    void sort(bool alphaOrder = true);

    static ImpReportNode *nodeFile(
            const QString &dirYear,
            const QString &importerName,
            const QString &reportName,
            const QString &fileName);
    /*
    static ImpReportNode *nodeReport(
            const QString &dirYear,
            const QString &importerName,
            const QString &reportName);
            //*/

    virtual void setValue(const QString &value);

protected:
    static QHash<QString, QHash<QString, QHash<QString, QHash<QString, ImpReportNode *>>>> *dicNodeFile();
    //static QHash<QString, QHash<QString, QHash<QString, ImpReportNode *>>> *dicNodeReport();


private:
    QString m_value;
    int m_row;
    ImpReportNode *m_parent;
    QList<ImpReportNode *> m_children;
    QHash<QString, ImpReportNode *> m_childrenMapping;
};

class ImpReportNodeYear : public ImpReportNode{
public:
    ImpReportNodeYear(const QString &value, ImpReportNode *parent = nullptr);
};

class ImpReportNodeImporter : public ImpReportNode{
public:
    ImpReportNodeImporter(const QString &value, ImpReportNode *parent = nullptr);
};

class ImpReportNodeReportType : public ImpReportNode{
public:
    ImpReportNodeReportType(const QString &value, ImpReportNode *parent = nullptr);
    ~ImpReportNodeReportType() override;
};

class ImpReportNodeFile : public ImpReportNode{
public:
    ImpReportNodeFile(
            const QDate &begin,
            const QDate &end,
            const QString &value,
            ImpReportNode *parent = nullptr);
    ~ImpReportNodeFile() override;
    QDate dateBegin() const;
    QDate dateEnd() const;
    QStringList years() const;
    QVariant data(int column, int role) const override;
    void setValue(const QString &value) override;

private:
    QDate m_dateBegin;
    QDate m_dateEnd;
};

#endif // IMPORTEDFILEREPORTMANAGER_H
