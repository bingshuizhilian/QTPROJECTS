#ifndef BITMAPPROCESS_H
#define BITMAPPROCESS_H

#include <QMainWindow>
#include <QImage>

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
