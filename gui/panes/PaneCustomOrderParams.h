#ifndef PANECUSTOMORDERPARAMS_H
#define PANECUSTOMORDERPARAMS_H

#include <QWidget>

namespace Ui {
class PaneCustomOrderParams;
}

class PaneCustomOrderParams : public QWidget
{
    Q_OBJECT

public:
    explicit PaneCustomOrderParams(QWidget *parent = nullptr);
    ~PaneCustomOrderParams();


public slots:
    void add();
    void remove();
    void test();


private:
    Ui::PaneCustomOrderParams *ui;
};

#endif // PANECUSTOMORDERPARAMS_H
