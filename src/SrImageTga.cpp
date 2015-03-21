/************************************************************************		
\link	www.twinklingstar.cn
\author Twinkling Star
\date	2013/11/21
****************************************************************************/
#include "SrImageTga.h"
#include <assert.h>

SrImageTga::SrImageTga(int isReadOnly):SrImage()
{
	m_ptrFileHeader		=	NULL;
	m_ptrFileFooter		=	NULL;
	m_ptrStruInfo		=	NULL;
	m_inError			=	NULL;
	m_ptrImageData		=	NULL;

	m_inIsReadOnly		=	isReadOnly;
}

SrImageTga::~SrImageTga()
{
	deallocMemory();
}

bool	SrImageTga::isValid()const
{
	return m_ptrImageData!=NULL;
}

bool	SrImageTga::isNewTgaFormat()const
{
	return m_ptrFileFooter!=NULL;
}

int		SrImageTga::getWidth()const
{
	assert(m_ptrFileHeader);
	return m_ptrFileHeader->tgaWidth;
}

int		SrImageTga::getHeight()const
{
	assert(m_ptrFileHeader);
	return m_ptrFileHeader->tgaHeight;
}

unsigned char* SrImageTga::getImageData()const
{
	return m_ptrImageData;
}

bool	SrImageTga::getIsRGB()const
{
	assert(m_ptrFileHeader);
	return !(m_ptrFileHeader->tgaPixelDepth ==32 || m_ptrFileHeader->tgaColorMapEnSize==32);
}

unsigned char	SrImageTga::getPixelDepth()const
{
	assert(m_ptrFileHeader);
	return m_ptrFileHeader->tgaPixelDepth;
}

unsigned char	SrImageTga::getImageType()const
{
	assert(m_ptrFileHeader);
	return m_ptrFileHeader->tgaImageType;
}

bool			SrImageTga::getUseMapType()const
{
	assert(m_ptrFileHeader);
	return m_ptrFileHeader->tgaColorMapType==1;
}

int				SrImageTga::getGetMapEntrySize()const
{
	assert(m_ptrFileHeader);
	return m_ptrFileHeader->tgaColorMapEnSize;
}

int				SrImageTga::getImageInfo(unsigned char*& ptrImageInfo)const
{
	assert(m_ptrFileHeader);
	assert(m_ptrStruInfo);
	if( m_ptrFileHeader->tgaIdLength )
	{
		ptrImageInfo = (unsigned char*)malloc(m_ptrFileHeader->tgaIdLength+1);
		int i;
		for( i=0 ; i<m_ptrFileHeader->tgaIdLength ; i++ )
			ptrImageInfo[i] = m_ptrStruInfo->tgaImageColorData.tgaImageId[i];
		ptrImageInfo[i] = '\0';
	}
	return 0;
}
bool SrImageTga::mallocMemory()
{
	m_ptrStruInfo = (TGASTRUCTUREDATA*)malloc(sizeof(TGASTRUCTUREDATA));
	if( !m_ptrStruInfo )
	{
		m_inError =IMAGE_NO_MEMORY;
		return false;
	}
	memset(m_ptrStruInfo,0,sizeof(TGASTRUCTUREDATA));
	m_ptrFileHeader = (TGAFILEHEADER*)malloc(sizeof(TGAFILEHEADER));
	if( !m_ptrFileHeader )
	{
		m_inError = IMAGE_NO_MEMORY;
		return 0;
	}
	memset(m_ptrFileHeader,0,sizeof(TGAFILEHEADER));
	m_ptrFileFooter = (TGAFILEFOOTER*)malloc(sizeof(TGAFILEFOOTER));
	if( !m_ptrFileFooter )
	{
		m_inError = IMAGE_NO_MEMORY;
		return 0;
	}
	memset(m_ptrFileFooter,0,sizeof(TGAFILEFOOTER));
	return true;
}

void SrImageTga::deallocMemory()
{
	if( m_ptrFileHeader )
	{
		free(m_ptrFileHeader);
		m_ptrFileHeader = NULL;
	}
	if( m_ptrStruInfo )
	{
		if( m_ptrStruInfo->tgaPtrDevData )
		{
			free(m_ptrStruInfo->tgaPtrDevData);
			m_ptrStruInfo->tgaPtrDevData = NULL;

		}
		if( m_ptrStruInfo->tgaPtrExtData )
		{
			free(m_ptrStruInfo->tgaPtrExtData);
			m_ptrStruInfo->tgaPtrExtData = NULL;
		}
		if( m_ptrStruInfo->tgaImageColorData.tgaImageId )
		{
			free(m_ptrStruInfo->tgaImageColorData.tgaImageId);
			m_ptrStruInfo->tgaImageColorData.tgaImageId = NULL;
		}
		if( m_ptrStruInfo->tgaImageColorData.tgaColorMap )
		{
			free(m_ptrStruInfo->tgaImageColorData.tgaColorMap);
			m_ptrStruInfo->tgaImageColorData.tgaColorMap = NULL;
		}
		if( m_ptrStruInfo->tgaImageColorData.tgaImageData )
		{
			free(m_ptrStruInfo->tgaImageColorData.tgaImageData);
			m_ptrStruInfo->tgaImageColorData.tgaImageData = NULL;
		}
		free(m_ptrStruInfo);
		m_ptrStruInfo = NULL;
	}
	if( m_ptrFileFooter )
	{
		free(m_ptrFileFooter);
		m_ptrFileFooter = NULL;
	}
	if( m_ptrImageData )
	{
		free(m_ptrImageData);
		m_ptrImageData = NULL;
	}
}

