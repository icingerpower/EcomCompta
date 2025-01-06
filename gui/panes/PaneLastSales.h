#ifndef PANELASTSALES_H
#define PANELASTSALES_H

#include <QWidget>
#include <QtCore/qitemselectionmodel.h>

namespace Ui {
class PaneLastSales;
}

class PaneLastSales : public QWidget
{
    Q_OBJECT

public:
    explicit PaneLastSales(QWidget *parent = nullptr);
    ~PaneLastSales();

public slots:
    void addGroup();
    void removeGroupSelected();
    void compute();
    void exportCsv();
    void browseGsprFolder();

private slots:
    void onGroupKeywordsEdited();
    void onGroupSelected(const QItemSelection &newSelection,
                       const QItemSelection &previousSelection);

private:
    Ui::PaneLastSales *ui;
    void _connectSlots();
    bool m_updatingTextEditGroup;
    static const QString SETTING_DIR_GSPR_FILES;
};

#endif // PANELASTSALES_H
