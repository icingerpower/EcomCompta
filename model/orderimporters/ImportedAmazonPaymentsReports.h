#ifndef IMPORTEDAMAZONPAYMENTSREPORTS_H
#define IMPORTEDAMAZONPAYMENTSREPORTS_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qfilesystemwatcher.h>

class ImpAmPaymentNode;

class ImportedAmazonPaymentsReports : public QAbstractItemModel
{
    Q_OBJECT

public:
    static ImportedAmazonPaymentsReports *instance();
    ~ImportedAmazonPaymentsReports() override;

    void saveInSettings() const;
    void loadFromSettings();
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

public slots:
    void onCustomerSelectedChanged(const QString &customerId);
    void onFileChanged(const QString &filePath);

private:
    explicit ImportedAmazonPaymentsReports(QObject *parent = nullptr);
    QString m_settingKey;
    ImpAmPaymentNode *m_rootItem;
    QHash<QString, QString> m_filePathToAmazon;
    void _findSubChannel(const QString &filePath);
    void _generateTree();
    QFileSystemWatcher *m_fileSystemWatcher;
};


class ImpAmPaymentNode {
public:
    ImpAmPaymentNode(const QString &title,
                     ImpAmPaymentNode *parent = nullptr);
    virtual ~ImpAmPaymentNode();
    virtual QVariant data(int column, int role) const;
    //bool contains(const QString &key) const;
    //ImpAmPaymentNode *child(const QString &key) const;
    ImpAmPaymentNode *child(int row) const;
    //QStringList keys() const;
    int rowCount() const;
    void removeChild(int row);
    //void removeChild(const QString &key);
    //void removeRecursively();
    //void sort(bool alphaOrder = true);


    int row() const;

    ImpAmPaymentNode *parent() const;

    QList<ImpAmPaymentNode *> children() const;

    QString title() const;
    void setTitle(const QString &title);

private:
    QString m_title;
    int m_row;
    void _setRow(int row);
    ImpAmPaymentNode *m_parent;
    QList<ImpAmPaymentNode *> m_children;
    //QHash<QString, ImpAmPaymentNode *> m_childrenMapping;
};

class ImpAmPaymentNodeYear : public ImpAmPaymentNode{
public:
    ImpAmPaymentNodeYear(const QString &title,
                     ImpAmPaymentNode *parent = nullptr);
};

class ImpAmPaymentNodeAmazon : public ImpAmPaymentNode{
public:
    ImpAmPaymentNodeAmazon(const QString &title,
                     ImpAmPaymentNode *parent = nullptr);
};

class ImpAmPaymentNodeFile : public ImpAmPaymentNode{
public:
    ImpAmPaymentNodeFile(const QString &title,
                     ImpAmPaymentNode *parent = nullptr);
};

/*
class ImpAmPaymentNodeLine : public ImpAmPaymentNode{
public:
    ImpAmPaymentNodeLine(const QString &title,
                      const QString &account,
                      double value,
                     ImpAmPaymentNode *parent = nullptr);
    QVariant data(int column, int role) const override;
    QString account() const;
    void setAccount(const QString &account);

    double value() const;
    QString valueString() const;
    void setValue(double value);

private:
    QString m_account;
    double m_value;
};

class ImpAmPaymentNodeDetails : public ImpAmPaymentNodeLine{
public:
    ImpAmPaymentNodeDetails(const QString &title,
                      const QString &account,
                      double value,
                     ImpAmPaymentNode *parent = nullptr);
};
//*/

#endif // IMPORTEDAMAZONPAYMENTSREPORTS_H
