/*
 * �ο����£�
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

//������ص���Ҫ������ͨ��λͼ���ݷ�����ã�����ֱ�ӵ���
typedef struct tagBITMAPCALCULATEPARAMETER
{
    qint32 totalBytesPerLine; //ɨ��һ�е����ֽ���������padding��
    qint32 paddingBytesPerLine; //ɨ��һ�еĲ�λ���ֽ�������ͼ����Ч��������Ϊ��windows�����ݶ�������䣨ɨ��һ�е����ֽ���Ϊ4�ı�����
    qint32 imageDataRealSize; //λͼ������ʵ�ʴ�С����ȷ������ֵ��infoheader��biSizeImage��2���۲��֪������bmp�ļ�β����2��0x00��
} BMPCALCPARAM;

enum BMPSCANDIRECTION
{
    BMPSCANDIRECTION_DOWNTOUP = 0, //�Ե�����ɨ�裬��Ϊ�������
    BMPSCANDIRECTION_UPTODOWN = 1 //�Զ�����ɨ��
};

//ÿ�����ص���ɫ�ȼ��ö��ٸ�bit����ʾ
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
    BITMAPFILEHEADER bitmapFileHeader; //�ļ�ͷ
    BITMAPINFOHEADER bitmapInfoHeader; //������Ϣͷ
    QList<RGBQUAD> colorTable; //��ɫ��
    QByteArray bmpData; //λͼ����

private:
    //�ڲ��ӿڣ���ȡ�ļ����ֽ���ת��
    bool readBitmapFile(QString filepathname);
    quint32 DWORDtoQuint32(DWORD n);
    quint16 WORDtoQuint16(WORD n);
    qint32 LONGtoQint32(LONG n);

public:
    //�ṩ��ȡ/�޸�ͼ��ԭʼ���ݵĽӿ�
    BITMAPFILEHEADER& fileheader(void); //�ļ�ͷ
    BITMAPINFOHEADER& infoheader(void); //������Ϣͷ
    QList<RGBQUAD>& colortable(void); //��ɫ��
    QByteArray& bmpdata(void); //λͼ����
    //�ṩλͼ���ʵ�ýӿ�
    bool load(QString filename); //����λͼ
    bool save(void); //����λͼ
    bool flipcolor(void); //��ת��ɫ
    void clear(void); //��ռ��ص�λͼ��Ϣ
    bool isvalid(void); //��ǰ���ص�λͼ�Ƿ���Ч
    quint32 filesize(void); //�ļ��Ĵ�С
    quint16 bitsperpixel(void); //ͼ�����ɫλ��
    quint32 datasize(void); //λͼ���ݵĴ�С
    qint32 width(void); //λͼ�Ŀ��
    quint32 height(void); //λͼ�ĸ߶�
    BMPSCANDIRECTION bmpscandirection(void); //λͼɨ��ķ���
    BMPCALCPARAM calcparam(void); //λͼ������ص���Ҫ����
};

#endif /* BITMAPCLASS_H_ */
