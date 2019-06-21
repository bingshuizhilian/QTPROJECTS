#include "bitmapprocess.h"
#include "ui_bitmapprocess.h"
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QClipboard>
#include <QPixmap>
#include <QProcess>
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

void BitmapProcess::on_btn_openBmp_clicked()
{
    selectedFilePathName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开图片"),
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
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("加载失败，请选择一张图片"), QMessageBox::Yes);
        return;
    }

    image->load(selectedFilePathName);
    if(image->width() > 240 || image->height() > 320)
        ui->lable_bmpView->setPixmap(QPixmap::fromImage(image->scaled(240, 320, Qt::KeepAspectRatio)));
    else
        ui->lable_bmpView->setPixmap(QPixmap::fromImage(*image));
    ui->label_showPicSize->setText(QString("%1x%2").arg(image->width()).arg(image->height()));
    ui->btn_generateCArray->setEnabled(true);

    qDebug() << image->bytesPerLine();

    qDebug() << image->byteCount() << image->bits();

    int index = allImageNamesList.indexOf(QFileInfo(selectedFilePathName).fileName());
    ui->btn_prevPic->setEnabled(0 != index ? true : false);
    ui->btn_nextPic->setEnabled(allImageNamesList.size() - 1 != index ? true : false);

    qDebug() << QString("[0 - %2]:%1 -> ").arg(index).arg(allImageNamesList.size() - 1) + QFileInfo(selectedFilePathName).fileName();
}

void BitmapProcess::on_btn_generateCArray_clicked()
{
    bmp.load(selectedFilePathName);

    if(ui->cb_flipColor->isChecked())
        bmp.flipcolor();

//    bmp.save();

    QString saveFilePathName = toCTypeArray(bmp, ui->rbtn_grayLv1bit->isChecked() ? BMP_1BITPERPIXEL : BMP_2BITSPERPIXEL);

    if(ui->cb_isCompress->isChecked())
        compressCArrayOfBitmap(saveFilePathName);
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
    if(image->width() > 240 || image->height() > 320)
        ui->lable_bmpView->setPixmap(QPixmap::fromImage(image->scaled(240, 320, Qt::KeepAspectRatio)));
    else
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
    if(image->width() > 240 || image->height() > 320)
        ui->lable_bmpView->setPixmap(QPixmap::fromImage(image->scaled(240, 320, Qt::KeepAspectRatio)));
    else
        ui->lable_bmpView->setPixmap(QPixmap::fromImage(*image));
    ui->label_showPicSize->setText(QString("%1x%2").arg(image->width()).arg(image->height()));

    qDebug() << QString("[0 - %2]:%1 -> ").arg(newIndex).arg(allImageNamesList.size() - 1) + QFileInfo(selectedFilePathName).fileName();
}

/*
 * 1.前言
 * (0)RGB颜色表示
 *  (0,0,0)代表黑色，(255,255,255)代表白色；1bpp时，0代表白色，1代表黑色。
 * (1)“单色”的图像数据：
 * 单色图像数据一个象素使用一个位(1 Bit)表示，0表示白色，1表示黑色。由于数据的最小存储单位是字节，每个字节有8个位，
 * 当图像的宽或高不是8的倍数时，图像数据的宽或高将补0扩充到8的倍数以吻合字节宽度(具体是宽度还是高度取决于扫描模式)。
 * (2)“4灰”的图像数据：
 * 4灰图像数据一个象素使用两个位(2 Bit)表示，00表示白色，01表示浅灰色，10表示深灰色，11表示黑色。由于数据的最小存储单位是字节，
 * 每个字节有8个位，当图像的宽或高不是4的倍数时，图像数据的宽或高将补0扩充到4的倍数以吻合字节宽度(具体是宽度还是高度取决于扫描模式)。
 *
 * 2.图像头数据结构
 * 仿照Image2Lcd保存的图像数据组织方式：图像头数据-调色板数据-图像数据。
 * “单色/4灰/16灰/256色”的图像数据头如下：
 *
 * typedef struct _HEADGRAY
 * {
 *   unsigned char scan;
 *   unsigned char gray;
 *   unsigned short w;
 *   unsigned short h;
 * } HEADGRAY;
 *
 * scan: 扫描模式
 * Bit7: 0:自左至右扫描，1:自右至左扫描。
 * Bit6: 0:自顶至底扫描，1:自底至顶扫描。
 * Bit5: 0:字节内象素数据从高位到低位排列，1:字节内象素数据从低位到高位排列。
 * Bit4: 0:WORD类型高低位字节顺序与PC相同，1:WORD类型高低位字节顺序与PC相反。
 * Bit3~2: 保留。
 * Bit1~0: [00]水平扫描，[01]垂直扫描，[10]数据水平,字节垂直，[11]数据垂直,字节水平。
 *
 * gray: 灰度值
 *    灰度值，1:单色，2:四灰，4:十六灰，8:256色，12:4096色，16:16位彩色，24:24位彩色，32:32位彩色。
 *
 * w: 图像的宽度。
 *
 * h: 图像的高度。
 *
 * 3.函数说明
 * (1)参数destbpp为1时，C数组里一个字节代表8个像素；为2时一个字节代表4个像素
 * (2)后续若实现“扫描模式”、“字节内像素数据反序”、“高位在前(MSB First)(此参数在文件中存储WORD即16位数据时生效，主要是图片尺寸数据)”，可在此处理
 */
