#include "StdAfx.h"
#include "ArxUtility.h"

namespace arxcookbook
{

//功能：判断点 pt 是否在区域 plVertices 内
//实现：根据射线法求交点个数，偶数－区域外，奇数－区域内
//变量：pt: 指定点, plVertices: 多边形顶点 
//返回：在区域 TRUE, 不在 false
bool PointInPolygon(AcGePoint3d pt, AcGePoint3dArray plVertices)
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
	ptNum = plVertices.length();
	plVertices.append(plVertices.at(0));
	for (i = 0; i < ptNum; i++) {
		//多边形的一条边
		ptA = plVertices.at(i);
		ptB = plVertices.at(i + 1);
		pt1[X] = ptA.x;
		pt1[Y] = ptA.y;
		pt1[Z] = 0.0;
		pt2[X] = ptB.x;
		pt2[Y] = ptB.y;
		pt2[Z] = 0.0;
		if (acdbInters(ptX,pt0,pt1,pt2,1,ptIns) == RTNORM) {
			//如果交点恰好是边的上部顶点，则不计数
			if (!arxmath::Equal(ptIns[Y], arxmath::MAX(ptA.y, ptB.y)))
				interNum++;
		}
	}
	if (interNum % 2 == 0)
		return false;
	else
		return true;
}
bool neoPointInPolygon(AcGePoint3d pt, AcGePoint3dArray plVertices)
{
	int interNum = 0;
	//无限向右延伸的射线
	AcGePoint3d ptX = pt;
	ptX.x = 1E10;
	AcGeLine3d xline(pt, ptX);
	int ptNum = plVertices.length();
	plVertices.append(plVertices.at(0));
	for (int i = 0; i < ptNum; i++) {
		//多边形的一条边
		AcGePoint3d ptA = plVertices.at(i);
		AcGePoint3d ptB = plVertices.at(i + 1);
		AcGeLine3d next_line(ptA, ptB);
		AcGePoint3d ptIns;
		if (xline.intersectWith(next_line, ptIns) == Adesk::kTrue)  {
			//如果交点恰好是边的上部顶点，则不计数
			if (!arxmath::Equal(ptIns[Y], arxmath::MAX(ptA.y, ptB.y)))
				interNum++;
		}
	}
	if (interNum % 2 == 0)
		return false;
	else
		return true;
}

//区域是否在区域内
// Function name : RgnInRgn
// Description: is Region1 in Region2?
// Return type : bool 
// Argument : const AcDbRegion * pRegion1
// Argument : const AcDbRegion * pRegion2
bool RgnInRgn(const AcDbRegion * pRegion1,const AcDbRegion * pRegion2)
{
    if (pRegion1==NULL||pRegion2==NULL) return false;

    AcDbObjectPointer<AcDbRegion> spRegion1;
    AcDbObjectPointer<AcDbRegion> spRegion2;
    if (spRegion1.create()!=Acad::eOk)
    {
        acdbFail(_T("\n内存不足"));
        return false;
    }
    if (spRegion2.create()!=Acad::eOk)
    {
        acdbFail(_T("\n内存不足"));
        return false;
    }

    if ((spRegion1->copyFrom(pRegion1)!= Acad::eOk)||(spRegion2->copyFrom(pRegion2)!= Acad::eOk))
    {
        acdbFail(_T("\n无法复制对象"));
        return false;
    }

    bool bResult=false;
    if(spRegion1->booleanOper(AcDb::kBoolIntersect, spRegion2.object()) == Acad::eOk)
    {
        if ((spRegion2->isNull()==Adesk::kTrue)&&(spRegion1->isNull()!=Adesk::kTrue))
        {
            double area1,area0;
            spRegion1->getArea(area1);
            pRegion1->getArea(area0);

            if ((area0 - area1) < AcGeContext::gTol.equalPoint())
                bResult=true;
        }
    }

    return bResult;
}

