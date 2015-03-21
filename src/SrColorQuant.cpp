/************************************************************************		
\link	www.twinklingstar.cn
\author Twinkling Star
\date	2013/11/21
****************************************************************************/

#include "SrColorQuant.h"

SrColorQuant::SrColorQuant( )
{
	m_inMaxPixelCount	= 0;
	m_inLeafCount		= 0;
	m_ptrTree			= NULL;
	int i;
	for( i=0 ; i<9 ; i++ )
		m_ptrReducibleNodes[i] = NULL;
}

SrColorQuant::~SrColorQuant()
{
	empty();
}

int SrColorQuant::buildOctree( unsigned char* btPtrRgbData,int inPixelCount,int nMaxColors)
{
	if( nMaxColors>256 || nMaxColors<=0 )
		return false;
	if( m_ptrTree )
	{
		empty();
	}
	OctreeNode* tree = NULL;
	m_inLeafCount		=	0;
	m_inMaxPixelCount	=	0;
	unsigned char* ptrRgbDataEnd = btPtrRgbData + inPixelCount*3;
	while( btPtrRgbData<ptrRgbDataEnd )
	{
		//往八叉树中加入一个节点
		if( !addColor(tree,*btPtrRgbData,*(btPtrRgbData+1),*(btPtrRgbData+2),0) )
		{
			empty();
			return 0;
		}
		//如果叶点数超过了最大允许的颜色数nMaxColors，需要裁剪至小于nMaxColors为止
		while (m_inLeafCount > nMaxColors)
			reduceTree();
		btPtrRgbData+=3;
	}

	int inIndexNum = 0;
	setColorIndex(tree,inIndexNum);
	m_ptrTree = tree;
	return inIndexNum;
}

unsigned char  SrColorQuant::indexOctree(unsigned char byRed,unsigned char byGreen,unsigned char byBlue)const
{
	int nLevel = 0 , shift;
	int nIndex;
	static unsigned char mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	OctreeNode * ptrTree = m_ptrTree;
	while(!ptrTree->blIsLeaf)
	{
		shift = 7 - nLevel;
		nIndex = (((byRed & mask[nLevel]) >> shift) << 2) |
			(((byGreen & mask[nLevel]) >> shift) << 1) |
			((byBlue & mask[nLevel]) >> shift);
		ptrTree = ptrTree->ptrChild[nIndex];
		nLevel++;
	}
	return ptrTree->inColorIndex;
}
int SrColorQuant::getMaxPixelCount() const
{
	return m_inMaxPixelCount;
}

int	SrColorQuant::getLeafNodeCount()const
{
	return m_inLeafCount;
}

bool SrColorQuant::isEmpty()const
{
	return m_ptrTree==NULL;
}
void SrColorQuant::getColorPallette(unsigned char* ptrColorPal )const
{
	getColorPallette(m_ptrTree,ptrColorPal);
}


void SrColorQuant::getColorPallette(OctreeNode*ptrTree, unsigned char* ptrColorPal)const
{
	if (ptrTree->blIsLeaf) 
	{
		int base = ptrTree->inColorIndex * 3;
		*(ptrColorPal + base)		=	(unsigned char)ptrTree->uiRedSum;
		*(ptrColorPal + base + 1)	=	(unsigned char)ptrTree->uiGreenSum;
		*(ptrColorPal + base + 2)	=	(unsigned char)ptrTree->uiBlueSum;
	}
	else 
	{
		for (int i=0; i<8; i++) 
		{
			if (ptrTree->ptrChild[i] != NULL)
				getColorPallette (ptrTree->ptrChild[i],ptrColorPal);
		}
	}
}
SrColorQuant::OctreeNode* SrColorQuant::createNode (int inLevel)
{
	OctreeNode* pNode = (OctreeNode*)malloc(sizeof(OctreeNode));
	if( !pNode ) return NULL;
	memset(pNode,0,sizeof(OctreeNode));

	pNode->blIsLeaf = (inLevel == 8) ? true : false;
	if (pNode->blIsLeaf)
	{
		m_inLeafCount++;
	}
	else 
	{
		pNode->ptrNext = m_ptrReducibleNodes[inLevel];
		m_ptrReducibleNodes[inLevel] = pNode;
	}
	return pNode;
}

