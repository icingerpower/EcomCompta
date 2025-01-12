#ifndef DIALOGEDITSALESTEMPLATE_H
#define DIALOGEDITSALESTEMPLATE_H

#include <QDialog>
#include <QItemSelection>

namespace Ui {
class DialogEditSalesTemplate;
}

class SaleColumnTree;

class DialogEditSalesTemplate : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditSalesTemplate(QWidget *parent = nullptr);
    ~DialogEditSalesTemplate();
    SaleColumnTree *saleColmunTreeModel() const;

public slots:
    void addTemplate();
    void removeTemplateSelected();
    void addColumnTop();
    void addColumn();
    void removeColumnSelected();
    void upColumn();
    void downColumn();
    void unselect();
    void onTemplateSelected(
            const QItemSelection &newSelection,
            const QItemSelection &oldSelection);

private:
    Ui::DialogEditSalesTemplate *ui;
    void _connectSlots();
};

#endif // DIALOGEDITSALESTEMPLATE_H
