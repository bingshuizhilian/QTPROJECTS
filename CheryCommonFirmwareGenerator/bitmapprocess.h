#ifndef BITMAPPROCESS_H
#define BITMAPPROCESS_H

#include <QMainWindow>
#include <QImage>
#include "bmp.h"

namespace Ui {
class BitmapProcess;
}

class BitmapProcess : public QMainWindow
{
    Q_OBJECT

public:
    explicit BitmapProcess(QWidget *parent = 0);
    ~BitmapProcess();

private:
    Ui::BitmapProcess *ui;

private:
    QImage* image;
    QString selectedFilePathName;
    QString selectedFilePath;
    QStringList allImageNamesList;
    BitmapHandler bmp;

private:
    QString toCTypeArray(BitmapHandler& bmp, BMPBITPERPIXEL destbpp); //生成C语言形式数组
    void compressCArrayOfBitmap(QString filepathname);
    QStringList getDirFilesName(QString pathsDir);

private slots:
    void on_btn_generateCArray_clicked();
    void on_btn_openBmp_clicked();
    void on_btn_prevPic_clicked();
    void on_btn_nextPic_clicked();
};

#endif // BITMAPPROCESS_H
