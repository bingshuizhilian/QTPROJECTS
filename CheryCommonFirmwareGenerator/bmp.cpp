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

    QDataStream in(&file); //创建输入流
    in.setVersion(QDataStream::Qt_5_8);

    /*开始读取文件头信息*/
    /*===================================*/
    in >> bitmapFileHeader.bfType; //读取文件类型
    in >> bitmapFileHeader.bfSize; //读取文件大小
    in >> bitmapFileHeader.bfReserved1; //读取两个保留字
    in >> bitmapFileHeader.bfReserved2;
    in >> bitmapFileHeader.bfOffBits; //读取偏移量
    /*===================================*/
    /**文件头信息读取结束*/

    /*开始读取位图信息头*/
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
    /*位图信息头读取结束*/

    /*开始读取颜色表*/
    /*====================================*/
    quint16 bpp = WORDtoQuint16(bitmapInfoHeader.biBitCount);

    if(BMP_1BITPERPIXEL == bpp || BMP_8BITSPERPIXEL == bpp) //对于黑白图
    {
        //读入颜色表
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
    /*颜色表读取结束*/

    /*开始读取位图数据*/
    /*====================================*/
    //求得图像数据的字节数
    quint32 length = DWORDtoQuint32(bitmapFileHeader.bfSize) - DWORDtoQuint32(bitmapFileHeader.bfOffBits);

    for(quint32 i = 0; i < length; i++)
    {
        quint8 readByte = 0;
        in >> readByte;
        bmpData.append(readByte);
    }
    /*====================================*/
    /*位图数据读取结束*/

    qDebug() << "read bmp data start";

    qDebug() << bmpData.size();
    qDebug() << bmpData.toHex();

    qDebug() << "read bmp data end";

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

    QDataStream out(&file); //创建输出流
    out.setVersion(QDataStream::Qt_5_8);

    /*开始写入文件头信息*/
    /*===================================*/
    out << bitmapFileHeader.bfType; //写入文件类型
    out << bitmapFileHeader.bfSize; //写入文件大小
    out << bitmapFileHeader.bfReserved1; //写入两个保留字
    out << bitmapFileHeader.bfReserved2;
    out << bitmapFileHeader.bfOffBits; //写入偏移量
    /*===================================*/
    /**文件头信息写入结束*/

    /*开始写入位图信息头*/
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
    /*位图信息头写入结束*/

    /*开始写入颜色表*/
    /*====================================*/
    if(!colorTable.isEmpty()) //对于黑白图
    {
        //写入颜色表
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
    /*颜色表写入结束*/

    /*开始写入位图数据*/
    /*====================================*/

    out.writeRawData(bmpData, bmpData.size());

    /*====================================*/
    /*位图数据写入结束*/

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

    if(!colorTable.isEmpty() && this->bitsperpixel() <= BMP_8BITSPERPIXEL) //有颜色表的直接在颜色表对颜色取反
    {
        for(auto& elem: colorTable)
        {
            elem.rgbBlue = qBound(0, 255 - elem.rgbBlue, 255);
            elem.rgbGreen = qBound(0, 255 - elem.rgbGreen, 255);
            elem.rgbRed = qBound(0, 255 - elem.rgbRed, 255);
        }
    }
    else if(BMP_24BITSPERPIXEL == this->bitsperpixel()) //无颜色表的，直接在像素颜色信息上取反，24位时只处理RGB信息
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
    else if(BMP_32BITSPERPIXEL == this->bitsperpixel()) //无颜色表的，直接在像素颜色信息上取反，32位时需要注意alpha通道不处理
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