bool SrImageTga::readFileFooter(FILE* flPtrFile)
{
	if( fseek(flPtrFile,-26L,SEEK_END)==-1 )
	{
		m_inError = IMAGE_SEEK_FAIL;
		return false;
	}
	if( fread(m_ptrFileFooter,sizeof(TGAFILEFOOTER),1,flPtrFile)!=1 )
	{
		m_inError = IMAGE_READ_FAIL;
		return false;
	}
	char signature[] = "TRUEVISION-XFILE";
	if( memcmp(signature,m_ptrFileFooter->tgaSigniture,16)!=0 )
	{
		free(m_ptrFileFooter);
		m_ptrFileFooter = NULL;
	}
	return true;
}

bool SrImageTga::readFileDevExt(FILE * flPtrFile)
{
	int devLength = 0, extLength = 0;
	BYTE* ptrDev = NULL;
	BYTE* ptrExt = NULL;
	if( m_ptrFileFooter->tgaExtOffset )
	{//读取拓展区域数据
		extLength = m_ptrStruInfo->tgaFileSize - m_ptrFileFooter->tgaExtOffset - 26;
		if( fseek(flPtrFile,m_ptrFileFooter->tgaExtOffset,SEEK_SET)==-1 )
		{
			m_inError = IMAGE_SEEK_FAIL;
			return false;
		}
		ptrExt = (BYTE*)malloc(extLength);
		if( fread(ptrExt,1,extLength,flPtrFile)!=extLength )
		{
			m_inError = IMAGE_READ_FAIL;
			free(ptrExt);
			return false;
		}
	}
	if( m_ptrFileFooter->tgaDevOffset )
	{//读取开发者区域
		devLength = m_ptrFileFooter->tgaExtOffset - m_ptrFileFooter->tgaDevOffset;
		if( fseek(flPtrFile,m_ptrFileFooter->tgaDevOffset,SEEK_SET)==-1 )
		{
			m_inError = IMAGE_SEEK_FAIL;
			free(ptrDev);
			return false;
		}
		ptrDev = (BYTE*)malloc(devLength);
		if( fread(ptrDev,1,devLength,flPtrFile)!=devLength )
		{
			m_inError = IMAGE_READ_FAIL;
			free(ptrDev);
			free(ptrExt);
			return false;
		}
	}

	m_ptrStruInfo->tgaPtrDevData	=	ptrDev;
	m_ptrStruInfo->tgaPtrExtData	=	ptrExt;
	return true;
}

