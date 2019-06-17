#include "bmp.h"
#include <cmath>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

BitmapHandler::BitmapHandler(QString filepathname)
{
    bool ret = readBitmapFile(filepathname);

    qDebug() << "BitmapHandler construct result: " << ret;
}

BitmapHandler::~BitmapHandler()
{

}

bool BitmapHandler::readBitmapFile(QString filepathname)
{
    if(!filepathname.endsWith(".bmp", Qt::CaseInsensitive))
        return false;

    QFile file(filepathname);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QDataStream in(&file); //����������
    in.setVersion(QDataStream::Qt_5_8);

    /*��ʼ��ȡ�ļ�ͷ��Ϣ*/
    /*===================================*/
    in >> bitmapFileHeader.bfType; //��ȡ�ļ�����
    in >> bitmapFileHeader.bfSize; //��ȡ�ļ���С
    in >> bitmapFileHeader.bfReserved1; //��ȡ����������
    in >> bitmapFileHeader.bfReserved2;
    in >> bitmapFileHeader.bfOffBits; //��ȡƫ����
    /*===================================*/
    /**�ļ�ͷ��Ϣ��ȡ����*/

    /*��ʼ��ȡλͼ��Ϣͷ*/
    /*====================================*/
    in >> bitmapInfoHeader.biSize;
    in >> bitmapInfoHeader.biWidth;
    in >> bitmapInfoHeader.biHeight;
    in >> bitmapInfoHeader.biPlanes;
    in >> bitmapInfoHeader.biBitCount;
    in >> bitmapInfoHeader.biCompression;
    in >> bitmapInfoHeader.biSizeImage;
    in >> bitmapInfoHeader.biXPelsPerMeter;
    in >> bitmapInfoHeader.biYPelsPerMeter;
    in >> bitmapInfoHeader.biClrUsed;
    in >> bitmapInfoHeader.biClrImportant;
    /*====================================*/
    /*λͼ��Ϣͷ��ȡ����*/

    /*��ʼ��ȡ��ɫ��*/
    /*====================================*/
    quint16 bpp = WORDtoQuint16(bitmapInfoHeader.biBitCount);

    if(BMP_1BITPERPIXEL == bpp || BMP_8BITSPERPIXEL == bpp) //���ںڰ�ͼ
    {
        //������ɫ��
        for(int i = 0; i < pow(2, bpp); i++)
        {
            RGBQUAD ct = { 0, 0, 0, 0 };

            in >> ct.rgbBlue;
            in >> ct.rgbGreen;
            in >> ct.rgbRed;
            in >> ct.rgbReserved;

            colorTable.append(ct);
        }
    }

    qDebug() << "read color table start";

    qDebug() << colorTable.size();

    foreach(auto elem, colorTable)
        qDebug() << elem.rgbRed << elem.rgbGreen << elem.rgbBlue;

    qDebug() << "read color table end";

    /*====================================*/
    /*��ɫ���ȡ����*/

    /*��ʼ��ȡλͼ����*/
    /*====================================*/
    //���ͼ�����ݵ��ֽ���
    quint32 length = DWORDtoQuint32(bitmapFileHeader.bfSize) - DWORDtoQuint32(bitmapFileHeader.bfOffBits);

    for(quint32 i = 0; i < length; i++)
    {
        quint8 readByte = 0;
        in >> readByte;
        bmpData.append(readByte);
    }
    /*====================================*/
    /*λͼ���ݶ�ȡ����*/

    qDebug() << "read bmp data start";

    qDebug() << bmpData.size();
    qDebug() << bmpData.toHex();

    qDebug() << "read bmp data end";

    file.close();

    return true;
}

/* ���ڶ�ȡ�Ķ��������ǵ���ģ����ȡ���ļ���СbfSize�����Ʊ�ʾΪ36 8C 0A 00
 * ��Ӧ��ʵ����ֵ�Ķ�����Ӧ��Ϊ000A8C36h=691254B
 * ������ʹ�ö�������ֵ֮ǰҪ�������ת��
 */
quint32 BitmapHandler::DWORDtoQuint32(DWORD n)
{
    quint32 r = 0x00000000, temp = 0x00000000;

    temp = n >> 24; //ȡ����һ���ֽ�
    r += temp;
    temp = (n & (0x00ff0000)) >> 8; //ȡ���ڶ����ֽ�
    r += temp;
    temp = (n & (0x0000ff00)) << 8; //ȡ���������ֽ�
    r += temp;
    temp = (n & (0x000000ff)) << 24; //ȡ�����ĸ��ֽ�
    r += temp;

    return r;
}

quint16 BitmapHandler::WORDtoQuint16(WORD n)
{
    quint16 r = 0x0000, temp = 0x0000;

    temp = (n >> 8) & 0x00ff; //ȡ����һ���ֽ�
    r += temp;
    temp = (n << 8) & 0xff00; //ȡ���ڶ����ֽ�
    r += temp;

    return r;
}

