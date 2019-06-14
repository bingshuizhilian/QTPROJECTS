/*
 * �ο����£�
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

typedef quint8 BYTE; //BYTE��ʾ8λ�޷���������һ���ֽ�
typedef quint16 WORD; //WORD��ʾ16λ�޷��������������ֽ�
typedef quint32 DWORD; //DWORD��ʾ32λ�޷����������ĸ����ֽ�
typedef qint32 LONG; //LONG��ʾ32λ�������ĸ��ֽ�

/*bitmapͼ���ļ����ļ�ͷ��ʽ���壬һ��14���ֽ�*/
typedef struct tagBITMAPFILEHEADER
{
    WORD bfType; //λͼ�ļ����ͣ�������Ox424D�����ַ�����BM��
    DWORD bfSize; //λͼ�ļ���С������ͷ�ļ����ĸ��ֽ�
    WORD bfReserved1; //bfReserved1��bfReserved2��Windows�����֣���ʱ����
    WORD bfReserved2;
    DWORD bfOffBits; //���ļ�ͷ��ʵ��λͼ���ݵ�ƫ���ֽ���
} BITMAPFILEHEADER;

/*λͼ��ϢͷBITMAPINFOHEADER�ṹ����*/
typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize; //���ṹ���ȣ�Ϊ40���ֽ�
    LONG biWidth; //λͼ�Ŀ�ȣ�������Ϊ��λ
    LONG biHeight; //λͼ�ĸ߶ȣ�������λ��λ(��ֵΪ���������Ե�����ɨ�裬���������Զ�����ɨ�裬����������ɨ�裬һ��Ϊ����)
    WORD biPlanes; //Ŀ���豸�ļ��𣬱�����1

    /*ÿ��������ռ��λ����
     * ��ֵ����Ϊ 1(�ڰ�ͼ��)��4(16 ɫͼ)��8(256ɫ)��24(���ɫͼ),
     * �µ� BMP ��ʽ֧�� 32 λɫ��
     */
    WORD biBitCount;

    /*λͼѹ������,��Ч��ֵΪ B
     * I_RGB(δ��ѹ��)��BI_RLE8��BI_RLE4��BI_BITFILEDS
     *(��Ϊ Windows ���峣��) ����ֻ����δ��ѹ�������, biCompression=BI_RGB��
     * */
    DWORD biCompression;
    DWORD biSizeImage; //ʵ�ʵ�λͼ����ռ�õ��ֽ���
    LONG biXPelsPerMeter; //ָ��Ŀ���豸��ˮƽ�ֱ���,��λ������/�ס�
    LONG biYPelsPerMeter; //ָ��Ŀ���豸�Ĵ�ֱ�ֱ���,��λ������/�ס�
    DWORD biClrUsed; //λͼʵ���õ�����ɫ��,�����ֵΪ��,���õ�����ɫ��Ϊ 2 �� biBitCount ���ݡ�
    DWORD biClrImportant; //λͼ��ʾ��������Ҫ����ɫ��,�����ֵΪ��,����Ϊ���е���ɫ������Ҫ�ġ�
} BITMAPINFOHEADER;

/*��ɫ��Ľṹ��
 * ��ɫ��ʵ������һ�� RGBQUAD �ṹ������,����ĳ����� biClrUsed
 * ָ��(�����ֵΪ��,���� biBitCount ָ��,�� 2 �� biBitCount ���ݸ�Ԫ��)��
 * ��ɫ��һ���� biBitCount <= 8 ʱ�Ż�ʹ��
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
    RGBQUAD *pColorTable; //��¼��ɫ���ָ��
    BYTE *bmpData; //��¼λͼ���ݵ�ָ��

private:
    bool readBitmapFile(QString filename);
    quint32 DWORDtoQuint32(DWORD n);
    quint16 WORDtoQuint16(WORD n);
    qint32 LONGtoQint32(LONG n);

public:
    bool load(QString filename);
    quint32 filesize(void); //�ļ��Ĵ�С
    quint16 bitperpixel(void); //ͼ�����ɫλ��
    quint32 datasize(void); //λͼ���ݵĴ�С
    qint32 width(void); //λͼ�Ŀ��
    qint32 height(void); //λͼ�ĸ߶�
    bool hasColorTable(void); //�Ƿ�����ɫ��
};

#endif /* BITMAPCLASS_H_ */
