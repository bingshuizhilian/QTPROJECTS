/*
 * 参考文章：
 * https://www.cnblogs.com/magiccaptain/archive/2011/10/28/2228254.html
 */

#ifndef BITMAPCLASS_H_
#define BITMAPCLASS_H_

#include <QVector>
#include <QFile>
#include <QString>
#include <QMessageBox>
#include <QByteArray>
#include <QList>

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

//运算相关的重要参数，通过位图数据分析获得，可以直接调用
typedef struct tagBITMAPCALCULATEPARAMETER
{
    qint32 totalBytesPerLine; //扫描一行的总字节数（包含padding）
    qint32 paddingBytesPerLine; //扫描一行的补位的字节数，非图像有效数据区，为了windows的数据对齐而补充（扫描一行的总字节数为4的倍数）
    qint32 imageDataRealSize; //位图数据区实际大小（待确定：此值比infoheader的biSizeImage大2，观察得知可能是bmp文件尾多了2个0x00）
} BMPCALCPARAM;

enum BMPSCANDIRECTION
{
    BMPSCANDIRECTION_DOWNTOUP = 0, //自底至顶扫描，此为多数情况
    BMPSCANDIRECTION_UPTODOWN = 1 //自顶至底扫描
};

//每个像素的颜色等级用多少个bit来表示
enum BMPBITPERPIXEL
{
    BMP_1BITPERPIXEL = 1,
    BMP_2BITSPERPIXEL = 2,
    BMP_8BITSPERPIXEL = 8,
    BMP_16BITSPERPIXEL = 16,
    BMP_24BITSPERPIXEL = 24,
    BMP_32BITSPERPIXEL = 32
};

class BitmapHandler
{
public:
    explicit BitmapHandler(QString filepathname = QString());
    ~BitmapHandler();

private:
    BITMAPFILEHEADER bitmapFileHeader; //文件头
    BITMAPINFOHEADER bitmapInfoHeader; //数据信息头
    QList<RGBQUAD> colorTable; //颜色表
    QByteArray bmpData; //位图数据

private:
    //内部接口，读取文件、字节序转换
    bool readBitmapFile(QString filepathname);
    quint32 DWORDtoQuint32(DWORD n);
    quint16 WORDtoQuint16(WORD n);
    qint32 LONGtoQint32(LONG n);

public:
    //提供获取/修改图像原始数据的接口
    BITMAPFILEHEADER& fileheader(void); //文件头
    BITMAPINFOHEADER& infoheader(void); //数据信息头
    QList<RGBQUAD>& colortable(void); //颜色表
    QByteArray& bmpdata(void); //位图数据
    //提供位图相关实用接口
    bool load(QString filename); //加载位图
    bool save(void); //保存位图
    bool flipcolor(void); //翻转颜色
    void clear(void); //清空加载的位图信息
    bool isvalid(void); //当前加载的位图是否有效
    quint32 filesize(void); //文件的大小
    quint16 bitsperpixel(void); //图像的颜色位数
    quint32 datasize(void); //位图数据的大小
    qint32 width(void); //位图的宽度
    quint32 height(void); //位图的高度
    BMPSCANDIRECTION bmpscandirection(void); //位图扫描的方向
    BMPCALCPARAM calcparam(void); //位图运算相关的重要参数
};

#endif /* BITMAPCLASS_H_ */