//ֵ��һ������ڴ����з�������ʱ����Ҫ����ʵ���߼����Ƶ����⣬Ҫ�����ܻ����
qint32 BitmapHandler::LONGtoQint32(LONG n)
{
    qint32 r = 0x00000000, temp = 0x00000000;

    temp = n >> 24; //ȡ����һ���ֽ�
    temp = temp & 0x000000ff; //Ϊ��ʵ���߼�����
    r += temp;
    temp = (n & (0x00ff0000)) >> 8; //ȡ���ڶ����ֽ�
    temp = temp & 0x00ffffff; //ʵ�����Ƶ��߼���
    r += temp;
    temp = (n & (0x0000ff00)) << 8; //ȡ���������ֽ�
    r += temp;
    temp = (n & (0x000000ff)) << 24; //ȡ�����ĸ��ֽ�
    r += temp;

    return r;
}

BITMAPFILEHEADER& BitmapHandler::fileheader(void)
{
    return bitmapFileHeader;
}

BITMAPINFOHEADER& BitmapHandler::infoheader(void)
{
    return bitmapInfoHeader;
}

QList<RGBQUAD>& BitmapHandler::colortable(void)
{
    return colorTable;
}

QByteArray& BitmapHandler::bmpdata(void)
{
    return bmpData;
}

bool BitmapHandler::load(QString filename)
{
    bool ret = false;

    this->clear();
    ret = readBitmapFile(filename);

    qDebug() << "BitmapHandler load result: " << ret;

    if(ret)
    {
        qDebug() << QString("filesize:%1, bpp:%2, datasize:%3, width:%4, height:%5")
                    .arg(this->filesize())
                    .arg(this->bitsperpixel())
                    .arg(this->datasize())
                    .arg(this->width())
                    .arg(this->height());

        this->calcparam();
        this->bmpscandirection();
    }

    return ret;
}

bool BitmapHandler::save(void)
{
    if(!this->isvalid())
    {
        return false;
    }

    QString saveFilePathName = QFileDialog::getSaveFileName(nullptr, "Save Picture",
                                                            "",
                                                            "Images (*.bmp)");

    qDebug() << "saveFilePathName" << saveFilePathName;

    QFile file(saveFilePathName);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QDataStream out(&file); //���������
    out.setVersion(QDataStream::Qt_5_8);

    /*��ʼд���ļ�ͷ��Ϣ*/
    /*===================================*/
    out << bitmapFileHeader.bfType; //д���ļ�����
    out << bitmapFileHeader.bfSize; //д���ļ���С
    out << bitmapFileHeader.bfReserved1; //д������������
    out << bitmapFileHeader.bfReserved2;
    out << bitmapFileHeader.bfOffBits; //д��ƫ����
    /*===================================*/
    /**�ļ�ͷ��Ϣд�����*/

    /*��ʼд��λͼ��Ϣͷ*/
    /*====================================*/
    out << bitmapInfoHeader.biSize;
    out << bitmapInfoHeader.biWidth;
    out << bitmapInfoHeader.biHeight;
    out << bitmapInfoHeader.biPlanes;
    out << bitmapInfoHeader.biBitCount;
    out << bitmapInfoHeader.biCompression;
    out << bitmapInfoHeader.biSizeImage;
    out << bitmapInfoHeader.biXPelsPerMeter;
    out << bitmapInfoHeader.biYPelsPerMeter;
    out << bitmapInfoHeader.biClrUsed;
    out << bitmapInfoHeader.biClrImportant;
    /*====================================*/
    /*λͼ��Ϣͷд�����*/

    /*��ʼд����ɫ��*/
    /*====================================*/
    if(!colorTable.isEmpty()) //���ںڰ�ͼ
    {
        //д����ɫ��
        foreach(auto elem, colorTable)
        {
            out << elem.rgbBlue;
            out << elem.rgbGreen;
            out << elem.rgbRed;
            out << elem.rgbReserved;
        }
    }

    qDebug() << "write color table start";

    qDebug() << colorTable.size();

    foreach(auto elem, colorTable)
        qDebug() << elem.rgbBlue << elem.rgbGreen << elem.rgbRed;

    qDebug() << "write color table end";

    /*====================================*/
    /*��ɫ��д�����*/

    /*��ʼд��λͼ����*/
    /*====================================*/

    out.writeRawData(bmpData, bmpData.size());

    /*====================================*/
    /*λͼ����д�����*/

    qDebug() << "write bmp data start";

    qDebug() << bmpData.size();
    qDebug() << bmpData.toHex();

    qDebug() << "write bmp data end";

    file.close();

    return true;
}

