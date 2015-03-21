/************************************************************************		
\link	www.twinklingstar.cn
\author Twinkling Star
\date	2013/11/21
****************************************************************************/
#ifndef SR_IMAGES_IMAGE_BMP_H_
#define SR_IMAGES_IMAGE_BMP_H_
/** \addtogroup images
  @{
*/

#include "SrImage.h"
#include "SrColorQuant.h"

#ifndef BI_RGB
#define BI_RGB		0x00
#endif

#ifndef	BI_RLE8
#define BI_RLE8	0x01
#endif

#ifndef BI_RLE4
#define BI_RLE4	0x02
#endif



/*
\brief BMP文件的读写解析类

一个BMP类对象只能进行BMP文件的读取或者写入。

读BMP文件：解析BMP文件必需满足如下条件：
		   （1）支持像素比特数是1，4，8，16，24，32,图像压缩类型为BI_RGB
		   （2）压缩类型为BI_RLE4和BI_RLE8，对应的像素比特数分别是4和8
		   （3）BMP文件的宽和高必需大于0，biPlanes必需是1，必需是"BM"类型
写BMP文件：支持写入像素比特数是1，4，8，16，24,图像压缩类型为BI_RGB的BMP文件
*/

class SrImageBmp: public SrImage
{

protected:

	//一些BMP文件中文件头和文件信息头即使有错误也不影响正确使用BMP中的图像信息
	//是否检测文件头中位图数据的起始位置
#define  SR_IMAGE_BMP_CHECK_OFFBITS 0
	//是否检测文件头中文件大小
#define  SR_IMAGE_BMP_CHECK_FILESIZE 0

#pragma pack(push) 
#pragma pack(1)
	/*
		BMP文件由四部分组成：BMP文件头(14字节)、位图信息头(40字节)、颜色表、位图数据。
	*/

	/*
		BMP文件头(14字节)
		BMP文件头数据结构含有BMP文件的类型、文件大小和位图起始位置等信息。 
	*/
	typedef struct tagBITMAPFILEHEADER
	{
		WORD bfType;				// 位图文件的类型，必须为BMP(0-1字节)
		DWORD bfSize;				// 位图文件的大小，以字节为单位(2-5字节)
		WORD bfReserved1;			// 位图文件保留字，必须为0(6-7字节)
		WORD bfReserved2;			// 位图文件保留字，必须为0(8-9字节)
		DWORD bfOffBits;			// 位图数据的起始位置，以相对于位图文件头的偏移量表示，以字节为单位(10-13字节)
	} BITMAPFILEHEADER;

	/*
		位图信息头(40字节)
		BMP位图信息头数据用于说明位图的尺寸等信息。
	*/
	typedef struct tagBITMAPINFOHEADER
	{
		DWORD biSize;				// 本结构所占用字节数,通是40个字节(14-17字节)
		LONG biWidth;				// 位图的宽度，以像素为单位(18-21字节)
		LONG biHeight;				// 位图的高度，以像素为单位(22-25字节)
		WORD biPlanes;				// 目标设备的级别，必须为1(26-27字节)
		WORD biBitCount;			// 每个像素所需的位数，必须是1(双色)、(28-29字节)、4(16色)、8(256色)或24(真彩色)之一
		DWORD biCompression;		// 位图压缩类型，必须是 0(不压缩),(30-33字节)
									// 1(BI_RLE8压缩类型)或2(BI_RLE4压缩类型)之一
		DWORD biSizeImage;			// 位图的大小，以字节为单位(34-37字节)
		LONG biXPelsPerMeter;		// 位图水平分辨率，每米像素数(38-41字节)，解析中未使用该信息
		LONG biYPelsPerMeter;		// 位图垂直分辨率，每米像素数(42-45字节)，解析中未使用该信息
		DWORD biClrUsed;			// 位图实际使用的颜色索引表中的颜色数(46-49字节)
		DWORD biClrImportant;		// 位图显示过程中重要的颜色数(50-53字节)，解析中未使用该信息
	}BITMAPINFOHEADER;