bool SrImageTga::readUncompressed(FILE*flPtrFile ,BYTE* btPtrOutData)
{
	BYTE* ptrFileData;
	int pixCount = m_ptrFileHeader->tgaWidth * m_ptrFileHeader->tgaHeight;
	int dataSize = 0;
	if( m_ptrFileHeader->tgaPixelDepth==8 )
	{
		dataSize = pixCount;
		ptrFileData = (BYTE*)malloc(dataSize);
		if( !ptrFileData )
		{
			m_inError = IMAGE_NO_MEMORY;
			return false;
		}
		if( fread(ptrFileData,1,dataSize,flPtrFile) != dataSize )
		{
			m_inError = IMAGE_READ_FAIL;
			free(ptrFileData);
			return false;
		}
		BYTE*  tmpPtrData = ptrFileData;
		BYTE*  tmpPtrDataEnd = tmpPtrData + dataSize;
		while( tmpPtrData<tmpPtrDataEnd )
		{
			*(btPtrOutData + 2)	=  *tmpPtrData;
			*(btPtrOutData + 1) =  *tmpPtrData;
			*(btPtrOutData)		=  *tmpPtrData;

			btPtrOutData += 3;
			tmpPtrData += 1;
		}
		free(ptrFileData);
	}
	else if( m_ptrFileHeader->tgaPixelDepth==15 ||  m_ptrFileHeader->tgaPixelDepth==16 )
	{
		dataSize = pixCount<<1;
		ptrFileData = (BYTE*)malloc(dataSize);
		if( !ptrFileData )
		{
			m_inError = IMAGE_NO_MEMORY;
			return false;
		}
		if( fread(ptrFileData,1,dataSize,flPtrFile) != dataSize )
		{
			m_inError = IMAGE_READ_FAIL;
			free(ptrFileData);
			return false;
		}
		BYTE*  tmpPtrData = ptrFileData;
		BYTE*  tmpPtrDataEnd = tmpPtrData + dataSize;
		while( tmpPtrData<tmpPtrDataEnd )
		{
			*(btPtrOutData + 2)	=  (*tmpPtrData & 0x1F);
			*(btPtrOutData + 1) =  ((*(tmpPtrData+1)<<3)&0x1C) | ((*tmpPtrData>>5)&0x07);
			*(btPtrOutData)		=  (*(tmpPtrData+1)>>2)&0x1F;

			*(btPtrOutData)		= (*(btPtrOutData)<<3 ) | (*(btPtrOutData)>>2 );
			*(btPtrOutData + 1) = (*(btPtrOutData + 1)<<3 ) | (*(btPtrOutData + 1)>>2 );
			*(btPtrOutData + 2) = (*(btPtrOutData + 2)<<3 ) | (*(btPtrOutData + 2)>>2 );

			btPtrOutData += 3;
			tmpPtrData += 2;
		}
		free(ptrFileData);
	}
	else if( m_ptrFileHeader->tgaPixelDepth==24 )
	{
		dataSize = pixCount*3;
		ptrFileData = (BYTE*)malloc(dataSize);
		if( !ptrFileData )
		{
			m_inError = IMAGE_NO_MEMORY;
			return false;
		}
		if( fread(ptrFileData,1,dataSize,flPtrFile) != dataSize )
		{
			m_inError = IMAGE_READ_FAIL;
			free(ptrFileData);
			return false;
		}
		BYTE*  tmpPtrData = ptrFileData;
		BYTE*  tmpPtrDataEnd = tmpPtrData + dataSize;
		while( tmpPtrData<tmpPtrDataEnd )
		{
			*(btPtrOutData + 2)	=  *(tmpPtrData);
			*(btPtrOutData + 1) =  *(tmpPtrData + 1);
			*(btPtrOutData)		=  *(tmpPtrData + 2);

			tmpPtrData += 3;
			btPtrOutData += 3;
		}
		free(ptrFileData);
	}
	else
	{
		dataSize = pixCount<<2;
		ptrFileData = (BYTE*)malloc(dataSize);
		if( !ptrFileData )
		{
			m_inError = IMAGE_NO_MEMORY;
			return false;
		}
		if( fread(ptrFileData,1,dataSize,flPtrFile) != dataSize )
		{
			m_inError = IMAGE_READ_FAIL;
			free(ptrFileData);
			return false;
		}
		BYTE*  tmpPtrData = ptrFileData;
		BYTE*  tmpPtrDataEnd = tmpPtrData + dataSize;
		while( tmpPtrData<tmpPtrDataEnd )
		{
			*(btPtrOutData + 2)	=  *(tmpPtrData);
			*(btPtrOutData + 1) =  *(tmpPtrData + 1);
			*(btPtrOutData)		=  *(tmpPtrData + 2);
			*(btPtrOutData + 3) =  *(tmpPtrData + 3);
			tmpPtrData	 += 4;
			btPtrOutData += 4;
		}
		free(ptrFileData);
	}
	return true;
}