QString BitmapProcess::toCTypeArray(BitmapHandler& bmp, BMPBITPERPIXEL destbpp)
{
    if(!bmp.isvalid())
    {
        QMessageBox::warning(nullptr, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("不是有效的bmp文件格式"), QMessageBox::Yes);
        return QString();
    }

    if(bmp.width() > 0xffff || bmp.height() > 0xffff)
    {
        QMessageBox::warning(nullptr, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("只能处理长度和宽度均不超过0xffff大小的bmp图片"), QMessageBox::Yes);
        return QString();
    }

    if(bmp.width() > 240 || bmp.height() > 320)
        QMessageBox::information(nullptr, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("图片尺寸不在240x320范围内"), QMessageBox::Yes);

    //数组仅支持1bpp(单色位图)和2bpp(4灰度位图)
    if(destbpp > BMP_2BITSPERPIXEL)
    {
        qDebug() << QString("dest bpp > 2 not supported");
        return QString();
    }

    QList<QByteArray> rawPixels;
    BMPCALCPARAM bcp = bmp.calcparam();

    //1bpp和24bpp直接读取像素信息；32bpp只读取rgb信息，略过alpha信息
    if(BMP_1BITPERPIXEL == bmp.bitsperpixel() || BMP_24BITSPERPIXEL == bmp.bitsperpixel())
    {
        for(unsigned int i = 0; i < bmp.height(); ++i)
        {
            QByteArray scanLinePixels;
            for(int j = 0, s = bcp.totalBytesPerLine - bcp.paddingBytesPerLine; j < s; ++j)
                scanLinePixels.append(static_cast<unsigned char>(bmp.bmpdata().at(i * bcp.totalBytesPerLine + j)));

            rawPixels.append(scanLinePixels);
        }

        if(BMPSCANDIRECTION_DOWNTOUP == bmp.bmpscandirection())
            std::reverse(rawPixels.begin(), rawPixels.end());
    }
    else if(BMP_32BITSPERPIXEL == bmp.bitsperpixel())
    {
        for(unsigned int i = 0; i < bmp.height(); ++i)
        {
            QByteArray scanLinePixels;
            for(int j = 0, s = bcp.totalBytesPerLine - bcp.paddingBytesPerLine; j + 3 < s; j += 4)
            {
                for(int k = 0; k < 3; ++k)
                    scanLinePixels.append(static_cast<unsigned char>(bmp.bmpdata().at(i * bcp.totalBytesPerLine + j + k)));
            }

            rawPixels.append(scanLinePixels);
        }

        if(BMPSCANDIRECTION_DOWNTOUP == bmp.bmpscandirection())
            std::reverse(rawPixels.begin(), rawPixels.end());
    }
    else
    {
        QMessageBox::warning(nullptr, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("生成C数组仅支持处理1、24、32位/像素的bmp位图"), QMessageBox::Yes);
        return QString();
    }

    //对于1bpp需要将1个字节拆分为8个字节；对于24bpp和32bpp现在只含有rgb信息，需要将3个字节rgb转换为对应的一个字节的2等级或4等级灰度值
    if(BMP_1BITPERPIXEL == bmp.bitsperpixel())
    {
        for(int i = 0, s1 = rawPixels.size(); i < s1; ++i)
        {
            QByteArray linePixels;
            for(int j = 0, s2 = rawPixels.at(i).size(); j < s2; ++j)
            {
                unsigned char gray = static_cast<unsigned char>(rawPixels.at(i).at(j));
                for(int k = 7; k >= 0; --k)
                {
                    if(BMP_1BITPERPIXEL == destbpp)
                        linePixels.append((gray >> k) & 0x01);
                    else
                        linePixels.append(((gray >> k) & 0x01) ? 3 : 0);
                }
            }

            rawPixels.replace(i, linePixels);
        }
    }
    else
    {
        for(int i = 0; i < rawPixels.size(); ++i)
        {
            QByteArray linePixels;
            for(int j = 0, s = rawPixels.at(i).size(); j + 2 < s; j += 3)
            {
                unsigned short gray = (static_cast<unsigned char>(rawPixels.at(i).at(j))
                                       + static_cast<unsigned char>(rawPixels.at(i).at(j + 1))
                                       + static_cast<unsigned char>(rawPixels.at(i).at(j + 2))) / 3;

                gray &= 0xff;

                //原bmp非1bpp时，在接下来的数组中转意之后0/00代表白，1/11代表黑
                if(BMP_1BITPERPIXEL == destbpp)
                {
                    gray >>= 7; // gray /= 128;
                    gray = qBound(0, 1 - gray, 1);
                }
                else
                {
                    gray >>= 6; // gray /= 64;
                    gray = qBound(0, 3 - gray, 3);
                }

                linePixels.append(gray);
            }

            rawPixels.replace(i, linePixels);
        }
    }

    //在字节扫描方向上，destbpp为1时若最后一个字节不满8bit，需要补0；destbpp为2时若最后一个字节不满4bit，需要补0
    int factor = (BMP_1BITPERPIXEL == destbpp ? 8 : 4);
    if(bmp.height() % factor != 0)
    {
        QByteArray paddingLinePixels(bmp.width(), 0);
        for(int i = 0, s = factor - bmp.height() % factor; i < s; ++i)
            rawPixels.append(paddingLinePixels);
    }

    QStringList cTypeArrayText;
    QStringList unsortedCTypeArrayText;
    QString firstLineData = "static const unsigned char bmp[] = { /* 0X32,";

    if(ui->cb_isUsingPictureName->isChecked())
        firstLineData.insert(firstLineData.indexOf('['), QFileInfo(selectedFilePathName).baseName().remove(QRegExp("((?![0-9]|[a-z]|[A-Z]|_).)*")));

    firstLineData.insert(firstLineData.indexOf(']'), QString::number(rawPixels.size() * rawPixels.at(0).size() / factor));
    firstLineData.append(QString("0X0%1,").arg(destbpp));

    QByteArray ba;
    ba.append((bmp.width() >> 8) & 0xff);
    ba.append(bmp.width() & 0xff);
    ba.append((bmp.height() >> 8) & 0xff);
    ba.append(bmp.height() & 0xff);

    foreach(auto elem, ba)
        firstLineData.append((static_cast<unsigned char>(elem) <= 0x0f ? "0X0" : "0X") + QString::number(static_cast<unsigned char>(elem), 16).toUpper() + ",");

    firstLineData.append(" */");
    cTypeArrayText << firstLineData;

    qDebug() << firstLineData;

    //将像素点按指定规则重组为C数组中的数据
    for(int i = 0, s1 = rawPixels.size(); i + factor - 1 < s1; i += factor)
    {
        for(int j = 0, s2 = rawPixels.at(i).size(); j < s2; ++j)
        {
            unsigned char gray = 0;
            for(int k = factor - 1; k >= 0; --k)
                gray |= ((static_cast<unsigned char>(rawPixels.at(i + k).at(j))) & (BMP_1BITPERPIXEL == destbpp ? 0x01 : 0x03)) << (k * (BMP_1BITPERPIXEL == destbpp ? 1 : 2));

            unsortedCTypeArrayText.append((gray <= 0x0f ? "0X0" : "0X") + QString::number(gray, 16).toUpper() + ",");
        }
    }

    qDebug() << unsortedCTypeArrayText;

    int times = (0 == unsortedCTypeArrayText.size() % 16 ? unsortedCTypeArrayText.size() / 16 : unsortedCTypeArrayText.size() / 16 + 1);
    for(int i = 0; i < times; ++i)
    {
        QString tmpStr;
        QStringList subList = unsortedCTypeArrayText.mid(i * 16, 16);
        foreach(auto elem, subList)
            tmpStr.append(elem);

        cTypeArrayText << tmpStr;
    }

    if(cTypeArrayText.last().size() == 5 * 16)
        cTypeArrayText << "};";
    else
        cTypeArrayText.last().append("};");

    foreach(auto elem, cTypeArrayText)
        qDebug() << elem;

    QString saveFilePathName = QFileDialog::getSaveFileName(nullptr, "Save C Type Array",
                                                            "",
                                                            "Images (*.c)");

    qDebug() << "saveFilePathName: " << saveFilePathName;

    if(saveFilePathName.isEmpty())
    {
        QMessageBox::warning(nullptr, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("保存失败，请选择一个文件"), QMessageBox::Yes);
        return QString();
    }

    QFile saveFile(saveFilePathName);
    if(!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("不能打开文件") + saveFilePathName, QMessageBox::Yes);
        return QString();
    }

    QTextStream out(&saveFile);
    QString toClipboard;
    out << "//the last array has already been copied to the os clipboard\n\n";
    foreach(auto elem, cTypeArrayText)
    {
        out << elem << endl;
        toClipboard.append(elem).append("\n");
    }

    saveFile.close();

    if(!ui->cb_isCompress->isChecked())
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->clear();
        clipboard->setText(toClipboard);

        //WINDOWS环境下，选中该文件
