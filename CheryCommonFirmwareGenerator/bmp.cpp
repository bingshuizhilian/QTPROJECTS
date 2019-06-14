#include "bmp.h"
#include <QDebug>

BitmapHandler::BitmapHandler(QString filename) :
    pColorTable(nullptr),
    bmpData(nullptr)
{
    memset(&BitMapFileHeader, 0, sizeof(BITMAPFILEHEADER));
    memset(&BitMapInfoHeader, 0, sizeof(BITMAPINFOHEADER));

    bool ret = readBitmapFile(filename);

    qDebug() << "BitmapHandler construct result: " << ret;
}

BitmapHandler::~BitmapHandler()
{
    if(nullptr != pColorTable)
        delete pColorTable;

    if(nullptr != bmpData)
        delete bmpData;
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
        in >> BitMapFileHeader.bfType; //��ȡ�ļ�����
        in >> BitMapFileHeader.bfSize; //��ȡ�ļ���С
        in >> BitMapFileHeader.bfReserved1; //��ȡ����������
        in >> BitMapFileHeader.bfReserved2;
        in >> BitMapFileHeader.bfOffBits; //��ȡƫ����
        /*===================================*/
        /**�ļ�ͷ��Ϣ��ȡ����*/

        /*��ʼ��ȡλͼ��Ϣͷ*/
        /*====================================*/
        in >> BitMapInfoHeader.biSize;
        in >> BitMapInfoHeader.biWidth;
        in >> BitMapInfoHeader.biHeight;
        in >> BitMapInfoHeader.biPlanes;
        in >> BitMapInfoHeader.biBitCount;
        in >> BitMapInfoHeader.biCompression;
        in >> BitMapInfoHeader.biSizeImage;
        in >> BitMapInfoHeader.biXPelsPerMeter;
        in >> BitMapInfoHeader.biYPelsPerMeter;
        in >> BitMapInfoHeader.biClrUsed;
        in >> BitMapInfoHeader.biClrImportant;
        /*====================================*/
        /*λͼ��Ϣͷ��ȡ����*/

        /*��ʼ��ȡ��ɫ��*/
        /*====================================*/
        quint16 bpp = WORDtoQuint16(BitMapInfoHeader.biBitCount);

        if (1 == bpp) //���ںڰ�ͼ
        {
            pColorTable = new RGBQUAD[2];
            //������ɫ��
            for (int i = 0; i < 2; i++)
            {
                in >> pColorTable[i].rgbBlue;
                in >> pColorTable[i].rgbGreen;
                in >> pColorTable[i].rgbRed;
                in >> pColorTable[i].rgbReserved;
            }
        }
        else if (8 == bpp) //���ڻҶ�ͼ������256����ɫ
        {
            pColorTable = new RGBQUAD[256];
            //������ɫ��
            for (int i = 0; i < 256; i++)
            {
                in >> pColorTable[i].rgbBlue;
                in >> pColorTable[i].rgbGreen;
                in >> pColorTable[i].rgbRed;
                in >> pColorTable[i].rgbReserved;
            }
        }
        else
        {
            ;//��ɫλ������8ʱû����ɫ��
        }

        /*====================================*/
        /*��ɫ���ȡ����*/

        /*��ʼ��ȡλͼ����*/
        /*====================================*/
        //���ͼ�����ݵ��ֽ���
        quint32 length = DWORDtoQuint32(BitMapFileHeader.bfSize) - DWORDtoQuint32(BitMapFileHeader.bfOffBits);
        bmpData = new BYTE[length];

        for (quint32 i = 0; i < length; i++)
        {
            in >> bmpData[i];
        }
        /*====================================*/
        /*λͼ���ݶ�ȡ����*/
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

bool BitmapHandler::load(QString filename)
{
    bool ret = false;

    memset(&BitMapFileHeader, 0, sizeof(BITMAPFILEHEADER));
    memset(&BitMapInfoHeader, 0, sizeof(BITMAPINFOHEADER));

    if(nullptr != pColorTable)
    {
        delete pColorTable;
        pColorTable = nullptr;
    }

    if(nullptr != bmpData)
    {
        delete bmpData;
        bmpData = nullptr;
    }

    ret = readBitmapFile(filename);

    qDebug() << "BitmapHandler load result: " << ret;

    return ret;
}

quint32 BitmapHandler::filesize(void)
{
    return DWORDtoQuint32(BitMapFileHeader.bfSize);
}

quint16 BitmapHandler::bitperpixel(void)
{
    return WORDtoQuint16(BitMapInfoHeader.biBitCount);
}

quint32 BitmapHandler::datasize(void)
{
    return DWORDtoQuint32(BitMapFileHeader.bfSize) - DWORDtoQuint32(BitMapFileHeader.bfOffBits);
}

qint32 BitmapHandler::width(void)
{
    return LONGtoQint32(BitMapInfoHeader.biWidth);
}

qint32 BitmapHandler::height(void)
{
    return LONGtoQint32(BitMapInfoHeader.biHeight);
}

bool BitmapHandler::hasColorTable()
{
    bool ret = false;

    if(1 == bitperpixel() || 8 == bitperpixel())
        ret = true;

    return ret;
}