//计算多边形的形心坐标
bool GetCentroid(AcDbPolyline * pPline, ads_point CenPt)
{
    unsigned int i, iCount = 0;
    AcDbVoidPtrArray curveSegments, regions;
    AcGePoint3d LinePt0, LinePt1;
    AcGePoint3d origin;
    AcGeVector3d xAxis, yAxis;
    double perimeter, area, prodInertia;
    double momInertia[2], prinMoments[2], radiiGyration[2];
    AcGePoint2d centroid;
    AcGeVector2d prinAxes[2];
    AcGePoint2d extentsLow, extentsHigh;

    if (pPline->isClosed() != Adesk::kTrue) {
        ads_printf(_T("\n折线不封闭, 无法形成正确的区域。"));
        return false;
    }
    curveSegments.append((AcDbCurve *) pPline);

    if (AcDbRegion::createFromCurves(curveSegments, regions) != Acad::eOk) {
        ads_printf(_T("\n创建临时区域对象失败!"));
        //清除Region, 应第9 贴的指点，即使createFromCurves错误，也应清除之；
        iCount = regions.length();
        for(i = 0; i < iCount; i++)
            delete (AcDbRegion *)regions.at(i);

        return false;
    }
    AcDbRegion * pRegion;
    if ((iCount = regions.length()) == 0) {
        ads_printf(_T("\n创建临时区域对象为空!"));
        return false;
    }
    if (iCount > 1) {
        // 多个 AcDbRegion , 无法确定应该返回哪一个，干脆返回NULL;
        ads_printf(_T("\n多个区域实体。"));
        for(i = 0; i < iCount; i++)
            delete (AcDbRegion *)regions.at(i); 
        return false;
    }
    pRegion = (AcDbRegion *) regions.at(0);

    origin.set(0,0,0); //设置原点坐标
    xAxis.set(1,0,0);  //设置X Y轴，
    yAxis.set(0,1,0);

    if (pRegion->getAreaProp(
                origin, xAxis, yAxis,
                perimeter, area, centroid, momInertia, prodInertia, prinMoments, prinAxes, radiiGyration,
                extentsLow, extentsHigh) != Acad::eOk ){
        ads_printf(_T("\n区域面积: %.3f, 周长:%.3f"), area, perimeter);
        ads_printf(_T("\n获取区域对象属性失败!"));
        delete pRegion; 
        return false;
    }
    //得到形心坐标
	CenPt[X] = centroid.x;
	CenPt[Y] = centroid.y;
	CenPt[Z] = 0;
    ads_printf(_T("\n区域面积: %.3f, 周长:%.3f"), area, perimeter);
    pRegion->close();
    delete pRegion;

    return true;
}

//从RGB得到CAD颜色索引值 
int GetColorIndexFromColorRef(COLORREF color)
{
    long acirgb, r,g,b;
    long mindst = 2147483647L;
    long dst = 0;
    int minndx = 0;
    long red=GetRValue(color);
    long green=GetGValue(color);
    long blue=GetBValue(color);
    for ( int i = 1; i < 255; i++ ) {
        acirgb = acdbGetRGB ( i );
        r = GetRValue(acirgb);
        g = GetGValue(acirgb);
        b = GetBValue(acirgb);

        dst = abs ( r-red) + abs ( g -green) + abs (b-blue);
        if ( dst < mindst ) {
            minndx = i;
            mindst = dst;
        }
    }
    return minndx;

}  

//功能：从CAD的颜色得到RGB
COLORREF GetColorRefFromIndex(int colorIndex)
{
    if(colorIndex < 0 || colorIndex > 255)
    {
        ads_alert(_T("传入的颜色号不在0~255之间!"));
        return 0;
    }

    BYTE R, G, B;
#ifdef _ARX2002_
    R = lpszRGBData[colorIndex*3+0];
    G = lpszRGBData[colorIndex*3+1];
    B = lpszRGBData[colorIndex*3+2];
#else
    long zhi = acdbGetRGB(colorIndex);
    WORD LOW = LOWORD(zhi);
    WORD HIG = HIWORD(zhi);
    R = LOBYTE(LOW);
    G = HIBYTE(LOW);
    B = LOBYTE(HIG);
#endif

    return RGB(R,G,B);
    //return acdbGetRGB(nColor); 
}

} //namespace arxcookbook
