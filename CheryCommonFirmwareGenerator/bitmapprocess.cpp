#include "bitmapprocess.h"
#include "ui_bitmapprocess.h"
#include "bmp.h"
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QClipboard>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

BitmapProcess::BitmapProcess(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BitmapProcess),
    image(new QImage)
{
    ui->setupUi(this);

    this->setFixedSize(this->size());
    ui->btn_generateCArray->setEnabled(false);
    ui->btn_prevPic->setEnabled(false);
    ui->btn_nextPic->setEnabled(false);
}

BitmapProcess::~BitmapProcess()
{
    delete ui;
}

QStringList BitmapProcess::getDirFilesName(QString pathsDir)
{
    /*获取文件夹下的文件名称*/
    QDir dir(pathsDir);
    if (!dir.exists())
    {
        return QStringList("");
    }

    dir.setFilter(QDir::Files | QDir::NoSymLinks);

    QStringList filters;
    filters << "*.bmp" << "*.png" << "*.jpg" << "*.jpeg";
    dir.setNameFilters(filters);
    dir.setSorting(QDir::Name);

    QStringList imageNamesList = dir.entryList();
    if (imageNamesList.size() <= 0)
    {
        return QStringList("");
    }

    return imageNamesList;
}
BitmapHandler bmp;
void BitmapProcess::on_btn_openBmp_clicked()
{
    selectedFilePathName = QFileDialog::getOpenFileName(this, tr("Open Picture"),
                                                "",
                                                tr("Images (*.bmp *.png *.jpg *.jpeg)"));

    selectedFilePath = QFileInfo(selectedFilePathName).absolutePath();

    allImageNamesList = getDirFilesName(selectedFilePath);

    qDebug() << selectedFilePathName << endl << selectedFilePath << endl << allImageNamesList;

    if(selectedFilePathName.isEmpty())
    {
        ui->btn_generateCArray->setEnabled(false);
        ui->lable_bmpView->clear();
        ui->btn_prevPic->setEnabled(false);
        ui->btn_nextPic->setEnabled(false);
        ui->label_showPicSize->clear();
        QMessageBox::warning(this, "Warnning", "load failed, please select a picture", QMessageBox::Yes);
        return;
    }

    image->load(selectedFilePathName);
    ui->lable_bmpView->setPixmap(QPixmap::fromImage(*image));
    ui->label_showPicSize->setText(QString("%1x%2").arg(image->width()).arg(image->height()));
    ui->btn_generateCArray->setEnabled(true);

    qDebug() << image->byteCount() << image->bits();

    bmp.load(selectedFilePathName);
    qDebug() << QString("filesize:%1, bpp:%2, datasize:%3, width:%4, height:%5, hasColorTable:%6")
                .arg(bmp.filesize())
                .arg(bmp.bitperpixel())
                .arg(bmp.datasize())
                .arg(bmp.width())
                .arg(bmp.height())
                .arg(bmp.hasColorTable());

    int index = allImageNamesList.indexOf(QFileInfo(selectedFilePathName).fileName());
    ui->btn_prevPic->setEnabled(0 != index ? true : false);
    ui->btn_nextPic->setEnabled(allImageNamesList.size() - 1 != index ? true : false);

    qDebug() << QString("[0 - %2]:%1 -> ").arg(index).arg(allImageNamesList.size() - 1) + QFileInfo(selectedFilePathName).fileName();
}

void BitmapProcess::on_btn_generateCArray_clicked()
{
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    if(ui->cb_isCompress)
        compressCArrayOfBitmap();
}

void BitmapProcess::on_btn_prevPic_clicked()
{
    int index = allImageNamesList.indexOf(QFileInfo(selectedFilePathName).fileName());
    if(0 == index)
        return;

    int newIndex = index - 1;

    ui->btn_nextPic->setEnabled(true);
    if(0 == newIndex)
        ui->btn_prevPic->setEnabled(false);

    selectedFilePathName = selectedFilePath + '/' + allImageNamesList.at(newIndex);

    image->load(selectedFilePathName);
    ui->lable_bmpView->setPixmap(QPixmap::fromImage(*image));
    ui->label_showPicSize->setText(QString("%1x%2").arg(image->width()).arg(image->height()));

    qDebug() << QString("[0 - %2]:%1 -> ").arg(newIndex).arg(allImageNamesList.size() - 1) + QFileInfo(selectedFilePathName).fileName();
}

