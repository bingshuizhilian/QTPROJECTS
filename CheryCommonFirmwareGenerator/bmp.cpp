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
        QDataStream in(&file); //创建输入流
        in.setVersion(QDataStream::Qt_5_8);

        /*开始读取文件头信息*/
        /*===================================*/
        in >> BitMapFileHeader.bfType; //读取文件类型
        in >> BitMapFileHeader.bfSize; //读取文件大小
        in >> BitMapFileHeader.bfReserved1; //读取两个保留字
        in >> BitMapFileHeader.bfReserved2;
        in >> BitMapFileHeader.bfOffBits; //读取偏移量
        /*===================================*/
        /**文件头信息读取结束*/

        /*开始读取位图信息头*/
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
        /*位图信息头读取结束*/

        /*开始读取颜色表*/
        /*====================================*/
        quint16 bpp = WORDtoQuint16(BitMapInfoHeader.biBitCount);

        if (1 == bpp) //对于黑白图
        {
            pColorTable = new RGBQUAD[2];
            //读入颜色表
            for (int i = 0; i < 2; i++)
            {
                in >> pColorTable[i].rgbBlue;
                in >> pColorTable[i].rgbGreen;
                in >> pColorTable[i].rgbRed;
                in >> pColorTable[i].rgbReserved;
            }
        }
        else if (8 == bpp) //对于灰度图，共有256种颜色
        {
            pColorTable = new RGBQUAD[256];
            //读入颜色表
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
            ;//颜色位数大于8时没有颜色表
        }

        /*====================================*/
        /*颜色表读取结束*/

        /*开始读取位图数据*/
        /*====================================*/
        //求得图像数据的字节数
        quint32 length = DWORDtoQuint32(BitMapFileHeader.bfSize) - DWORDtoQuint32(BitMapFileHeader.bfOffBits);
        bmpData = new BYTE[length];

        for (quint32 i = 0; i < length; i++)
        {
            in >> bmpData[i];
        }
        /*====================================*/
        /*位图数据读取结束*/
    }

    file.close();

    return true;
}

/* 由于读取的二进制数是倒序的，如读取的文件大小bfSize二进制表示为36 8C 0A 00
 * 对应的实际数值的二进制应该为000A8C36h=691254B
 * 所以在使用二进制数值之前要对其进行转换
 */
quint32 BitmapHandler::DWORDtoQuint32(DWORD n)
{
    quint32 r = 0x00000000, temp = 0x00000000;

    temp = n >> 24; //取出第一个字节
    r += temp;
    temp = (n & (0x00ff0000)) >> 8; //取出第二个字节
    r += temp;
    temp = (n & (0x0000ff00)) << 8; //取出第三个字节
    r += temp;
    temp = (n & (0x000000ff)) << 24; //取出第四个字节
    r += temp;

    return r;
}

quint16 BitmapHandler::WORDtoQuint16(WORD n)
{
    quint16 r = 0x0000, temp = 0x0000;

    temp = (n >> 8) & 0x00ff; //取出第一个字节
    r += temp;
    temp = (n << 8) & 0xff00; //取出第二个字节
    r += temp;

    return r;
}

//值得一提的是在处理有符号数的时候需要考虑实现逻辑右移的问题，要不可能会出错
qint32 BitmapHandler::LONGtoQint32(LONG n)
{
    qint32 r = 0x00000000, temp = 0x00000000;

    temp = n >> 24; //取出第一个字节
    temp = temp & 0x000000ff; //为了实现逻辑右移
    r += temp;
    temp = (n & (0x00ff0000)) >> 8; //取出第二个字节
    temp = temp & 0x00ffffff; //实现右移的逻辑化
    r += temp;
    temp = (n & (0x0000ff00)) << 8; //取出第三个字节
    r += temp;
    temp = (n & (0x000000ff)) << 24; //取出第四个字节
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
