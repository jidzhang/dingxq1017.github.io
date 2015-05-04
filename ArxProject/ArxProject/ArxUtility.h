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
	bool Equal(int a, int b) { return a == b; }
	bool Equal(double a, double b) { return fabs(a-b) < EPS; }
 	bool LessThan(int a, int b) { return a < b; }
	bool LessThan(double a, double b) { return a + EPS < b; }
	bool LessThanOrEqual(int a, int b) { return a <= b; }
	bool LessThanOrEqual(double a, double b) { return a + EPS < b || a < b + EPS; }
	
	template<typename T>
	T Max(T a, T b) { return LessThan(a, b) ? b : a; }
	template<typename T>
	T Min(T a, T b) { return LessThan(a, b) ? a : b; }

}


namespace arxcookbook
{

bool PointInPolygon(AcGePoint3d pt, AcGePoint3dArray plVertices);
bool neoPointInPolygon(AcGePoint3d pt, AcGePoint3dArray plVertices);
bool RgnInRgn(const AcDbRegion * pRegion1,const AcDbRegion * pRegion2);
bool GetPolyCentroid(AcDbPolyline * pPline, ads_point CenPt);
int GetColorIndexFromColorRef(COLORREF color);
COLORREF GetColorRefFromIndex(int colorIndex);

}