#ifdef WIN32
        QProcess process;
        QString openFileName = saveFilePathName;

        openFileName.replace("/", "\\");    //***这句windows下必要***
        process.startDetached("explorer /select," + openFileName);
#endif
    }

    return saveFilePathName;
}

void BitmapProcess::compressCArrayOfBitmap(QString filepathname)
{
    if(filepathname.isEmpty())
        return;

    const int DEVIDE_AMOUNT = 255; //下位机存储宽和高各用一个字节，此处应确保此值不大于255（即0xff）
    QString filePathName = filepathname;
    QString filePath = QFileInfo(filePathName).absolutePath();

    qDebug() << filePathName << endl << filePath << endl;

    if(filePathName.isEmpty())
        return;

    QFile targetFile(filePathName);
    if(!targetFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("不能打开文件") + filePathName, QMessageBox::Yes);
        return;
    }

    QTextStream targetFileIn(&targetFile);
    QStringList originalCodeStringList;
    while(!targetFileIn.atEnd())
    {
        QString readStr = targetFileIn.readLine();

        if(!readStr.isEmpty())
            originalCodeStringList.push_back(readStr);

        qDebug() << readStr;
    }

    targetFile.close();

    if(originalCodeStringList.first().startsWith("//"))
        originalCodeStringList.removeFirst();

    QString tmpStr = originalCodeStringList.first();

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

    QString bmpFileName = originalCodeStringList.first();
    bmpFileName = bmpFileName.mid(bmpFileName.indexOf("bmp") + 3, bmpFileName.indexOf('[') - bmpFileName.indexOf("bmp") - 3);

    QStringList targetFormattedStringList;
    targetFormattedStringList << QString("static const unsigned char cbmp%1[] =\n{").arg(bmpFileName) << widthAndHeight;
    originalCodeStringList.removeFirst();
    originalCodeStringList.last().remove("};");

    QStringList tmpStringList;
    for(auto elem: originalCodeStringList)
        tmpStringList += elem.split(',');

    tmpStringList.removeAll("");
    //在结尾添加一个结束标记，否则当数据全为0x00或0xff且数据量较大时，下面的正则表达式查找第一个不为0x00或0xff的数据非常耗时
    tmpStringList.append("END_FLAG");