bool SrImageTga::readCompressed(FILE*flPtrFile ,BYTE* btPtrOutData)
{
	int countPixel = m_ptrFileHeader->tgaWidth * m_ptrFileHeader->tgaHeight;
	int i , curPixel = 0;
	BYTE chunkHeader = 0;
	int bytesPerPixel = (m_ptrFileHeader->tgaPixelDepth+7)>>3;
	BYTE* ptrColorBuffer = (BYTE*)malloc(bytesPerPixel*128);
	bool isRLE = false;
	do
	{
		chunkHeader = 0;
		if( fread(&chunkHeader,1,1,flPtrFile)!=1 )
		{
			free(ptrColorBuffer);
			m_inError = IMAGE_READ_FAIL;
			return false;
		}
		if( chunkHeader<128 )
		{//原始包
			chunkHeader ++;
			if( fread(ptrColorBuffer,chunkHeader*bytesPerPixel,1,flPtrFile)!=1 )
			{
				free(ptrColorBuffer);
				m_inError = IMAGE_READ_FAIL;
				return false;
			}
			isRLE = false;
		}
		else		
		{//行程包
			chunkHeader -= 127;
			if( fread(ptrColorBuffer,bytesPerPixel,1,flPtrFile)!= 1 )
			{
				free(ptrColorBuffer);
				m_inError = IMAGE_READ_FAIL;
				return false;
			}
			isRLE = true;
		}
		curPixel += chunkHeader;
		if( curPixel>countPixel )
		{
			free(ptrColorBuffer);
			m_inError = IMAGE_UNKNOWN_FORMAT;
			return false;
		}

		BYTE* ptrTmpBuffer = ptrColorBuffer;
		if( m_ptrFileHeader->tgaPixelDepth==8 )
		{
			for( i=0 ; i<chunkHeader ; i++ )
			{
				*(btPtrOutData + 2)	=  *(ptrTmpBuffer);
				*(btPtrOutData + 1) =  *(ptrTmpBuffer);
				*(btPtrOutData)		=  *(ptrTmpBuffer);
				btPtrOutData += 3;
				if( !isRLE ) ptrTmpBuffer+=1;
			}
		}
		else if( m_ptrFileHeader->tgaPixelDepth==15 || m_ptrFileHeader->tgaPixelDepth==16)
		{
			for( i=0 ; i<chunkHeader ; i++ )
			{
				*(btPtrOutData + 2)	=  (*(ptrTmpBuffer) & 0x1F);
				*(btPtrOutData + 1) =  ((*(ptrTmpBuffer+1)<<3)&0x1C) | ((*ptrTmpBuffer>>5)&0x07);
				*(btPtrOutData)		=  (*(ptrTmpBuffer+1)>>2)&0x1F;

				*(btPtrOutData)		= (*(btPtrOutData)<<3 )		| (*(btPtrOutData)>>2 );
				*(btPtrOutData + 1) = (*(btPtrOutData + 1)<<3 ) | (*(btPtrOutData + 1)>>2 );
				*(btPtrOutData + 2) = (*(btPtrOutData + 2)<<3 ) | (*(btPtrOutData + 2)>>2 );
				btPtrOutData += 3;
				if( !isRLE ) ptrTmpBuffer+=2;
			}
		}
		else if( m_ptrFileHeader->tgaPixelDepth==24 )
		{
			for( i=0 ; i<chunkHeader ; i++ )
			{
				*(btPtrOutData + 2)	=  *(ptrTmpBuffer);
				*(btPtrOutData + 1) =  *(ptrTmpBuffer + 1);
				*(btPtrOutData) =  *(ptrTmpBuffer + 2);
				btPtrOutData += 3;
				if( !isRLE ) ptrTmpBuffer += 3;
			}
		}
		else if( m_ptrFileHeader->tgaPixelDepth==32 )
		{
			for( i=0 ; i<chunkHeader ; i++ )
			{
				*(btPtrOutData + 2)	=  *(ptrTmpBuffer);
				*(btPtrOutData + 1)	=  *(ptrTmpBuffer + 1);
				*(btPtrOutData)		=  *(ptrTmpBuffer + 2);
				*(btPtrOutData + 3) =  *(ptrTmpBuffer + 3);
				btPtrOutData += 4;
				if( !isRLE ) ptrTmpBuffer += 4;
			}
		}
	} while (curPixel<countPixel);

	free(ptrColorBuffer);

	return true;
}

bool SrImageTga::readCompressedMap(FILE*flPtrFile ,BYTE* btPtrColorMap,BYTE* btPtrOutData)
{
	int countPixel = m_ptrFileHeader->tgaWidth * m_ptrFileHeader->tgaHeight;
	int i=0 , curPixel=0 ,chunkHeader = 0;
	BYTE* ptrColorBuffer = (BYTE*)malloc(128);
	int bytesPerPixel = (m_ptrFileHeader->tgaColorMapEnSize+7)>>3;
	BYTE* ptrBase = btPtrColorMap + m_ptrFileHeader->tgaFirstEnIndex * bytesPerPixel;
	bool isRLE = false;

	do 
	{
		if( fread(&chunkHeader,1,1,flPtrFile)!=1 )
		{
			free(ptrColorBuffer);
			m_inError = IMAGE_READ_FAIL;
			return false;
		}
		if( chunkHeader<128 )
		{
			chunkHeader ++;
			isRLE = false;
			if( fread(ptrColorBuffer,chunkHeader,1,flPtrFile)!=1 )
			{
				free(ptrColorBuffer);
				m_inError = IMAGE_READ_FAIL;
				return false;
			}
		}
		else
		{
			chunkHeader -= 127;
			isRLE = true;
			if( fread(ptrColorBuffer,1,1,flPtrFile)!=1 )
			{
				free(ptrColorBuffer);
				m_inError = IMAGE_READ_FAIL;
				return false;
			}
		}
		curPixel += chunkHeader;
		if( curPixel > countPixel )
		{
			free(ptrColorBuffer);
			m_inError = IMAGE_UNKNOWN_FORMAT;;
			return false;
		}


		BYTE* ptrTmpBuffer = ptrColorBuffer;
		if( m_ptrFileHeader->tgaColorMapEnSize==15 || m_ptrFileHeader->tgaColorMapEnSize==16)
		{
			for( i=0 ; i<chunkHeader ; i++ )
			{
				*(btPtrOutData + 2)	=  (*(ptrBase + (*ptrTmpBuffer<<1))&0x1F);
				*(btPtrOutData + 1) =  ((*(ptrBase + (*ptrTmpBuffer<<1) + 1)<<3)&0x1C) | ((*(ptrBase + (*ptrTmpBuffer<<1))>>5)&0x07);
				*(btPtrOutData)		=  (*(ptrBase + (*ptrTmpBuffer<<1) + 1)>>2)&0x1F;

				*(btPtrOutData)		= (*(btPtrOutData)<<3 ) | (*(btPtrOutData)>>2 );
				*(btPtrOutData + 1) = (*(btPtrOutData + 1)<<3 ) | (*(btPtrOutData  + 1)>>2 );
				*(btPtrOutData + 2) = (*(btPtrOutData + 2)<<3 ) | (*(btPtrOutData  + 2)>>2 );

				btPtrOutData += 3;
				if( !isRLE ) ptrTmpBuffer++;
			}
		}
		else if( m_ptrFileHeader->tgaColorMapEnSize==24 )
		{
			for( i=0 ; i<chunkHeader ; i++ )
			{
				*(btPtrOutData + 2)	=  *(ptrBase + (*ptrTmpBuffer<<1));
				*(btPtrOutData + 1) =  *(ptrBase + (*ptrTmpBuffer<<1) + 1);
				*(btPtrOutData)		=  *(ptrBase + (*ptrTmpBuffer<<1) + 2);
				btPtrOutData += 3;
				if( !isRLE ) ptrTmpBuffer++;
			}
		}
		else if( m_ptrFileHeader->tgaColorMapEnSize==32 )
		{
			for( i=0 ; i<chunkHeader ; i++ )
			{
				*(btPtrOutData + 2)	=  *(ptrBase + (*ptrTmpBuffer<<1));
				*(btPtrOutData + 1) =  *(ptrBase + (*ptrTmpBuffer<<1) + 1);
				*(btPtrOutData)		=  *(ptrBase + (*ptrTmpBuffer<<1) + 2);
				*(btPtrOutData + 3) =  *(ptrBase + (*ptrTmpBuffer<<1) + 3);
				btPtrOutData += 4;
				if( !isRLE ) ptrTmpBuffer++;
			}
		}
	} while (curPixel<countPixel);

	free(ptrColorBuffer);

	return true;
}