void BitmapProcess::on_btn_nextPic_clicked()
{
    int index = allImageNamesList.indexOf(QFileInfo(selectedFilePathName).fileName());
    if(allImageNamesList.size() - 1 == index)
        return;

    int newIndex = index + 1;

    ui->btn_prevPic->setEnabled(true);
    if(allImageNamesList.size() - 1 == newIndex)
        ui->btn_nextPic->setEnabled(false);

    selectedFilePathName = selectedFilePath + '/' + allImageNamesList.at(newIndex);

    image->load(selectedFilePathName);
    ui->lable_bmpView->setPixmap(QPixmap::fromImage(*image));
    ui->label_showPicSize->setText(QString("%1x%2").arg(image->width()).arg(image->height()));

    qDebug() << QString("[0 - %2]:%1 -> ").arg(newIndex).arg(allImageNamesList.size() - 1) + QFileInfo(selectedFilePathName).fileName();
}

void BitmapProcess::compressCArrayOfBitmap()
{
    const int DEVIDE_AMOUNT = 255; //下位机存储宽和高各用一个字节，此处应确保此值不大于255（即0xff）
    QString filePathName = QFileDialog::getOpenFileName();
    QString filePath = QFileInfo(filePathName).absolutePath();

    qDebug()<<filePathName<<endl<<filePath<<endl;

    if(filePathName.isEmpty())
    {
        QMessageBox::warning(this, "Warnning", "compress failed, please select a file", QMessageBox::Yes);
        return;
    }

    QFile targetFile(filePathName);
    if(!targetFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Warnning", "Cannot open " + filePathName, QMessageBox::Yes);
        return;
    }

//    this->resize(640, 460);

    QTextStream targetFileIn(&targetFile);
    QStringList originalCodeStringList;
    while(!targetFileIn.atEnd())
    {
        QString readStr = targetFileIn.readLine();

        if(!readStr.isEmpty())
            originalCodeStringList.push_back(readStr);

//        ptOutputWnd->appendPlainText(readStr); //debug
    }

    targetFile.close();

    QString tmpStr = originalCodeStringList.first();

    if(!tmpStr.startsWith("const unsigned char gImage_", Qt::CaseInsensitive) || !tmpStr.endsWith("*/", Qt::CaseInsensitive))
    {
        QMessageBox::warning(this, "Warnning", "make sure the C source file was generated by Image2Lcd", QMessageBox::Yes);
        return;
    }

    //暂未考虑bmp宽或高大于256的情况，因为3.5" TFT实际使用到的图片尺寸在任意方向上不会且不能超过248
    QString widthAndHeight = tmpStr.mid(tmpStr.lastIndexOf(',') - 14, 4);
    widthAndHeight += ", ";
    widthAndHeight += tmpStr.mid(tmpStr.lastIndexOf(',') - 4, 4);
    widthAndHeight += ", //width: " + QString::number(tmpStr.mid(tmpStr.lastIndexOf(',') - 12, 2).toInt(nullptr, 16));
    widthAndHeight += ", height: " + QString::number(tmpStr.mid(tmpStr.lastIndexOf(',') - 2, 2).toInt(nullptr, 16));

    if(tmpStr.mid(tmpStr.lastIndexOf(',') - 17, 2).toInt(nullptr, 16) != 0 || tmpStr.mid(tmpStr.lastIndexOf(',') - 7, 2).toInt(nullptr, 16) != 0)
    {
        int realWidth = 0, realHeight = 0;
        realWidth = tmpStr.mid(tmpStr.lastIndexOf(',') - 17, 2).toInt(nullptr, 16) << 8 | tmpStr.mid(tmpStr.lastIndexOf(',') - 12, 2).toInt(nullptr, 16);
        realHeight = tmpStr.mid(tmpStr.lastIndexOf(',') - 7, 2).toInt(nullptr, 16) << 8 | tmpStr.mid(tmpStr.lastIndexOf(',') - 2, 2).toInt(nullptr, 16);
        widthAndHeight += " [!!! Be careful, realWidth: " + QString::number(realWidth) + ", realHeight: " + QString::number(realHeight) + " !!!]";
    }

    QStringList targetFormattedStringList;
    targetFormattedStringList << "static const unsigned char bmp[] =\n{" << widthAndHeight;
    originalCodeStringList.removeFirst();
    originalCodeStringList.last().remove("};");

    QStringList tmpStringList;
    for(auto elem: originalCodeStringList)
        tmpStringList += elem.split(',');

    tmpStringList.removeAll("");
    //在结尾添加一个结束标记，否则当数据全为0x00或0xff且数据量较大时，下面的正则表达式查找第一个不为0x00或0xff的数据非常耗时
    tmpStringList.append("END_FLAG");

    //debug
//    for(auto elem: tmpStringList)
//        ptOutputWnd->appendPlainText(elem);

    QStringList targetStringList;
    for(int index = 0; index < tmpStringList.size(); ++index)
    {
        if(0 == tmpStringList.at(index).compare("0X00", Qt::CaseInsensitive) || 0 == tmpStringList.at(index).compare("0XFF", Qt::CaseInsensitive))
        {
            targetStringList << tmpStringList.at(index);

            if(tmpStringList.size() - 1 == index)
            {
                targetStringList << "1";
            }
            else
            {
                QRegExp regexp;
                if(0 == tmpStringList.at(index).compare("0X00", Qt::CaseInsensitive))
                    regexp.setPattern("^((?!0[xX]00).)*$");
                else
                    regexp.setPattern("^((?!0[xX][fF][fF]).)*$");

                int matchIndex = tmpStringList.indexOf(regexp, index);
                if(matchIndex - index <= DEVIDE_AMOUNT)
                {
                    targetStringList << QString::number(matchIndex - index);
                }
                else
                {
                    for(int cnt = 0; cnt < (matchIndex - index) / DEVIDE_AMOUNT; ++cnt)
                    {
                        targetStringList << QString::number(DEVIDE_AMOUNT);
                        targetStringList << tmpStringList.at(index);
                    }

                    if((matchIndex - index) % DEVIDE_AMOUNT != 0)
                    {
                        targetStringList << QString::number((matchIndex - index) % DEVIDE_AMOUNT);
                    }
                    else
                    {
                        //移除多添加的0x00或0xff
                        targetStringList.removeLast();
                    }
                }

                //for循环马上会另index自增1，提前减去1后接下来才能定位到下一个非0x00或0xff的元素，即matchIndex所指向的元素
                index = matchIndex - 1;
            }
        }
        else
        {
            targetStringList << tmpStringList.at(index);
        }
    }

    //移除设置的"END_FLAG"
    targetStringList.removeLast();

    for(int index = 0; index < targetStringList.size(); ++index)
    {
        int numOfCharInElem = targetStringList.at(index).size();
        for(int cnt = 0; numOfCharInElem < 4 && cnt < 4 - numOfCharInElem; ++cnt)
            targetStringList[index].prepend(' ');

        if(0 == (index + 1) % 16)
            targetStringList[index] += ",";
        else
            targetStringList[index] += ", ";
    }

    //移除最后一个", "或","，(肯定为二者之一，两个都remove即可保证)
    targetStringList.last().remove(", ");
    targetStringList.last().remove(",");

    QString tmpStr2;
    for(int index = 0; index < targetStringList.size(); ++index)
    {
        if(index % 16 == 0)
        {
            if(!tmpStr2.isEmpty())
                targetFormattedStringList << tmpStr2;

            tmpStr2.clear();
        }

        tmpStr2 += targetStringList.at(index);

        if(index == targetStringList.size() - 1)
            targetFormattedStringList << tmpStr2;
    }

    targetFormattedStringList << "}; //total bytes: " + QString::number(targetStringList.size() + 2);

    for(auto iter = targetFormattedStringList.begin() + 1; iter != targetFormattedStringList.end() - 1; ++iter)
    {
        *iter = (*iter).prepend("    ");
        *iter = (*iter).toUpper();
    }

    //宽度和高度用小写字母
    if(!targetFormattedStringList.isEmpty())
        targetFormattedStringList[1] = targetFormattedStringList[1].left(14) + targetFormattedStringList[1].right(targetFormattedStringList[1].size() - 14).toLower();

//    for(auto& elem: targetFormattedStringList)
//        ptOutputWnd->appendPlainText(elem);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->clear();
//    clipboard->setText(ptOutputWnd->document()->toPlainText());

//    ptOutputWnd->appendPlainText("\n\n***上述结果已经拷贝至系统剪贴板中***");
}

