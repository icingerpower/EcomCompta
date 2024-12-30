#ifndef ORDERREPORTSWIDGET_H
#define ORDERREPORTSWIDGET_H

#include <QtWidgets/qwidget.h>
#include <QtCore/qdir.h>

#include "model/orderimporters/AbstractOrderImporter.h"
#include "model/UpdateToCustomer.h"

class AbstractOrderImporter;
class FileSystemModelNoDir;
class OrderManager;
class RefundManager;

namespace Ui {
class OrderReportsWidget;
}

class OrderReportsWidget : public QWidget, public UpdateToCustomer
{
    Q_OBJECT

public:
    explicit OrderReportsWidget(QWidget *parent = nullptr);
    ~OrderReportsWidget() override;
    void init(const ReportType &reportType,
              AbstractOrderImporter *importer);
    //QString reportDirPathRel() const;
    //QDir reportDir() const;
    void onCustomerSelectedChanged(const QString &customerId) override;
    QString uniqueId() const override;

public slots:
    void importReports();
    void loadSelectedReports();
    void loadAllReports();
    void removeSelectedReports();

    //void removeSelectedRefunds();

private:
    Ui::OrderReportsWidget *ui;
    AbstractOrderImporter *m_importer;
    OrderManager *m_orderManager;
    RefundManager *m_refundManager;
    ReportType m_reportType;
    void _connectSlots();
    FileSystemModelNoDir *m_fileModel;
    void _showOrHideOrderRefundTabs();
    void displayFileNames(const QStringList &fileNames, bool copyFilesInDir);
};

#endif // ORDERREPORTSWIDGET_H
