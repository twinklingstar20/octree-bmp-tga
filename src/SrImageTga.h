/************************************************************************		
\link	www.twinklingstar.cn
\author Twinkling Star
\date	2013/11/21
****************************************************************************/
#ifndef SR_IMAGES_IMAGE_TGA_H_
#define SR_IMAGES_IMAGE_TGA_H_

/** \addtogroup images
  @{
*/
#include "SrImage.h"
#include <stdio.h>

/*
\brief TGA文件的读写解析类

一个TGA类对象只能进行TGA文件的读取或者写入

读TGA文件：解析TGA文件必需满足如下条件：
		   （1）要求头文件中的宽和高必需是正数；
		   （2）位深度可以是8，15，16，24，32；
		   （3）如果位深度是15，16，24，32，则不使用颜色表；
		   （4）如果位深度是8，如果不采用颜色表，则只能解析黑白图，即图像类型是TGA_UN_BLACK_WHITE或者TGA_RLE_BLACK_WHITE；
		   （5）如果使用到颜色表，每个表项所占的大小必需是15，16，24，32中任意一个值
		   （6）tgaXOrigin，tgaYOrigin必需是0
		    (7）tgaImageDesc必需是0

写TGA文件：只实现了保存24bit真彩无压缩的TGA格式
*/

class SrImageTga:public SrImage
{
protected:
//定义了7种文件类型，TGA_IMAGE_NO_DATA文件类型不解析
#define		TGA_IMAGE_NO_DATA		0x00
#define		TGA_UN_COLOR_MAP		0x01
#define		TGA_UN_TRUE_COLOR		0x02
#define		TGA_UN_BLACK_WHITE		0x03
#define		TGA_RLE_COLOR_MAP		0x09
#define		TGA_RLE_TRUE_COLOR		0x0A
#define		TGA_RLE_BLACK_WHITE		0x0B

#define  SR_IMAGE_TGA_CHECK_ORIGIN		0
#define  SR_IMAGE_TGA_CHECK_IMAGEDESC	0

#pragma pack(push)
#pragma pack(1)
	//TGA文件头
	typedef struct 
	{
		BYTE tgaIdLength;			//图像信息字段长度
		BYTE tgaColorMapType;		//颜色表类型
		BYTE tgaImageType;			//图像类型
		WORD tgaFirstEnIndex;		//颜色表首地址
		WORD tgaColorMapLen;		//颜色表长度
		BYTE tgaColorMapEnSize;		//颜色表表项大小
		WORD tgaXOrigin;			//图像X位置的起始位置
		WORD tgaYOrigin;			//图像Y位置的起始位置
		WORD tgaWidth;				//图像宽度
		WORD tgaHeight;				//图像高度
		BYTE tgaPixelDepth;			//像素深度
		BYTE tgaImageDesc;			//图像描述符
	}TGAFILEHEADER;

	//TGA文件注脚
	typedef struct  
	{
		LONG tgaExtOffset;			//拓展区域偏移量
		LONG tgaDevOffset;			//开发者区域偏移量
		BYTE tgaSigniture[16];		//签名
		BYTE tgaAscii;				//ASCII码
		BYTE tgaTerminator;			//二进制数0x00
	}TGAFILEFOOTER;

	//图像、颜色表数据
	typedef struct  
	{
		BYTE*	tgaImageId;			//图像信息字段
		BYTE*	tgaColorMap;		//颜色表数据
		BYTE*	tgaImageData;		//图像数据
	}TGAIMAGECOLORDATA;

