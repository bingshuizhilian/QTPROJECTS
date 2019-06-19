#include "bitmapprocess.h"
#include "ui_bitmapprocess.h"
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QClipboard>
#include <QPixmap>
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
    /*��ȡ�ļ����µ��ļ�����*/
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
 * 1.ǰ��
 * (0)RGB��ɫ��ʾ
 *  (0,0,0)�����ɫ��(255,255,255)�����ɫ
 * (1)����ɫ����ͼ�����ݣ�
 * ��ɫͼ������һ������ʹ��һ��λ(1 Bit)��ʾ��0��ʾ��ɫ��1��ʾ��ɫ���������ݵ���С�洢��λ���ֽڣ�ÿ���ֽ���8��λ��
 * ��ͼ��Ŀ��߲���8�ı���ʱ��ͼ�����ݵĿ��߽���0���䵽8�ı������Ǻ��ֽڿ��(�����ǿ�Ȼ��Ǹ߶�ȡ����ɨ��ģʽ)��
 * (2)��4�ҡ���ͼ�����ݣ�
 * 4��ͼ������һ������ʹ������λ(2 Bit)��ʾ��00��ʾ��ɫ��01��ʾǳ��ɫ��10��ʾ���ɫ��11��ʾ��ɫ���������ݵ���С�洢��λ���ֽڣ�
 * ÿ���ֽ���8��λ����ͼ��Ŀ��߲���4�ı���ʱ��ͼ�����ݵĿ��߽���0���䵽4�ı������Ǻ��ֽڿ��(�����ǿ�Ȼ��Ǹ߶�ȡ����ɨ��ģʽ)��
 *
 * 2.����˵��
 * (1)����destbppΪ1ʱ��C������һ���ֽڴ���8�����أ�Ϊ2ʱһ���ֽڴ���4������
 * (2)������ʵ�֡�ɨ��ģʽ�������ֽ����������ݷ��򡱡�����λ��ǰ(MSB First)(�˲������ļ��д洢WORD��16λ����ʱ��Ч����Ҫ��ͼƬ�ߴ�����)�������ڴ˴���
 */
QString BitmapProcess::toCTypeArray(BitmapHandler& bmp, BMPBITPERPIXEL destbpp)
{
    if(!bmp.isvalid())
    {
        QMessageBox::warning(nullptr, "Warnning", "bmp file error", QMessageBox::Yes);
        return QString();
    }

//    if(bmp.width() > 240 || bmp.height() > 320)
//        QMessageBox::warning(nullptr, "Warnning", "be careful with bmp width > 240 or height > 320, which is not fit 240x320 resolution", QMessageBox::Yes);

    //�����֧��1bpp(��ɫλͼ)��2bpp(4�Ҷ�λͼ)
    if(destbpp > BMP_2BITSPERPIXEL)
    {
        qDebug() << QString("dest bpp > 2 not supported");
        return QString();
    }

    QList<QByteArray> rawPixels;
    BMPCALCPARAM bcp = bmp.calcparam();

    //1bpp��24bppֱ�Ӷ�ȡ������Ϣ��32bppֻ��ȡrgb��Ϣ���Թ�alpha��Ϣ
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
        QMessageBox::warning(nullptr, "Warnning", "only support 1 or 24 or 32 bit/pixel format bitmap", QMessageBox::Yes);
        return QString();
    }

    //����1bpp��Ҫ��1���ֽڲ��Ϊ8���ֽڣ�����24bpp��32bpp����ֻ����rgb��Ϣ����Ҫ��3���ֽ�rgbת��Ϊ��Ӧ��һ���ֽڵ�2�ȼ���4�ȼ��Ҷ�ֵ
    if(BMP_1BITPERPIXEL == bmp.bitsperpixel())
    {
        for(int i = 0; i < rawPixels.size(); ++i)
        {
            QByteArray linePixels;
            for(int j = 0, s = rawPixels.at(i).size(); j < s; ++j)
            {
                unsigned char gray = static_cast<unsigned char>(rawPixels.at(i).at(j));
                for(int k = 7; k >= 0; --k)
                    linePixels.append((gray >> k) & 0x01);
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

                if(BMP_1BITPERPIXEL == destbpp)
                {
                    gray >>= 7; // gray /= 128;
                }
                else
                {
                    gray >>= 6; // gray /= 64;
                }

                linePixels.append(gray);
            }

            rawPixels.replace(i, linePixels);
        }
    }

    //���ֽ�ɨ�跽���ϣ�destbppΪ1ʱ�����һ���ֽڲ���8bit����Ҫ��0��destbppΪ2ʱ�����һ���ֽڲ���4bit����Ҫ��0
    int factor = (BMP_1BITPERPIXEL == destbpp ? 8 : 4);
    if(bmp.height() % factor != 0)
    {
        QByteArray paddingLinePixels(bcp.totalBytesPerLine - bcp.paddingBytesPerLine, 0);

        for(int i = 0, s = factor - bmp.height() % factor; i < s; ++i)
            rawPixels.append(paddingLinePixels);
    }

    //�����ص㰴ָ����������ΪC�����е�����
    if(BMP_1BITPERPIXEL == destbpp)
    {

    }
    else
    {

    }

    QString saveFilePathName;
