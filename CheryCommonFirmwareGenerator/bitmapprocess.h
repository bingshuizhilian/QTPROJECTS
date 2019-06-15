#ifndef BITMAPPROCESS_H
#define BITMAPPROCESS_H

#include <QMainWindow>
#include <QImage>

namespace Ui {
class BitmapProcess;
}

enum BMPGRAYCOLOR
{
    //1bit->2 level gray
    BMP1BITCOLOR_WHITE = 0x00,
    BMP1BITCOLOR_BLACK = 0x01,
    //2bits->4 level gray
    BMP2BITSCOLOR_WHITE = 0x00,
    BMP2BITSCOLOR_LIGHTGRAY = 0x01,
    BMP2BITSCOLOR_DARKGRAY = 0x10,
    BMP2BITSCOLOR_BLACK = 0x11
};

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

private:
    void compressCArrayOfBitmap();
    QStringList getDirFilesName(QString pathsDir);

private slots:
    void on_btn_generateCArray_clicked();
    void on_btn_openBmp_clicked();
    void on_btn_prevPic_clicked();
    void on_btn_nextPic_clicked();
};

#endif // BITMAPPROCESS_H