	//存储除TGA文件头以外的数据，称为文件结构数据
	typedef struct  
	{
		LONG				tgaFileSize;		//文件大小
		TGAIMAGECOLORDATA	tgaImageColorData;	//图像、颜色表数据
		BYTE*				tgaPtrDevData;		//开发者区域数据
		BYTE*				tgaPtrExtData;		//拓展区域数据
	}TGASTRUCTUREDATA;
#pragma pack(pop)

public:
	SrImageTga(int isReadOnly);
	~SrImageTga();
	/*
	\brief	从TGA文件中读取数据，并且返回数据
	\param[in] chPtrFileName 读取的TGA文件名
	\param[out] btPtrImageData 如果inRGBType等于IMAGE_RGB,则返回的RGB颜色数据，每个颜色占三个字节，依次为红、绿、蓝，每种颜色分量以1个字节保存；
							   如果inRGBType等于IMAGE_RGBA,则返回的RGBA颜色数据,每个颜色占四个字节，依次为红、绿、蓝，Alpha，每种颜色分量以1个字节保存;
							   不可以析构数据ptrOutDatak中的内存，它由BMP对象进行管理。
	\param[out] lgPixelCount 返回的RGB颜色数
	\param[out] inRGBType	只能是IMAGE_RGBA和IMAGE_RGB中的一个值，表示返回的数据格式是RGBA还是RGB
	\return true，读文件成功；false，读文件失败，通过getErrorId()方法，获得错误代号。
	*/
	bool readFile(const char* chPtrFileName,BYTE*& btPtrImageData,int& lgPixelCount,int& inRGBType);
	/*
	\brief	将RGB颜色数据保存进TGA对象中，以便后续写文件操作。
	\param[in] ucPtrRgbData RGB颜色数据，要保存进TGA对象中的数据，依次为红、绿、蓝，每种颜色分量以1个字节保存
	\param[in] inWidth 指定写入TGA对象的宽，必需大于0
	\param[in] inHeight 指定写入TGA对象的高，必需大于0
	\return true，装载文件成功；false，装载文件失败，通过getErrorId()方法，获得错误代号。
	*/
	bool loadImageData(unsigned char* ucPtrRgbData ,unsigned short inWidth,unsigned short inHeight);
	/*
	\brief 将存储在TGA文件对象中的数据保存到TGA文件中，只保存24bit真彩无压缩的TGA格式
	\param[in] chPtrImageFile 写入的文件名
	*/
	bool writeFile(const char* chPtrImageFile)const;

	virtual bool	isValid()const;
	virtual int		getWidth()const;
	virtual int		getHeight()const;
	virtual unsigned char* getImageData()const;
	/*
	\brief	判断TGA对象中存储的是RGB数据还是RGBA数据
	*/
	bool			getIsRGB()const;
	/*
	\brief	判断TGA对象是采用原始的TGA文件格式，还是新的TGA文件格式
	*/
	bool			isNewTgaFormat()const;
	/*
	\brief	每个像素的比特数
	*/
	unsigned char	getPixelDepth()const;
	/*
	\brief	TGA文件类型，总共包括6种文件类型
	*/
	unsigned char	getImageType()const;
	/*
	\brief	判断TGA是否使用了颜色表
	*/
	bool			getUseMapType()const;
	/*
	\brief	颜色表表项的位数，
	\return 0,表示非使用颜色表；否则，是15,16,24,32中的某个值
	*/
	int				getGetMapEntrySize()const;
	/*
	\brief	返回图像信息字段
	\param[out] ptrImageInfo 返回图像信息字段的内容，注意：该块内存不规TGA管理，使用完后，需要释放，否则会造成内存泄漏
	\return 图像信息字段的长度
	*/
	int				getImageInfo(unsigned char*& ptrImageInfo)const;

	

private:
	bool mallocMemory();
	void deallocMemory();

	bool readFileFooter(FILE* flPtrFile);
	bool readFileDevExt(FILE * flPtrFile);
	bool readUncompressed(FILE*flPtrFile ,BYTE* btPtrOutData);
	bool readCompressed(FILE*flPtrFile ,BYTE* btPtrOutData);
	bool readCompressedMap(FILE*flPtrFile ,BYTE* btPtrColorMap,BYTE* btPtrOutData);
	bool readUncompressedMap(FILE*flPtrFile ,BYTE* btPtrColorMap,BYTE* btPtrOutData);
	bool checkFileFormat()const;
	bool readFileImageData(FILE* flPtrFile, BYTE*& btPtrOutData, int& inRGBType );

private:
	TGAFILEHEADER*		m_ptrFileHeader;
	TGAFILEFOOTER*		m_ptrFileFooter;
	TGASTRUCTUREDATA*	m_ptrStruInfo;

	int					m_inIsReadOnly;
	BYTE*				m_ptrImageData;
};

/** @} */
#endif