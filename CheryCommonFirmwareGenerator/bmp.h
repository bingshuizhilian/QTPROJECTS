/*
 * 参考文章：
 * https://www.cnblogs.com/magiccaptain/archive/2011/10/28/2228254.html
 */
//<html><head/><body><p><a href="www.baidu.com"><span style=" text-decoration: underline; color:#0000ff;">www.baidu.com</span></a></p></body></html>

#ifndef BITMAPCLASS_H_
#define BITMAPCLASS_H_

#include <QVector>
#include <QFile>
#include <QString>
#include <QMessageBox>
#include <Cmath>

typedef quint8 BYTE; //BYTE表示8位无符号整数，一个字节
typedef quint16 WORD; //WORD表示16位无符号整数，两个字节
typedef quint32 DWORD; //DWORD表示32位无符号整数，四个个字节
typedef qint32 LONG; //LONG表示32位整数，四个字节

/*bitmap图像文件的文件头格式定义，一共14个字节*/
typedef struct tagBITMAPFILEHEADER
{
    WORD bfType; //位图文件类型，必须是Ox424D，即字符创“BM”
    DWORD bfSize; //位图文件大小，包括头文件的四个字节
    WORD bfReserved1; //bfReserved1，bfReserved2，Windows保留字，暂时不用
    WORD bfReserved2;
    DWORD bfOffBits; //从文件头到实际位图数据的偏移字节数
} BITMAPFILEHEADER;

/*位图信息头BITMAPINFOHEADER结构定义*/
typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize; //本结构长度，为40个字节
    LONG biWidth; //位图的宽度，以像素为单位
    LONG biHeight; //位图的高度，以像素位单位(此值为正数代表自底至顶扫描，负数代表自顶至底扫描，均自左至右扫描，一般为正数)
    WORD biPlanes; //目标设备的级别，必须是1

    /*每个像素所占的位数，
     * 其值必须为 1(黑白图像)、4(16 色图)、8(256色)、24(真彩色图),
     * 新的 BMP 格式支持 32 位色。
     */
    WORD biBitCount;

    /*位图压缩类型,有效的值为 B
     * I_RGB(未经压缩)、BI_RLE8、BI_RLE4、BI_BITFILEDS
     *(均为 Windows 定义常量) 这里只讨论未经压缩的情况, biCompression=BI_RGB。
     * */
    DWORD biCompression;
    DWORD biSizeImage; //实际的位图数据占用的字节数
    LONG biXPelsPerMeter; //指定目标设备的水平分辨率,单位是像素/米。
    LONG biYPelsPerMeter; //指定目标设备的垂直分辨率,单位是像素/米。
    DWORD biClrUsed; //位图实际用到的颜色数,如果该值为零,则用到的颜色数为 2 的 biBitCount 次幂。
    DWORD biClrImportant; //位图显示过程中重要的颜色数,如果该值为零,则认为所有的颜色都是重要的。
} BITMAPINFOHEADER;

/*颜色表的结构体
 * 颜色表实际上是一个 RGBQUAD 结构的数组,数组的长度由 biClrUsed
 * 指定(如果该值为零,则由 biBitCount 指定,即 2 的 biBitCount 次幂个元素)，
 * 颜色表一般在 biBitCount <= 8 时才会使用
 * */
typedef struct tagRGBQUAD
{
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

class BitmapHandler
{
public:
    explicit BitmapHandler(QString filename = "");
    ~BitmapHandler();

private:
    BITMAPFILEHEADER BitMapFileHeader;
    BITMAPINFOHEADER BitMapInfoHeader;
    RGBQUAD *pColorTable; //记录颜色表的指针
    BYTE *bmpData; //记录位图数据的指针

private:
    bool readBitmapFile(QString filename);
    quint32 DWORDtoQuint32(DWORD n);
    quint16 WORDtoQuint16(WORD n);
    qint32 LONGtoQint32(LONG n);

public:
    bool load(QString filename);
    quint32 filesize(void); //文件的大小
    quint16 bitperpixel(void); //图像的颜色位数
    quint32 datasize(void); //位图数据的大小
    qint32 width(void); //位图的宽度
    qint32 height(void); //位图的高度
    bool hasColorTable(void); //是否有颜色表
};

#endif /* BITMAPCLASS_H_ */
