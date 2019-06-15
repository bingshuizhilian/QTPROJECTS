#include "bmp.h"
#include <cmath>
#include <QDebug>

BitmapHandler::BitmapHandler(QString filename)
{
    bool ret = readBitmapFile(filename);

    qDebug() << "BitmapHandler construct result: " << ret;
}

BitmapHandler::~BitmapHandler()
{

}

bool BitmapHandler::readBitmapFile(QString filename)
{
    if(!filename.endsWith(".bmp", Qt::CaseInsensitive))
        return false;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }
    else
    {
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

        if (1 == bpp || 8 == bpp) //���ںڰ�ͼ
        {
            //������ɫ��
            for (int i = 0; i < pow(2, bpp); i++)
            {
                RGBQUAD ct = { 0, 0, 0, 0 };

                in >> ct.rgbBlue;
                in >> ct.rgbGreen;
                in >> ct.rgbRed;
                in >> ct.rgbReserved;

                colorTable.append(ct);
            }
        }

        qDebug() << "color table start";

        qDebug() << colorTable.size();
        foreach(auto elem, colorTable)
            qDebug() << elem.rgbRed << elem.rgbGreen << elem.rgbBlue;

        qDebug() << "color table end";

        /*====================================*/
        /*��ɫ���ȡ����*/

        /*��ʼ��ȡλͼ����*/
        /*====================================*/
        //���ͼ�����ݵ��ֽ���
        quint32 length = DWORDtoQuint32(bitmapFileHeader.bfSize) - DWORDtoQuint32(bitmapFileHeader.bfOffBits);

        for (quint32 i = 0; i < length; i++)
        {
            quint8 readByte = 0;
            in >> readByte;
            bmpData.append(readByte);
        }
        /*====================================*/
        /*λͼ���ݶ�ȡ����*/

        qDebug() << "bmp data start";

        qDebug() << bmpData.size();
        qDebug() << bmpData.toHex();

        qDebug() << "bmp data end";
    }

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

    memset(&bitmapFileHeader, 0, sizeof(BITMAPFILEHEADER));
    memset(&bitmapInfoHeader, 0, sizeof(BITMAPINFOHEADER));

    colorTable.clear();
    bmpData.clear();

    ret = readBitmapFile(filename);

    qDebug() << "BitmapHandler load result: " << ret;

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