bool SrImageTga::readUncompressedMap(FILE*flPtrFile ,BYTE* btPtrColorMap,BYTE* btPtrOutData)
{
	BYTE* ptrFileData;
	int pixelCount = m_ptrFileHeader->tgaWidth * m_ptrFileHeader->tgaHeight;
	int base = m_ptrFileHeader->tgaFirstEnIndex;
	int bytesPerPixel = (m_ptrFileHeader->tgaColorMapEnSize + 7)>>3;
	BYTE* ptrBase = btPtrColorMap + m_ptrFileHeader->tgaFirstEnIndex * bytesPerPixel;
	ptrFileData = (BYTE*)malloc(pixelCount);
	if( !ptrFileData )
	{
		m_inError = IMAGE_NO_MEMORY;
		return false;
	}
	if( fread(ptrFileData,1,pixelCount,flPtrFile) != pixelCount )
	{
		m_inError = IMAGE_READ_FAIL;
		free(ptrFileData);
		return false;
	}
	BYTE*  tmpPtrData = ptrFileData;
	BYTE*  tmpPtrOutData = btPtrOutData;
	BYTE*  ptrFileDataEnd = tmpPtrData + pixelCount;

	if( m_ptrFileHeader->tgaColorMapEnSize==15 ||  m_ptrFileHeader->tgaColorMapEnSize==16 )
	{
		while( tmpPtrData<ptrFileDataEnd )
		{
			*(tmpPtrOutData+2)	=  (*(ptrBase+*tmpPtrData) & 0x1F);
			*(tmpPtrOutData+1)	=  ((*(ptrBase+(*tmpPtrData<<1)+1)<<3)&0x1C) | ((*(ptrBase+(*tmpPtrData<<1))>>5)&0x07);
			*(tmpPtrOutData)	=  (*(ptrBase+(*tmpPtrData<<1)+1)>>2)&0x1F;

			*(tmpPtrOutData)		= (*(tmpPtrOutData)<<3 ) | (*(tmpPtrOutData)>>2 );
			*(tmpPtrOutData + 1)	= (*(tmpPtrOutData + 1)<<3 ) | (*(tmpPtrOutData + 1)>>2 );
			*(tmpPtrOutData + 2)	= (*(tmpPtrOutData + 2)<<3 ) | (*(tmpPtrOutData + 2)>>2 );

			tmpPtrOutData += 3;
			tmpPtrData += 1;
		}
	}
	else if( m_ptrFileHeader->tgaColorMapEnSize==24 )
	{
		while( tmpPtrData<ptrFileDataEnd )
		{
			*(tmpPtrOutData + 2)	=	*(ptrBase + *tmpPtrData*3);
			*(tmpPtrOutData + 1)	=	*(ptrBase + 1 + *tmpPtrData*3);
			*(tmpPtrOutData)		=	*(ptrBase + 2 + *tmpPtrData*3);
			tmpPtrOutData += 3;
			tmpPtrData += 1;
		}
	}
	else
	{
		while( tmpPtrData<ptrFileDataEnd )
		{
			*(tmpPtrOutData + 2)=  *(ptrBase + (*tmpPtrData<<2));
			*(tmpPtrOutData + 1)= *(ptrBase + (*tmpPtrData<<2) + 1);
			*(tmpPtrOutData)	= *(ptrBase + (*tmpPtrData<<2) + 2);
			*(tmpPtrOutData + 3) = *(ptrBase + (*tmpPtrData<<2) + 3);
			tmpPtrData += 1;
			tmpPtrOutData += 4;
		}
	}
	free(ptrFileData);

	return true;
}

