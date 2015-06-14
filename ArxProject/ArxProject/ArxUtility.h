#pragma once

#ifndef DOUBLE_EPSILON
#define DOUBLE_EPSILON
#ifdef HIGH_PRECISION
#define EPS 1E-6
#else
#define EPS 1E-3
#endif
#endif //DOUBLE_EPSILON


namespace arxmath
{
	inline bool Equal(int a, int b) { return a == b; }
	inline bool Equal(double a, double b) { return fabs(a-b) < EPS; }
 	inline bool LessThan(int a, int b) { return a < b; }
	inline bool LessThan(double a, double b) { return a + EPS < b; }
	inline bool LessThanOrEqual(int a, int b) { return a <= b; }
	inline bool LessThanOrEqual(double a, double b) { return a + EPS < b || a < b + EPS; }
	
	template<typename T>
	T Max(T a, T b) { return LessThan(a, b) ? b : a; }
	template<typename T>
	T Min(T a, T b) { return LessThan(a, b) ? a : b; }

}


namespace arxcookbook
{

//���ڶ������
bool PointInPolygon(AcGePoint3d pt, AcGePoint3dArray plVertices);
bool neoPointInPolygon(AcGePoint3d pt, AcGePoint3dArray plVertices);
//������������
bool RgnInRgn(const AcDbRegion * pRegion1,const AcDbRegion * pRegion2);
//���������
bool GetPolyCentroid(AcDbPolyline * pPline, ads_point CenPt);
//��ɫת��
int GetColorIndexFromColorRef(COLORREF color);
COLORREF GetColorRefFromIndex(int colorIndex);

}