void SrColorQuant::reduceTree ()
{
	int i;
	OctreeNode* pNode;
	unsigned int nRedSum = 0, nGreenSum = 0 , nBlueSum = 0;
	int nChildren = 0;

	// Find the deepest level containing at least one reducible node
	for ( i=7 ; (i>0) && (m_ptrReducibleNodes[i] == NULL); i--);

	// Reduce the node most recently added to the list at level i
	pNode = m_ptrReducibleNodes[i];
	m_ptrReducibleNodes[i] = pNode->ptrNext;

	for (i=0; i<8; i++) 
	{
		if (pNode->ptrChild[i] != NULL) 
		{
			nRedSum				+= pNode->ptrChild[i]->uiRedSum;
			nGreenSum			+= pNode->ptrChild[i]->uiGreenSum;
			nBlueSum			+= pNode->ptrChild[i]->uiBlueSum;
			pNode->uiPixelCount += pNode->ptrChild[i]->uiPixelCount;

			free(pNode->ptrChild[i]);
			pNode->ptrChild[i] = NULL;
			nChildren++;
		}
	}

	pNode->blIsLeaf = true;
	pNode->uiRedSum	= nRedSum;
	pNode->uiGreenSum = nGreenSum;
	pNode->uiBlueSum = nBlueSum;

	//更新节点的最大像素数量。
	if(pNode->uiPixelCount > m_inMaxPixelCount)
		m_inMaxPixelCount = pNode->uiPixelCount;

	//更新叶节点数
	m_inLeafCount -= (nChildren - 1);
}

bool SrColorQuant::addColor(OctreeNode*& ptrTreeNode,unsigned char byRed,unsigned char byGreen,unsigned char byBlue,int inLevel)
{
	int nIndex, shift;
	static unsigned char mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

	// 结点不存在，创建一个新的结点
	if (ptrTreeNode== NULL)
		ptrTreeNode = createNode (inLevel);
	if( ptrTreeNode==NULL )
		return false;

	//如果是叶结点，更新颜色信息
	if (ptrTreeNode->blIsLeaf) 
	{
		ptrTreeNode->uiPixelCount++;
		ptrTreeNode->uiRedSum += byRed;
		ptrTreeNode->uiGreenSum += byGreen;
		ptrTreeNode->uiBlueSum += byBlue;

		if(ptrTreeNode->uiPixelCount > m_inMaxPixelCount)
			m_inMaxPixelCount = ptrTreeNode->uiPixelCount;
	}
	//如果不是叶结点，递归下一层
	else 
	{
		shift = 7 - inLevel;
		nIndex = (((byRed & mask[inLevel]) >> shift) << 2) |
			(((byGreen & mask[inLevel]) >> shift) << 1) |
			((byBlue & mask[inLevel]) >> shift);
		if( !addColor (ptrTreeNode->ptrChild[nIndex], byRed, byGreen,byBlue,inLevel + 1) )
			return false;
	}
	return true;
}

void SrColorQuant::setColorIndex(OctreeNode* ptrTree,int& inIndex)
{
	if (ptrTree->blIsLeaf) 
	{//这个节点所代表的颜色在调色板中的索引！
		ptrTree->inColorIndex	= (unsigned char) (inIndex);
		ptrTree->uiRedSum		= (unsigned char) ((ptrTree->uiRedSum) / (ptrTree->uiPixelCount));
		ptrTree->uiGreenSum		= (unsigned char) ((ptrTree->uiGreenSum) / (ptrTree->uiPixelCount));
		ptrTree->uiBlueSum		= (unsigned char) ((ptrTree->uiBlueSum) / (ptrTree->uiPixelCount));
		ptrTree->uiPixelCount = 1;
		inIndex++;
	}
	else 
	{
		for (int i=0; i<8; i++) 
		{
			if (ptrTree->ptrChild[i] != NULL)
				setColorIndex (ptrTree->ptrChild[i],inIndex);
		}
	}
}

void SrColorQuant::freeOctree(OctreeNode*& ocPtrTree)
{
	if( !ocPtrTree )
		return;
	int i;
	for( i=0 ; i<8 ; i++ )
	{
		if( ocPtrTree->ptrChild[i] )
			freeOctree(ocPtrTree->ptrChild[i]);
	}
	free(ocPtrTree);
	ocPtrTree = NULL;
}

void SrColorQuant::empty()
{
	freeOctree(m_ptrTree);
	m_inLeafCount = 0;
	m_inMaxPixelCount = 0;
	int i;
	for( i=0 ; i<9 ; i++ )
		m_ptrReducibleNodes[i] = NULL;
}