bool SrImageTga::checkFileFormat()const
{
	//头文件中的宽和高必需大于0
	if( m_ptrFileHeader->tgaHeight<=0 || m_ptrFileHeader->tgaWidth<=0 )
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
	//只支持位深度是8,15,16,24,32的情况
	if( !(m_ptrFileHeader->tgaPixelDepth==8) &&
		!(m_ptrFileHeader->tgaPixelDepth==15)&&
		!(m_ptrFileHeader->tgaPixelDepth==16)&&
		!(m_ptrFileHeader->tgaPixelDepth==24)&&
		!(m_ptrFileHeader->tgaPixelDepth==32) )
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
	if( m_ptrFileHeader->tgaPixelDepth==8 )
	{
		//如果位深度是8，不采用颜色表，则只能表示黑白图像类型
		if( !m_ptrFileHeader->tgaColorMapType&&(m_ptrFileHeader->tgaImageType&0x03)!=0x03 )
		{
			m_inError = IMAGE_UNKNOWN_FORMAT;
			return false;
		}
	}
	else if( m_ptrFileHeader->tgaColorMapType)
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
	//判断图像类型与颜色表是否匹配
	if( !(m_ptrFileHeader->tgaImageType==TGA_UN_COLOR_MAP&&m_ptrFileHeader->tgaColorMapType)&&
		!(m_ptrFileHeader->tgaImageType==TGA_UN_TRUE_COLOR&&!m_ptrFileHeader->tgaColorMapType)&&
		!(m_ptrFileHeader->tgaImageType==TGA_UN_BLACK_WHITE&&!m_ptrFileHeader->tgaColorMapType)&&
		!(m_ptrFileHeader->tgaImageType==TGA_RLE_COLOR_MAP&&m_ptrFileHeader->tgaColorMapType)&&
		!(m_ptrFileHeader->tgaImageType==TGA_RLE_TRUE_COLOR&&!m_ptrFileHeader->tgaColorMapType)&&
		!(m_ptrFileHeader->tgaImageType==TGA_RLE_BLACK_WHITE&&!m_ptrFileHeader->tgaColorMapType) )
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
	//如果采用颜色表，则每个表项占的位数
	if( m_ptrFileHeader->tgaColorMapType )
	{
		if( m_ptrFileHeader->tgaColorMapEnSize != 15 &&
			m_ptrFileHeader->tgaColorMapEnSize != 16 &&
			m_ptrFileHeader->tgaColorMapEnSize != 24 &&
			m_ptrFileHeader->tgaColorMapEnSize != 32 )
		{
			m_inError = IMAGE_UNKNOWN_FORMAT;
			return false;
		}
	}
#if SR_IMAGE_TGA_CHECK_ORIGIN
	if( m_ptrFileHeader->tgaXOrigin!=0 || m_ptrFileHeader->tgaYOrigin!=0)
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
#endif

#if SR_IMAGE_TGA_CHECK_IMAGEDESC
	if( m_ptrFileHeader->tgaImageDesc )
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
#endif
	return true;
}