bool BitmapHandler::flipcolor(void)
{
    if(!this->isvalid())
    {
        return false;
    }

    if(!colorTable.isEmpty() && this->bitsperpixel() <= BMP_8BITSPERPIXEL) //����ɫ���ֱ������ɫ�����ɫȡ��
    {
        for(auto& elem: colorTable)
        {
            elem.rgbBlue = qBound(0, 255 - elem.rgbBlue, 255);
            elem.rgbGreen = qBound(0, 255 - elem.rgbGreen, 255);
            elem.rgbRed = qBound(0, 255 - elem.rgbRed, 255);
        }
    }
    else if(BMP_24BITSPERPIXEL == this->bitsperpixel()) //����ɫ��ģ�ֱ����������ɫ��Ϣ��ȡ����24λʱֻ����RGB��Ϣ
    {
        BMPCALCPARAM bcp = this->calcparam();

        for(int i = 0; i < this->height(); ++i)
        {
            for(int j = 0; j < bcp.totalBytesPerLine - bcp.paddingBytesPerLine; ++j)
            {
                bmpData[i * bcp.totalBytesPerLine + j] = qBound(0, 255 - static_cast<unsigned char>(bmpData.at(i * bcp.totalBytesPerLine + j)), 255);
            }
        }
    }
    else if(BMP_32BITSPERPIXEL == this->bitsperpixel()) //����ɫ��ģ�ֱ����������ɫ��Ϣ��ȡ����32λʱ��Ҫע��alphaͨ��������
    {
        BMPCALCPARAM bcp = this->calcparam();

        for(int i = 0; i < this->height(); ++i)
        {
            for(int j = 0; j + 3 < bcp.totalBytesPerLine - bcp.paddingBytesPerLine; j += 4)
            {
                for(int k = 0; k < 3; ++k)
                    bmpData[i * bcp.totalBytesPerLine + j + k] = qBound(0, 255 - static_cast<unsigned char>(bmpData.at(i * bcp.totalBytesPerLine + j + k)), 255);
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}

QString BitmapHandler::toctypearray(BMPBITPERPIXEL bpp)
{
    if(!this->isvalid() || bpp > BMP_2BITSPERPIXEL)
    {
        QMessageBox::warning(nullptr, "Warnning", "bmp file error", QMessageBox::Yes);
        return QString();
    }

    QString saveFilePathName = QFileDialog::getSaveFileName(nullptr, "Save C Type Array",
                                                            "",
                                                            "Images (*.txt *.c *.h)");

    if(saveFilePathName.isEmpty())
    {
        QMessageBox::warning(nullptr, "Warnning", "save failed, please selecte a file", QMessageBox::Yes);
        return QString();
    }

    qDebug() << saveFilePathName;

    return saveFilePathName;
}

void BitmapHandler::clear(void)
{
    memset(&bitmapFileHeader, 0, sizeof(BITMAPFILEHEADER));
    memset(&bitmapInfoHeader, 0, sizeof(BITMAPINFOHEADER));
    colorTable.clear();
    bmpData.clear();
}

bool BitmapHandler::isvalid(void)
{
    bool ret = true;

    if(0x424D != bitmapFileHeader.bfType
            || 0x28 != DWORDtoQuint32(bitmapInfoHeader.biSize)
            || (colorTable.isEmpty() && this->bitsperpixel() <= BMP_8BITSPERPIXEL)
            || (!colorTable.isEmpty() && this->bitsperpixel() > BMP_8BITSPERPIXEL)
            || bmpData.isEmpty())
    {
        ret = false;
    }

    return ret;
}

quint32 BitmapHandler::filesize(void)
{
    return DWORDtoQuint32(bitmapFileHeader.bfSize);
}

quint16 BitmapHandler::bitsperpixel(void)
{
    return WORDtoQuint16(bitmapInfoHeader.biBitCount);
}

quint32 BitmapHandler::datasize(void)
{
    return DWORDtoQuint32(bitmapFileHeader.bfSize) - DWORDtoQuint32(bitmapFileHeader.bfOffBits);
}

qint32 BitmapHandler::width(void)
{
    return LONGtoQint32(bitmapInfoHeader.biWidth);
}

qint32 BitmapHandler::height(void)
{
    return abs(LONGtoQint32(bitmapInfoHeader.biHeight));
}

BMPSCANDIRECTION BitmapHandler::bmpscandirection(void)
{
    BMPSCANDIRECTION bsd = BMPSCANDIRECTION_DOWNTOUP;

    if(LONGtoQint32(bitmapInfoHeader.biHeight) < 0)
        bsd = BMPSCANDIRECTION_UPTODOWN;

    qDebug() << bsd;

    return bsd;
}

BMPCALCPARAM BitmapHandler::calcparam()
{
    BMPCALCPARAM bcp;

    bcp.totalBytesPerLine = (((this->width() * this->bitsperpixel()) + 31) >> 5) << 2;
    bcp.paddingBytesPerLine = (4 - ((this->width() * this->bitsperpixel()) >> 3)) & 3;
    bcp.imageDataRealSize = bcp.totalBytesPerLine * this->height();

    qDebug() << QString("totalBytesPerLine: %1, paddingBytesPerLine: %2, imageDataRealSize: %3")
                .arg(bcp.totalBytesPerLine)
                .arg(bcp.paddingBytesPerLine)
                .arg(bcp.imageDataRealSize);

    return bcp;
}