//    for(auto elem: tmpStringList)
//        qDebug() << elem;

    QStringList targetStringList;
    for(int index = 0, s = tmpStringList.size(); index < s; ++index)
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

    for(int index = 0, s = targetStringList.size(); index < s; ++index)
    {
        int numOfCharInElem = targetStringList.at(index).size();
        for(int cnt = 0; numOfCharInElem < 4 && cnt < 4 - numOfCharInElem; ++cnt)
            targetStringList[index].prepend(' ');

        if(0 == (index + 1) % 16)
            targetStringList[index] += ",";
        else
            targetStringList[index] += ", ";
    }

    //移除最后一个", "或","，(必然为二者之一，两个都remove即可保证)
    targetStringList.last().remove(", ");
    targetStringList.last().remove(",");

    QString tmpStr2;
    for(int index = 0, s = targetStringList.size(); index < s; ++index)
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

    QFile saveFile(filepathname);
    if(!saveFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("不能打开文件") + filepathname, QMessageBox::Yes);
        return;
    }

    QTextStream out(&saveFile);
    QString toClipboard;
    out << endl;
    foreach(auto elem, targetFormattedStringList)
    {
        out << elem << endl;
        toClipboard.append(elem).append("\n");
    }

    foreach(auto elem, targetFormattedStringList)
        qDebug() << elem;

    saveFile.close();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->clear();
    clipboard->setText(toClipboard);

    //WINDOWS环境下，选中该文件
#ifdef WIN32
    QProcess process;
    QString openFileName = filepathname;

    openFileName.replace("/", "\\");    //***这句windows下必要***
    process.startDetached("explorer /select," + openFileName);
#endif
}