bool SrImageTga::readFileImageData(FILE* flPtrFile, BYTE*& btPtrOutData, int& inRGBType )
{
	int inRGB;
	if( fseek(flPtrFile,sizeof(TGAFILEHEADER),SEEK_SET)==-1 )
	{
		m_inError = IMAGE_SEEK_FAIL;
		return false;
	}
	BYTE* rgbIdInfo = NULL;
	BYTE* ptrOutData = NULL;
	//图像信息字段长度不为0，则需要读取图像信息字段
	if( m_ptrFileHeader->tgaIdLength )
	{
		rgbIdInfo = (BYTE*)malloc(m_ptrFileHeader->tgaIdLength);
		if( !rgbIdInfo )
		{
			m_inError = IMAGE_NO_MEMORY;
			return false;
		}
		if( fread(rgbIdInfo,m_ptrFileHeader->tgaIdLength,1,flPtrFile)!=1 )
		{
			m_inError = IMAGE_READ_FAIL;
			free(rgbIdInfo);
			return false;
		}
	}
	if( !m_ptrFileHeader->tgaColorMapType )
	{//如果不存在颜色表的情况下解析TGA文件
		ptrOutData = NULL;
		//分配存储RGB数据的内存空间
		if( m_ptrFileHeader->tgaPixelDepth==32 )
		{
			ptrOutData = (BYTE*)malloc(4*m_ptrFileHeader->tgaWidth*m_ptrFileHeader->tgaHeight);
			inRGB = IMAGE_RGBA;
		}
		else
		{
			ptrOutData = (BYTE*)malloc(3*m_ptrFileHeader->tgaWidth*m_ptrFileHeader->tgaHeight);
			inRGB = IMAGE_RGB;
		}
		if( !ptrOutData )
		{
			m_inError = IMAGE_NO_MEMORY;
			if( rgbIdInfo )
				free(rgbIdInfo);
			return false;
		}
		if( m_ptrFileHeader->tgaImageType&0x08 )
		{
			//解析RLE压缩的TGA文件
			if( !readCompressed(flPtrFile,ptrOutData) )
			{
				free(ptrOutData);
				if( rgbIdInfo )
					free(rgbIdInfo);
				return false;
			}
		}
		else
		{
			//解析未经过RLE压缩的TGA文件
			if( !readUncompressed(flPtrFile,ptrOutData) )
			{
				free(ptrOutData);
				if( rgbIdInfo )
					free(rgbIdInfo);
				return false;
			}
		}
	}
	else
	{//如果存在颜色表的情况下解析TGA文件
		int bytesPerPixel = (m_ptrFileHeader->tgaColorMapLen + 7)>>3;
		int colorMapSize = m_ptrFileHeader->tgaColorMapEnSize*bytesPerPixel;
		//分配颜色表的内存空间
		BYTE* ptrColorMap = (BYTE*)malloc(colorMapSize);
		if( !ptrColorMap )
		{
			m_inError = IMAGE_NO_MEMORY;
			if( rgbIdInfo )
				free(rgbIdInfo);
			return false;
		}
		if( fread(ptrColorMap,1,colorMapSize,flPtrFile)!=colorMapSize )
		{
			free(ptrColorMap);
			if( rgbIdInfo )
				free(rgbIdInfo);
			m_inError = IMAGE_READ_FAIL;
			return false;
		}
		//分配存储RGB数据的内存空间
		ptrOutData = NULL;
		if( m_ptrFileHeader->tgaColorMapEnSize==32 )
		{
			ptrOutData = (BYTE*)malloc(4*m_ptrFileHeader->tgaWidth*m_ptrFileHeader->tgaHeight);
			inRGB = IMAGE_RGBA;
		}
		else
		{
			ptrOutData = (BYTE*)malloc(3*m_ptrFileHeader->tgaWidth*m_ptrFileHeader->tgaHeight);
			inRGB = IMAGE_RGB;
		}
		if( !ptrOutData )
		{
			m_inError = IMAGE_NO_MEMORY;
			free(ptrColorMap);
			if( rgbIdInfo )
				free(rgbIdInfo);
			return false;
		}
		//解析RLE压缩的TGA文件
		if( m_ptrFileHeader->tgaImageType&0x08 )
		{
			if( !readCompressedMap(flPtrFile,ptrColorMap,ptrOutData) )
			{
				free(ptrOutData);
				free(ptrColorMap);
				if( rgbIdInfo )
					free(rgbIdInfo);
				return false;
			}
		}
		else
		{
			//解析未经过RLE压缩的TGA文件
			if( !readUncompressedMap(flPtrFile,ptrColorMap,ptrOutData) )
			{
				free(ptrOutData);
				free(ptrColorMap);
				if( rgbIdInfo )
					free(rgbIdInfo);
				return false;
			}
		}
		free(ptrColorMap);
	}


	//解析TGA文件正确，更新图像信息字段
	m_ptrStruInfo->tgaImageColorData.tgaImageId = rgbIdInfo;
	//函数的返回值
	btPtrOutData = ptrOutData;
	inRGBType = inRGB;
	return true;
}

bool SrImageTga::readFile(const char* chPtrFileName,BYTE*& btPtrImageData,int& lgPixelCount,int& inRGBType)
{
	if( m_inIsReadOnly!=IMAGE_READ_ONLY )
	{
		m_inError = IMAGE_OBJECT_WRITE_ONLY;
		return false;
	}
	FILE* ptrHandle = fopen(chPtrFileName,"rb");
	if( !ptrHandle )	
	{
		m_inError = IMAGE_OPEN_FAIL;
		return false;
	}
	//分配TGAFILEHEADER，TGAFILEFOOTER，TGASTRUCTUREDATA三个结构体的内存空间
	if( !mallocMemory() )
	{
		fclose(ptrHandle);
		return false;
	}
	//读取文件头
	if( fread(m_ptrFileHeader,sizeof(TGAFILEHEADER),1,ptrHandle)!=1 )
	{
		m_inError = IMAGE_READ_FAIL;
		deallocMemory();
		fclose(ptrHandle);
		return false;
	}
	//判断文件格式是否符合解析的要求
	if( !checkFileFormat() )
	{
		deallocMemory();
		fclose(ptrHandle);
		return false;
	}
	BYTE* imageData = NULL;
	int rgbType = 0;
	//读取图像、颜色表数据
	if( !readFileImageData(ptrHandle,imageData,rgbType) )
	{
		deallocMemory();
		fclose(ptrHandle);
		return false;
	}
	else
	{
		m_ptrImageData = imageData;
	}
	//读取当前的文件指针位置
	long imageDataOffset = ftell(ptrHandle);
	//移动文件指针至文件结尾，记录文件大小
	if( fseek(ptrHandle,0L,SEEK_END)==-1 )
	{
		m_inError = IMAGE_SEEK_FAIL;
		return false;
	}
	m_ptrStruInfo->tgaFileSize = ftell(ptrHandle);
	//读取文件注脚，如果成功，说明TGA文件遵守新的TGA文件格式；
	//如果失败，说明TGA文件属于旧的TGA文件格式，则释放注脚结构体的数据,
	//m_ptrFileFooter的值为NULL
	if(m_ptrStruInfo->tgaFileSize>=imageDataOffset + 26 && !readFileFooter(ptrHandle) )
	{
		deallocMemory();
		fclose(ptrHandle);
		return false;
	}
	if( m_ptrFileFooter )
	{//如果是新的TGA格式，则可能存在开发者区域和拓展区域，读取这两块区域
		if( !readFileDevExt(ptrHandle) )
		{
			deallocMemory();
			fclose(ptrHandle);
			return false;
		}
	}
	fclose(ptrHandle);

	inRGBType = rgbType;
	lgPixelCount = m_ptrFileHeader->tgaWidth*m_ptrFileHeader->tgaHeight;
	btPtrImageData = imageData;
	return true;
}


