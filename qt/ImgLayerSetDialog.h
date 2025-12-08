#ifndef IMGLAYERSETDIALOG_H
#define IMGLAYERSETDIALOG_H

#include <QDialog>
#include "../Inc/ryprtapi.h"

namespace Ui {
class ImgLayerSetDialog;
}

class ImgLayerSetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImgLayerSetDialog(QWidget *parent = nullptr);
    ~ImgLayerSetDialog();

private slots:
    void accept() override;

private:
    void loadParams();
    void saveParams();

    Ui::ImgLayerSetDialog *ui;
};

#endif // IMGLAYERSETDIALOG_H
