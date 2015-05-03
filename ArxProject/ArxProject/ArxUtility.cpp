#include "StdAfx.h"
#include "ArxUtility.h"

namespace arxcookbook
{

//功能：判断点 pt 是否在区域 ptArr 内
//实现：根据射线法求交点个数，偶数－区域外，奇数－区域内
//变量：pt 指定点, ptArr 判断区域
//返回：在区域 TRUE, 不在 FALSE
BOOL PointIsInPolygon(AcGePoint3d pt, AcGePoint3dArray ptArr)
{
	int ptNum,i,interNum;
	AcGePoint3d ptA,ptB;
	ads_point pt0,pt1,pt2,ptIns,ptX;

	interNum = 0;
	//无限向右延伸的射线
	pt0[X] = pt.x;
	pt0[Y] = pt.y;
	pt0[Z] = pt.z;
	ptX[X] = 1E10;
	ptX[Y] = pt.y;
	ptX[Z] = pt.z;
	ptNum = ptArr.length();
	for (i = 0;i < ptNum - 1;i++) {
		//多边形的一条边
		ptA = ptArr.at(i);
		ptB = ptArr.at(i + 1);
		pt1[X] = ptA.x;
		pt1[Y] = ptA.y;
		pt1[Z] = 0.0;
		pt2[X] = ptB.x;
		pt2[Y] = ptB.y;
		pt2[Z] = 0.0;
		if (acdbInters(ptX,pt0,pt1,pt2,1,ptIns) == RTNORM) {
			//如果交点恰好是边的上部顶点，则不计数
			if (!arxmath::Equal(ptIns[Y], arxmath::Max(ptA.y, ptB.y)))
				interNum++;
		}
	}
	if (interNum % 2 == 0)
		return false;
	else
		return true;
}
BOOL neoPointIsInPolygon(AcGePoint3d pt, AcGePoint3dArray ptArr)
{
	int interNum = 0;
	//无限向右延伸的射线
	AcGePoint3d ptX = pt;
	ptX.x = 1E10;
	AcGeLine3d xline(pt, ptX);
	int ptNum = ptArr.length();
	ptArr.append(ptArr.at(0));
	for (int i = 0; i < ptNum; i++) {
		//多边形的一条边
		AcGePoint3d ptA = ptArr.at(i);
		AcGePoint3d ptB = ptArr.at(i + 1);
		AcGeLine3d next_line(ptA, ptB);
		AcGePoint3d ptIns;
		if (xline.intersectWith(next_line, ptIns) == Adesk::kTrue)  {
			//如果交点恰好是边的上部顶点，则不计数
			if (!arxmath::Equal(ptIns[Y], arxmath::Max(ptA.y, ptB.y)))
				interNum++;
		}
	}
	if (interNum % 2 == 0)
		return false;
	else
		return true;
}

} //namespace arxcookbook