	/*
		颜色表 
		颜色表用于说明位图中的颜色，它有若干个表项，每一个表项是一个RGBQUAD类型的结构，定义一种颜色。
	*/
	typedef struct tagRGBQUAD 
	{
		BYTE rgbBlue;			// 蓝色的亮度(值范围为0-255)
		BYTE rgbGreen;			// 绿色的亮度(值范围为0-255)
		BYTE rgbRed;			// 红色的亮度(值范围为0-255)
		BYTE rgbReserved;		// 保留，必须为0
	}RGBQUAD;
	/*
		位图信息头和颜色表组成位图信息
	*/
	typedef struct tagBITMAPINFO
	{
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD bmiColors[1];
	}BITMAPINFO;
#pragma pack(pop)

public:
	SrImageBmp(int isReadOnly);
	~SrImageBmp();
	/*
	\brief	从BMP文件中读取数据，并且返回数据
	\param[in] chPtrImageFile 读取的BMP文件名
	\param[out] ptrOutData 如果inRGBType等于IMAGE_RGB,则返回的RGB颜色数据，每个颜色占三个字节，依次为红、绿、蓝，每种颜色分量以1个字节保存；
						   如果inRGBType等于IMAGE_RGBA,则返回的RGBA颜色数据,每个颜色占四个字节，依次为红、绿、蓝，Alpha，每种颜色分量以1个字节保存;
						   不可以析构数据ptrOutDatak中的内存，它由BMP对象进行管理。
	\param[out] lgPixelCount 返回的RGB颜色数
	\param[out] inRGBType 只能是IMAGE_RGBA和IMAGE_RGB中的一个值，表示返回的数据格式是RGBA还是RGB
	\return true，读文件成功；false，读文件失败，通过getErrorId()方法，获得错误代号。
	*/
	bool	readFile(const char* chPtrImageFile,unsigned char*& ptrOutData , int& lgPixelCount,int& inRGBType);
	/*
	\brief	将RGB颜色数据保存进BMP对象中，以便后续写文件操作。
	\param[in] ucPtrRgbData RGB颜色数据，要保存进BMP对象中的数据，依次为红、绿、蓝，每种颜色分量以1个字节保存
	\param[in] inWidth 指定写入BMP对象的宽，必需大于0
	\param[in] inHeight 指定写入BMP对象的高，必需大于0
	\param[in] usBitCount 指定BMP对象中每种颜色占的位数，BMP文件只支持1,4,8,16,24,32这6个数值
	\param[in] ulCompression 这里只支持BI_RGB类型的写入文件操作
	\return true，装载文件成功；false，装载文件失败，通过getErrorId()方法，获得错误代号。
	*/
	bool	loadImageData(unsigned char* ucPtrRgbData ,long inWidth,long inHeight,unsigned short usBitCount=8 );
	/*
	\brief	将保存在BMP对象中的数据写入BMP文件中
	\param[in] chPtrImageFile 写入的BMP文件名
	\return true，写文件成功；false，写文件失败，通过getErrorId()方法，获得错误代号。
	*/
	bool	writeFile(const char* chPtrImageFile);

	virtual bool	isValid()const;
	virtual int		getWidth()const;
	virtual int		getHeight()const;
	/*
	\brief	返回BMP对象中的RGB数据或者RGBA数据
	*/
	virtual unsigned char* getImageData()const;

	unsigned long		getFileSize()const;
	/*
	\brief	判断BMP对象中BMP的压缩类型，只能是BI_RGB或者BI_RGBA
	*/
	unsigned long		getCompression()const;
	/*
	\brief	每个像素的比特数
	*/
	unsigned short		getPixelDepth()const;
	/*
	\brief	判断BMP对象中存储的是RGB数据还是RGBA数据
	*/
	bool				getIsRGB()const;


private:
	/*
	\brief	析构BMP对象中的所有数据
	*/
	void	empty();
	/*
	\brief	计算BMP文件一行的字节数
	*/
	long	getLineBytes(int imgWidth,int bitCount)const;
	/*
	\brief	判断读取的BMP文件格式是否正确
	*/
	bool	checkReadFileFormat(FILE* flFileHandle);
	bool	readUncompression(FILE* filePtrImage,unsigned char*& ptrOutData);
	bool	decodeRLE(FILE* flPtrImage,BYTE*& btPtrImageData);
	/*
	\brief	从文件中读取BMP文件头和文件信息头
	*/
	bool	readHeader(FILE* flPtrImage);
	bool	readColorMap(FILE* flPtrImage);

	bool	decodeFile(BYTE* ptrImageData,unsigned char*& rgbPtrOutData);

	bool	initHeader(long inWidth,long inHeight,unsigned short usBitCount);
	bool	writeHeader(FILE* flPtrImage);
	bool	writeColorMap(FILE* filePtrImage,SrColorQuant& colorQuant);
	bool	writeImageData(FILE* flPtrFile,SrColorQuant& colorQuant);
	bool	writeBinary(FILE* filePtrImage);
	bool	writeColorMapImage(FILE* filePtrImage,const SrColorQuant& colorQuant);
	bool	writeNoColorMapImage(FILE* filePtrImage);
	bool	checkWriteFileFormat();

public:
#ifdef _DEBUG
#ifdef _CONSOLE
	void printFileHeader(BITMAPFILEHEADER *bmfh);

	void printFileHeader(BITMAPINFOHEADER *bmih);
#endif
#endif

private:

	BITMAPINFOHEADER*	m_ptrInfoHeader;
	BITMAPFILEHEADER*	m_ptrFileHeader;
	RGBQUAD*			m_ptrColorMap;		
	BYTE*				m_ptrImageData;

	int					m_inIsReadOnly;

};

/** @} */
#endif