//    QString saveFilePathName = QFileDialog::getSaveFileName(nullptr, "Save C Type Array",
//                                                            "",
//                                                            "Images (*.c)");

//    qDebug() << "saveFilePathName: " << saveFilePathName;

//    if(saveFilePathName.isEmpty())
//    {
//        QMessageBox::warning(nullptr, "Warnning", "save failed, please selecte a file", QMessageBox::Yes);
//        return QString();
//    }

    return saveFilePathName;
}

void BitmapProcess::compressCArrayOfBitmap(QString filepathname)
{
    if(filepathname.isEmpty())
        return;

    const int DEVIDE_AMOUNT = 255; //��λ���洢��͸߸���һ���ֽڣ��˴�Ӧȷ����ֵ������255����0xff��
    QString filePathName = filepathname;
    QString filePath = QFileInfo(filePathName).absolutePath();

    qDebug() << filePathName << endl << filePath << endl;

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

    //��δ����bmp���ߴ���256���������Ϊ3.5" TFTʵ��ʹ�õ���ͼƬ�ߴ������ⷽ���ϲ����Ҳ��ܳ���248
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
    //�ڽ�β���һ��������ǣ���������ȫΪ0x00��0xff���������ϴ�ʱ�������������ʽ���ҵ�һ����Ϊ0x00��0xff�����ݷǳ���ʱ
    tmpStringList.append("END_FLAG");

    //debug
//    for(auto elem: tmpStringList)
//        ptOutputWnd->appendPlainText(elem);

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
                        //�Ƴ�����ӵ�0x00��0xff
                        targetStringList.removeLast();
                    }
                }

                //forѭ�����ϻ���index����1����ǰ��ȥ1����������ܶ�λ����һ����0x00��0xff��Ԫ�أ���matchIndex��ָ���Ԫ��
                index = matchIndex - 1;
            }
        }
        else
        {
            targetStringList << tmpStringList.at(index);
        }
    }

    //�Ƴ����õ�"END_FLAG"
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

    //�Ƴ����һ��", "��","��(�϶�Ϊ����֮һ��������remove���ɱ�֤)
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

    //��Ⱥ͸߶���Сд��ĸ
    if(!targetFormattedStringList.isEmpty())
        targetFormattedStringList[1] = targetFormattedStringList[1].left(14) + targetFormattedStringList[1].right(targetFormattedStringList[1].size() - 14).toLower();

//    for(auto& elem: targetFormattedStringList)
//        ptOutputWnd->appendPlainText(elem);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->clear();
//    clipboard->setText(ptOutputWnd->document()->toPlainText());

//    ptOutputWnd->appendPlainText("\n\n***��������Ѿ�������ϵͳ��������***");
}

