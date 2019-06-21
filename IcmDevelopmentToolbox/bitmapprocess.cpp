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
    selectedFilePathName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("��ͼƬ"),
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
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����ʧ�ܣ���ѡ��һ��ͼƬ"), QMessageBox::Yes);
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
 *  (0,0,0)�����ɫ��(255,255,255)�����ɫ��1bppʱ��0�����ɫ��1�����ɫ��
 * (1)����ɫ����ͼ�����ݣ�
 * ��ɫͼ������һ������ʹ��һ��λ(1 Bit)��ʾ��0��ʾ��ɫ��1��ʾ��ɫ���������ݵ���С�洢��λ���ֽڣ�ÿ���ֽ���8��λ��
 * ��ͼ��Ŀ��߲���8�ı���ʱ��ͼ�����ݵĿ��߽���0���䵽8�ı������Ǻ��ֽڿ��(�����ǿ�Ȼ��Ǹ߶�ȡ����ɨ��ģʽ)��
 * (2)��4�ҡ���ͼ�����ݣ�
 * 4��ͼ������һ������ʹ������λ(2 Bit)��ʾ��00��ʾ��ɫ��01��ʾǳ��ɫ��10��ʾ���ɫ��11��ʾ��ɫ���������ݵ���С�洢��λ���ֽڣ�
 * ÿ���ֽ���8��λ����ͼ��Ŀ��߲���4�ı���ʱ��ͼ�����ݵĿ��߽���0���䵽4�ı������Ǻ��ֽڿ��(�����ǿ�Ȼ��Ǹ߶�ȡ����ɨ��ģʽ)��
 *
 * 2.ͼ��ͷ���ݽṹ
 * ����Image2Lcd�����ͼ��������֯��ʽ��ͼ��ͷ����-��ɫ������-ͼ�����ݡ�
 * ����ɫ/4��/16��/256ɫ����ͼ������ͷ���£�
 *
 * typedef struct _HEADGRAY
 * {
 *   unsigned char scan;
 *   unsigned char gray;
 *   unsigned short w;
 *   unsigned short h;
 * } HEADGRAY;
 *
 * scan: ɨ��ģʽ
 * Bit7: 0:��������ɨ�裬1:��������ɨ�衣
 * Bit6: 0:�Զ�����ɨ�裬1:�Ե�����ɨ�衣
 * Bit5: 0:�ֽ����������ݴӸ�λ����λ���У�1:�ֽ����������ݴӵ�λ����λ���С�
 * Bit4: 0:WORD���͸ߵ�λ�ֽ�˳����PC��ͬ��1:WORD���͸ߵ�λ�ֽ�˳����PC�෴��
 * Bit3~2: ������
 * Bit1~0: [00]ˮƽɨ�裬[01]��ֱɨ�裬[10]����ˮƽ,�ֽڴ�ֱ��[11]���ݴ�ֱ,�ֽ�ˮƽ��
 *
 * gray: �Ҷ�ֵ
 *    �Ҷ�ֵ��1:��ɫ��2:�Ļң�4:ʮ���ң�8:256ɫ��12:4096ɫ��16:16λ��ɫ��24:24λ��ɫ��32:32λ��ɫ��
 *
 * w: ͼ��Ŀ�ȡ�
 *
 * h: ͼ��ĸ߶ȡ�
 *
 * 3.����˵��
 * (1)����destbppΪ1ʱ��C������һ���ֽڴ���8�����أ�Ϊ2ʱһ���ֽڴ���4������
 * (2)������ʵ�֡�ɨ��ģʽ�������ֽ����������ݷ��򡱡�����λ��ǰ(MSB First)(�˲������ļ��д洢WORD��16λ����ʱ��Ч����Ҫ��ͼƬ�ߴ�����)�������ڴ˴���
 */
QString BitmapProcess::toCTypeArray(BitmapHandler& bmp, BMPBITPERPIXEL destbpp)
{
    if(!bmp.isvalid())
    {
        QMessageBox::warning(nullptr, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("������Ч��bmp�ļ���ʽ"), QMessageBox::Yes);
        return QString();
    }

    if(bmp.width() > 0xffff || bmp.height() > 0xffff)
    {
        QMessageBox::warning(nullptr, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("ֻ�ܴ����ȺͿ�Ⱦ�������0xffff��С��bmpͼƬ"), QMessageBox::Yes);
        return QString();
    }

    if(bmp.width() > 240 || bmp.height() > 320)
        QMessageBox::information(nullptr, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("ͼƬ�ߴ粻��240x320��Χ��"), QMessageBox::Yes);

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
        QMessageBox::warning(nullptr, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����C�����֧�ִ���1��24��32λ/���ص�bmpλͼ"), QMessageBox::Yes);
        return QString();
    }

    //����1bpp��Ҫ��1���ֽڲ��Ϊ8���ֽڣ�����24bpp��32bpp����ֻ����rgb��Ϣ����Ҫ��3���ֽ�rgbת��Ϊ��Ӧ��һ���ֽڵ�2�ȼ���4�ȼ��Ҷ�ֵ
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

                //ԭbmp��1bppʱ���ڽ�������������ת��֮��0/00����ף�1/11�����
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

    //���ֽ�ɨ�跽���ϣ�destbppΪ1ʱ�����һ���ֽڲ���8bit����Ҫ��0��destbppΪ2ʱ�����һ���ֽڲ���4bit����Ҫ��0
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

    //�����ص㰴ָ����������ΪC�����е�����
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
        QMessageBox::warning(nullptr, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����ʧ�ܣ���ѡ��һ���ļ�"), QMessageBox::Yes);
        return QString();
    }

    QFile saveFile(saveFilePathName);
    if(!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���ܴ��ļ�") + saveFilePathName, QMessageBox::Yes);
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

        //WINDOWS�����£�ѡ�и��ļ�
#ifdef WIN32
        QProcess process;
        QString openFileName = saveFilePathName;

        openFileName.replace("/", "\\");    //***���windows�±�Ҫ***
        process.startDetached("explorer /select," + openFileName);
#endif
    }

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
        return;

    QFile targetFile(filePathName);
    if(!targetFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���ܴ��ļ�") + filePathName, QMessageBox::Yes);
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
    //�ڽ�β���һ��������ǣ���������ȫΪ0x00��0xff���������ϴ�ʱ�������������ʽ���ҵ�һ����Ϊ0x00��0xff�����ݷǳ���ʱ
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

    //�Ƴ����һ��", "��","��(��ȻΪ����֮һ��������remove���ɱ�֤)
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

    QFile saveFile(filepathname);
    if(!saveFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���ܴ��ļ�") + filepathname, QMessageBox::Yes);
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

    //WINDOWS�����£�ѡ�и��ļ�
#ifdef WIN32
    QProcess process;
    QString openFileName = filepathname;

    openFileName.replace("/", "\\");    //***���windows�±�Ҫ***
    process.startDetached("explorer /select," + openFileName);
#endif
}

