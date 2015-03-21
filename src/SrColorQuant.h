/************************************************************************		
\link	www.twinklingstar.cn
\author Twinkling Star
\date	2013/11/21
****************************************************************************/
#ifndef SR_IMAGES_COLOR_QUANT_H_
#define SR_IMAGES_COLOR_QUANT_H_
/** \addtogroup images
  @{
*/

#include <malloc.h>
#include <algorithm>

/*	
/brief 颜色量化类

通过八叉树算法来实现颜色量化，给定一个允许的颜色数上限MaxColors，量化后的颜色数一定要小于这个值
可以创建量化后调色板，使用原始颜色可以快速的检索出对应调色板的索引值
*/

class SrColorQuant
{
protected:

#pragma pack(push)
#pragma pack(1)
	typedef struct  _OctreeNode
	{
		bool			blIsLeaf;							// TRUE if node has no children
		unsigned char	inColorIndex;						// Index of the pallette
		unsigned int	uiPixelCount;						// Number of pixels represented by this leaf
		unsigned int	uiRedSum;							// Sum of red components
		unsigned int	uiGreenSum;							// Sum of green components
		unsigned int	uiBlueSum;							// Sum of blue components
		_OctreeNode*	ptrChild[8];						// Pointers to child nodes
		_OctreeNode*	ptrNext;							// Pointer to next reducible node
	}OctreeNode;
#pragma  pack(pop)

public:
	SrColorQuant( );
	~SrColorQuant();
	/*
	\brief 创建八叉树
	\param[in] btPtrRgbData	需要量化的颜色数组,每个颜色占三个字节，依次为红、绿、蓝，每种颜色分量以1个字节保存。
	\param[in] inPixelCount 需要量化的颜色个数
	\param[in] nMaxColors 规定量化后最大允许的颜色数，它的值必需[1,256]范围内。
	\return 采用八叉树量化的颜色数个数。如果返回0，则八叉树创建失败
	*/
	int		buildOctree( unsigned char* btPtrRgbData,int inPixelCount,int nMaxColors);
	/*
	\brief 根据颜色RGB,从已创建出的八叉树中获得相近颜色的索引
	\param[in] (byRed,byGreen,byBlue) RGB颜色值
	*/
	unsigned char  indexOctree(unsigned char byRed,unsigned char byGreen,unsigned char byBlue)const;
	/*
	\brief 叶节点的最大计数值
	*/
	int		getMaxPixelCount() const;
	/*
	\brief  叶节点的数量，即量化后的颜色种数
	*/
	int		getLeafNodeCount()const;
	/*
	\brief 如果八叉树为空，返回true
	*/
	bool	isEmpty()const;
	/*
	\brief 获得量化后的调色板，每个颜色占三个字节，依次为红、绿、蓝，每种颜色分量以1个字节保存。
	\param[in] ptrColorPal 不能为空，内存需要提前分配，至少有getLeafNodeCount()*3大小的内存空间。
	*/
	void	getColorPallette(unsigned char* ptrColorPal )const;

protected:
	void		getColorPallette(OctreeNode*ptrTree, unsigned char* ptrColorPal)const;
	OctreeNode* createNode (int inLevel);
	/*
	\brief	合并八叉树节点，降低叶节点数
	*/
	void		reduceTree ();
	/*
	\brief 递归的往八叉树中插入颜色值
	\param[in] inLevel表示当前插入的层数，初始值是0，每递归一层，值加1 
	*/
	bool		addColor(OctreeNode*& ptrTreeNode,unsigned char byRed,unsigned char byGreen,unsigned char byBlue,int inLevel);
	/*
	\brief 计算每个叶节点的颜色值同时把颜色计数值设置为1.
	*/
	void		setColorIndex(OctreeNode* ptrTree,int& inIndex);
	/*
	\brief	析构八叉树内存
	*/
	void		freeOctree(OctreeNode*& ocPtrTree);
	/*
	\brief 清空八叉树
	*/
	void		empty();


	unsigned int	m_inMaxPixelCount;
	int				m_inLeafCount;	
	OctreeNode*		m_ptrReducibleNodes[9];
	OctreeNode*		m_ptrTree;
};


/** @} */
#endif