bool SrImageTga::writeFile(const char* chPtrImageFile) const
{
	if( m_inIsReadOnly!=IMAGE_WRITE_ONLY )
	{
		m_inError = IMAGE_OBJECT_READ_ONLY;
		return false;
	}
	if( !m_ptrImageData )
	{
		m_inError = IMAGE_UNKNOWN;
		return false;
	}
	FILE* ptrHandle = fopen(chPtrImageFile,"wb");
	if( !ptrHandle )
	{
		m_inError = IMAGE_OPEN_FAIL;
		return false;
	}

	//写文件头
	if( fwrite(m_ptrFileHeader,sizeof(TGAFILEHEADER),1,ptrHandle)!=1 )
	{
		m_inError = IMAGE_WRITE_FAIL;
		return false;
	}
	if( m_ptrFileHeader->tgaIdLength )
	{
		assert(m_ptrStruInfo->tgaImageColorData.tgaImageId);
		if( fwrite(m_ptrStruInfo->tgaImageColorData.tgaImageId,1,m_ptrFileHeader->tgaIdLength,ptrHandle)!=m_ptrFileHeader->tgaIdLength )
		{
			m_inError = IMAGE_WRITE_FAIL;
			return false;
		}
	}
	//写图像数据
	int pixelCount = m_ptrFileHeader->tgaHeight * m_ptrFileHeader->tgaWidth;
	if( fwrite(m_ptrImageData,1,pixelCount*3,ptrHandle)!=3*pixelCount )
	{
		m_inError = IMAGE_WRITE_FAIL;
		return false;
	}
	//写文件注脚
	if( fwrite(m_ptrFileFooter,sizeof(TGAFILEFOOTER),1,ptrHandle)!=1 )
	{
		m_inError = IMAGE_WRITE_FAIL;
		return false;
	}

	fclose(ptrHandle);
	return true;
}


bool SrImageTga::loadImageData(unsigned char* ucPtrRgbData ,unsigned short inWidth,unsigned short inHeight)
{
	if( m_inIsReadOnly!=IMAGE_WRITE_ONLY )
	{
		m_inError = IMAGE_OBJECT_READ_ONLY;
		return false;
	}
	if( !ucPtrRgbData )
	{
		m_inError = IMAGE_UNKNOWN;
		return false;
	}
	//清空TGA对象中原始的图像数据
	deallocMemory();
	//分配文件头数据，文件结构数据，文件注脚数据内存
	if( !mallocMemory())
	{
		return false;
	}
	m_ptrFileHeader->tgaImageType = 0x02;
	m_ptrFileHeader->tgaWidth  = inWidth;
	m_ptrFileHeader->tgaHeight = inHeight;
	m_ptrFileHeader->tgaPixelDepth = 24;
	//分配图像数据内存
	int pixelCount = 3*inWidth*inHeight;
	m_ptrImageData = (BYTE*)malloc(pixelCount);
	if( !m_ptrImageData )
	{
		m_inError = IMAGE_NO_MEMORY;
		free(m_ptrFileHeader);
		m_ptrFileHeader = NULL;
		return false;
	}
	//把数据复制进TGA文件对象中
	int i;
	for( i=0 ; i<pixelCount ; i++ )
		m_ptrImageData[i] = ucPtrRgbData[i];

	//初始化注脚信息
	char signature[] = "TRUEVISION-XFILE";
	m_ptrFileFooter->tgaAscii='.';
	for( i=0 ; i<16 ; i++ )
		m_ptrFileFooter->tgaSigniture[i] = signature[i];

	//初始化图像信息字段
	char imageInfo[]="Copywrite by TwinkingStar";
	m_ptrFileHeader->tgaIdLength = strlen(imageInfo);
	m_ptrStruInfo->tgaImageColorData.tgaImageId = (BYTE*)malloc(m_ptrFileHeader->tgaIdLength);
	for( i=0 ; i<m_ptrFileHeader->tgaIdLength ; i++ )
	{
		m_ptrStruInfo->tgaImageColorData.tgaImageId[i] = imageInfo[i];
	}
	return true;
}
