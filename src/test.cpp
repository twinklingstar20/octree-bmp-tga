/************************************************************************	
\link	www.twinklingstar.cn
\author Twinkling Star
\date	2013/11/21
****************************************************************************/

#include <stdio.h>
#include <malloc.h>
#include <algorithm>
#include "SrImageBmp.h"
#include "SrImageTga.h"
#include "SrColorQuant.h"



int main( )
{
	//要读取的BMP文件名
	char input[256] = "LAND.BMP";
	SrImageBmp bmp(IMAGE_READ_ONLY);
	unsigned char* buffer;
	int pixelCount, test;
	if( !bmp.readFile(input,buffer,pixelCount,test) )
	{
		printf("Error!%d\n",bmp.getErrorId());
		return 0;
	}

	SrImageTga tga(IMAGE_WRITE_ONLY);
	if( !tga.loadImageData(bmp.getImageData(),bmp.getWidth(),bmp.getHeight()) )
	{
		printf("Load Error %d\n",tga.getErrorId());
	}
	else if( !tga.writeFile("testTGAWrite.tga"))
	{
		printf("Write Error %d\n",tga.getErrorId());
	}

	//读TGA文件
	SrImageTga readTga(IMAGE_READ_ONLY);
	if( !readTga.readFile("testTGAWrite.tga",buffer,pixelCount,test) )
	{
		printf("Read tga file testTGAWrite.tga Error! %d\n",readTga.getErrorId());
	}

	//写BMP文件
	int i;
	int bitCount[]={1,4,8,16,24};
	char fileName[100];
	SrImageBmp writeFile(IMAGE_WRITE_ONLY);
	for( i=0 ; i<5 ; i++ )
	{
		sprintf(fileName,"test%d.bmp",bitCount[i]);
		if( writeFile.loadImageData(bmp.getImageData(),bmp.getWidth(),bmp.getHeight(),bitCount[i]))
		{
			if( !writeFile.writeFile(fileName) )
				printf("Write test%d.bmp Error!\n",bitCount[i]);
			else
				printf("Write test%d.bmp Success!\n",bitCount[i]);
		}
		else
		{
			printf("Load test%d.bmp Error!\n",bitCount[i]);
		}
	}

	return 0;
}
