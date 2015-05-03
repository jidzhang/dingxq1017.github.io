//ARX函数

//选择指定图层上的所有实体
Acad::ErrorStatus selectEntityInLayer(const char * nLayerName, AcDbObjectIdArray & nIDs)
{
    Acad::ErrorStatus es = Acad::eOk;

    ads_name ents;
    struct resbuf *rb;
    rb=acutNewRb(AcDb::kDxfLayerName);
    rb->restype=8;
    rb->resval.rstring=(char*)nLayerName;
    rb->rbnext=NULL;
    acedSSGet("X",NULL,NULL,rb,ents);
    long entNums=0;
    acedSSLength(ents,&entNums);
    if (entNums == 0)
        es = Acad::eInvalidInput;
    else
    {
        for (long a = 0; a < entNums ; a ++)
        {
            AcDbObjectId objId;
            ads_name      ent;
            acedSSName(ents,a,ent);
            acdbGetObjectId(objId, ent);
            nIDs.append(objId);
        }
    }
    acedSSFree(ents);
    acutRelRb(rb);

    return es;
}

//设置当前层
Acad::ErrorStatus SetCurLayer(const char* lpLayerName, AcDbDatabase* pDb/* = NULL */)
{
    AcDbDatabase* pCurDb = pDb;
    if (pCurDb == NULL)
        pCurDb = acdbHostApplicationServices()->workingDatabase();

    AcDbLayerTableRecordPointer spRecord(lpLayerName, pCurDb, AcDb::kForRead);
    Acad::ErrorStatus es = spRecord.openStatus();
    if (es == Acad::eOk)
    {
        es = pCurDb->setClayer(spRecord->objectId());
    }
    return es;
}

//生成新组
//生成新组(sGroupName)
//追加数组中所有实体到该组中
//组名字 , Id数组
int createGroup(CString sGroupName, const AcDbObjectIdArray * idArr)
{
    AcDbGroup *pGroup = new AcDbGroup((LPSTR)(LPCTSTR)sGroupName);
    AcDbObjectId groupObjectId;
    AcDbDictionary *pGroupDict = NULL;

    acdbHostApplicationServices()->workingDatabase()
        ->getGroupDictionary(pGroupDict, AcDb::kForWrite);
    pGroupDict->setAt(sGroupName, pGroup, groupObjectId);
    pGroupDict->close();
    pGroup->close();
    acdbOpenObject(pGroup, groupObjectId, AcDb::kForWrite);
    for (int i = 0; i < idArr->length(); i++)
    {
        groupObjectId = idArr->at(i);
        pGroup->append(groupObjectId); 
    }
    pGroup->close();
    return TRUE;
}

//建立文本格式表函数
AcDbObjectId CreateNewTextStyle()
{
    AcDbTextStyleTable *pTextStyleTable;
    AcDbTextStyleTableRecord *pTextStyleTableRcd
        AcDbObjectId textId;
    acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pTextStyleTable,AcDb::kForWrite);

    if (!pTextStyleTable->has(StyleName))
    {
            AcDbTextStyleTableRecord *pTSTblRcd= new AcDbTextStyleTableRecord;
            pTSTblRcd->setName(StyleName);
            pTSTblRcd->setFileName(fontName);
            pTSTblRcd->setBigFontFileName(bigfontName);
            pTSTblRcd->setTextSize(textSize);
            pTSTblRcd->setXScale(xScale);
            pTSTblRcd->setObliquingAngle(obliqueAngle);
            pTSTblRcd->setPriorSize(trPercent);
            pTextStyleTable->add(textId,pTextStyleTableRcd);
            pTSTblRcd->close();
    }
    pTextStyleTable->close();
    return textId;
}

//区域是否在区域内
// Function name : RgnInRgn
// Descrīption : is Region1 in Region2?
// Return type : bool 
// Argument : const AcDbRegion* pRegion1
// Argument : const AcDbRegion* pRegion2
bool RgnInRgn(const AcDbRegion* pRegion1,const AcDbRegion* pRegion2)
{
    if (pRegion1==NULL||pRegion2==NULL) return false;

    AcDbObjectPointer< AcDbRegion > spRegion1;
    AcDbObjectPointer< AcDbRegion > spRegion2;
    if (spRegion1.create()!=Acad::eOk)
    {
        acdbFail("\n内存不足");
        return false;
    }
    if (spRegion2.create()!=Acad::eOk)
    {
        acdbFail("\n内存不足");
        return false;
    }

    if ((spRegion1->copyFrom(pRegion1)!= Acad::eOk)||(spRegion2->copyFrom(pRegion2)!= Acad::eOk))
    {
        acdbFail("\n无法复制对象");
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

//组旋转
void CMyDatabase::rotationGroup(CString strGroupName ,CReiPoint ptRotation,double rotationAngle) 
{
    AcGePoint3d pt;
    AcDbDictionary *pGroupDict;
    acdbCurDwg()->getGroupDictionary(pGroupDict,AcDb::kForWrite);
    AcDbObjectId groupId;
    AcDbGroup *pGroup;
    pt.x=ptRotation.x;
    pt.y=ptRotation.y;
    pt.z=ptRotation.z;
    if(pGroupDict->getAt(strGroupName,groupId)==Acad::eOk)
        acdbOpenObject(pGroup,groupId,AcDb::kForWrite);
    else
    {
        pGroupDict->close();
        return;
    }
    pGroupDict->close();
    AcDbGroupIterator* pIter=pGroup->newIterator();
    AcDbEntity* pEnt;
    AcDbObjectId objId;
    pIter=pGroup->newIterator();
    for(;!pIter->done();pIter->next())
    {
        objId=pIter->objectId();
        acdbOpenAcDbEntity(pEnt,objId,AcDb::kForWrite);
        rotationEntity(pEnt,pt,rotationAngle);
        pEnt->close();
    }
    delete pIter;
    pGroup->close();
}

//点是否在区域内
//功能：判断点 pt 是否在区域 ptArr 内
//实现：根据射线法求交点个数，偶数－区域外，奇数－区域内
//变量：pt 指定点 ptArr 判断区域
//返回：在区域 TRUE 不在 FALSE
BOOL BaseHandle::PointIsInPolygon(AcGePoint3d pt, AcGePoint3dArray ptArr)
{
    int ptNum,i,interNum;
    AcGePoint3d ptA,ptB;
    ads_point pt0,pt1,pt2,ptIns,ptX;

    interNum = 0;
    pt0[X] = 0.0;
    pt0[Y] = 0.0;
    pt0[Z] = 0.0;
    ptX[X] = pt.x;
    ptX[Y] = pt.y;
    ptX[Z] = pt.z;
    ptNum = ptArr.length();
    for (i = 0;i < ptNum - 1;i++) {
        ptA = ptArr.at(i);
        ptB = ptArr.at(i + 1);
        pt1[X] = ptA.x;
        pt1[Y] = ptA.y;
        pt1[Z] = 0.0;
        pt2[X] = ptB.x;
        pt2[Y] = ptB.y;
        pt2[Z] = 0.0;
        if (acdbInters(ptX,pt0,pt1,pt2,1,ptIns) == RTNORM){
            interNum++;
        }
    }
    if (interNum % 2 == 0)
        return false;
    else
        return true;
}

/******************************************************************
  函数名:         setView
  功能:           设置视图（相当于Zoom Window命令）   
  输入参数:       Pt1 -- 视图左上角点
  Pt2 -- 视图右下角点
  ex_ratio -- 扩展比率,一般为1.0
  输出参数:   
  返回值:         void
  其它:   
 *****************************************************************/
void setView(AcGePoint2d Pt1, AcGePoint2d Pt2, double ex_ratio)
{
    AcGePoint2d CenterPt;

    //若X坐标或Y坐标重合，判为意外，不进行SetView操作
    if ((fabs(Pt1.x-Pt2.x)<1e-6)||(fabs(Pt1.y-Pt2.y)<1e-6))
        return;

    //确保两个坐标点分别为左上角和右下角
    if (Pt1.x>Pt2.x) {
        double tmp;
        tmp = Pt1.x;
        Pt1.x = Pt2.x;
        Pt2.x = tmp;
    }
    if (Pt2.y>Pt1.y) {
        double tmp;
        tmp = Pt1.y;
        Pt1.y = Pt2.y;
        Pt2.y = tmp;
    }

    //获取当前DwgView的尺寸
    CRect CADrect;
    acedGetAcadDwgView()->GetClientRect(&CADrect);

    double width,height,ratio;

    ratio = (double)(CADrect.right-CADrect.left)/(double)(CADrect.bottom-CADrect.top);

    if (fabs(ratio)<1e-6)
        return;

    if ((Pt2.x-Pt1.x)/(Pt1.y-Pt2.y) > ratio) {
        width = Pt2.x-Pt1.x;
        height = width/ratio;
    }else{
        height = Pt1.y-Pt2.y;
        width = height * ratio;
    }

    //设置当前视图中心点
    CenterPt.x = (Pt1.x+Pt2.x)/2;
    CenterPt.y = (Pt1.y+Pt2.y)/2;

    //改变当前视图
    AcDbViewTableRecord pVwRec;
    pVwRec.setCenterPoint(CenterPt);
    pVwRec.setWidth(width * ex_ratio);
    pVwRec.setHeight(height * ex_ratio); 
    acedSetCurrentView( &pVwRec, NULL );
}

///////////////////////////////////////////////////////////////
//    函数名 : oxaGetVar
//    函数功能 : 
//    处理过程 : 
//    备   注 : 
//    作   者 : user
//    时   间 : 2004年6月16日
//    返回值 : int
//    参数说明 : const CString strSym,
//                 AcGePoint3d &vOut 
///////////////////////////////////////////////////////////////
int oxaGetVar(const CString strSym, AcGePoint3d &vOut )
{
    resbuf rbVar ; 
    int iRt=acedGetVar(strSym, &rbVar) ;
    if (iRt!=RTNORM)
    {
        return iRt;
    }
    //oxaPrint(&rbVar); 

    if (rbVar.restype==RTPOINT)
    {
        vOut.x=rbVar.resval.rpoint[0];
        vOut.y=rbVar.resval.rpoint[1];
    }    
    if (rbVar.restype==RT3DPOINT)
    {
        vOut.x=rbVar.resval.rpoint[0];
        vOut.y=rbVar.resval.rpoint[1];
        vOut.z=rbVar.resval.rpoint[2];
    }    
    return RTNORM;
}

/////////////////////////////////////////////////////////////////////////////////
//# DOC.BEGIN
//# 函数名称： oxaGetVar
//# 函数编号： OXA
//# 函数声明： 
//# 函数参数： const CString strSym,
//                 int &vOut 
//# 返回值：   int
//# 函数分类： 
//# 函数功能：获取系统变量，封装acedGetVar()
//# 注意事项： 
//# 涉及的全局变量：
//# 调用的OXARX函数： 
//# 函数算法： 
//# ACAD版本：R14 R15 R16
//# 配合函数：
//# 类似函数：
//# 替换函数： 
//# 现存缺陷：
//# 示例程序：
//# 测试要求： 
//# 历史记录： 2003年11月10日 , zjw ,完成
//
//# DOC.END
//////////////////////////////////////////////////////////////////////////
int oxaGetVar(const CString strSym, int &vOut )
{   
    resbuf rbVar;
    int iRt=acedGetVar(strSym, &rbVar) ;
    if (iRt!=RTNORM)
    {
        return iRt;
    }

    if (rbVar.restype==RTLONG)
    {
        vOut=rbVar.resval.rlong;
    }
    if (rbVar.restype==RTSHORT) 
    {
        vOut=rbVar.resval.rint;
    }

    return RTNORM;
}

/////////////////////////////////////////////////////////////////////////////////
//# DOC.BEGIN
//# 函数名称： oxaGetVar
//# 函数编号： OXA
//# 函数声明： 
//# 函数参数： const CString strSym,
//                 double &vOut 
//# 返回值：   int
//# 函数分类： 
//# 函数功能：获取系统变量，封装acedGetVar()
//# 注意事项： 
//# 涉及的全局变量：
//# 调用的OXARX函数： 
//# 函数算法： 
//# ACAD版本：R14 R15 R16
//# 配合函数：
//# 类似函数：
//# 替换函数： 
//# 现存缺陷：
//# 示例程序：
//# 测试要求： 
//# 历史记录： 2003年11月24日 , zjw ,完成
//
//# DOC.END
int oxaGetVar(const CString strSym, double &vOut )
{   
    resbuf rbVar;
    int iRt=acedGetVar(strSym, &rbVar) ;
    if (iRt!=RTNORM)
    {
        return iRt;
    }

    if (rbVar.restype==RTREAL)
    {
        vOut=rbVar.resval.rreal;
    }    
    return RTNORM;
}

/////////////////////////////////////////////////////////////////////////////////
//# DOC.BEGIN
//# 函数名称： oxaGetVar
//# 函数编号： OXA
//# 函数声明： 
//# 函数参数： const CString strSym,
//                 CString &vOut 
//# 返回值：   int
//# 函数分类： 
//# 函数功能：获取系统变量，封装acedGetVar()
//# 注意事项： 
//# 涉及的全局变量：
//# 调用的OXARX函数： 
//# 函数算法： 
//# ACAD版本：R14 R15 R16
//# 配合函数：
//# 类似函数：
//# 替换函数： 
//# 现存缺陷：
//# 示例程序：
//# 测试要求： 
//# 历史记录： 2003年11月24日 , zjw ,完成
//
//# DOC.END
int oxaGetVar(const CString strSym, CString &vOut )
{   
    resbuf rbVar;
    int iRt=acedGetVar(strSym, &rbVar) ;
    if (iRt!=RTNORM)
    {
        return iRt;
    }

    if (rbVar.restype==RTSTR)
    {
        vOut=rbVar.resval.rstring;
    }    
    return RTNORM;
}
// 函数名   : SetCurTextStyle
// 描述     : 设置当前TextStyle
// 返回     : Acad::ErrorStatus 
// 参数     : const char* lpStyleName
// 参数     : AcDbDatabase* pDb/* = NULL */
Acad::ErrorStatus SetCurTextStyle(const char* lpStyleName, AcDbDatabase* pDb/* = NULL */)
{
    AcDbDatabase* pCurDb = pDb;
    if (pCurDb == NULL)
        pCurDb = acdbHostApplicationServices()->workingDatabase();

    AcDbTextStyleTableRecordPointer spRecord(lpStyleName, pCurDb, AcDb::kForRead);
    Acad::ErrorStatus es = spRecord.openStatus();
    if (es == Acad::eOk)
    {
        es = pCurDb->setTextstyle(spRecord->objectId());
    }
    return es;
}
// Function name   : SetCurLayer
// Descrīption     : 设置当前层
// Return type     : Acad::ErrorStatus 
// Argument       : const char* lpLayerName
// Argument       : AcDbDatabase* pDb/* = NULL */
Acad::ErrorStatus SetCurLayer(const char* lpLayerName, AcDbDatabase* pDb/* = NULL */)
{
    AcDbDatabase* pCurDb = pDb;
    if (pCurDb == NULL)
        pCurDb = acdbHostApplicationServices()->workingDatabase();

    AcDbLayerTableRecordPointer spRecord(lpLayerName, pCurDb, AcDb::kForRead);
    Acad::ErrorStatus es = spRecord.openStatus();
    if (es == Acad::eOk)
    {
        es = pCurDb->setClayer(spRecord->objectId());
    }
    return es;
}
//获取属性块中所有属性的字符串值，并且存于链表中
void FEGroups::iterateDictionary()
{
    //obtain the GROUP dictionary by looking up "ACAD_GROUP" in the named object dictionary
    //
    /* AcDbDictionary *pNamedobj;
       acdbHostApplicationServices()->workingDatabase()
       ->getNamedObjectsDictionary(pNamedobj, AcDb::kForRead);

    // Get a pointer to the ASDK_DICT dictionary.
    //
    AcDbDictionary *pDict;
    pNamedobj->getAt("ACAD_GROUP", (AcDbObject*&)pDict,
    AcDb::kForRead);
    pNamedobj->close();
    */
    // Get a pointer to the ACAD_GROUP dictionary
    AcDbDictionary *pDict;
    acdbHostApplicationServices()->workingDatabase()
        ->getGroupDictionary(pDict, AcDb::kForRead);

    // Get an iterator for the ASDK_DICT dictionary.
    //
    AcDbDictionaryIterator* pDictIter = pDict->newIterator();
    AcDbGroup *pGroup;
    char* name;
    for (; !pDictIter->done(); pDictIter->next()) {
        // Get the current record, open it for read, and
        // print its name.
        //
        pDictIter->getObject((AcDbObject*&)pGroup,
                AcDb::kForRead);
        pGroup->getName(name);
        pGroup->close();
        acutPrintf("\nintval is: %s", name);
    }
    delete pDictIter;
    pDict->close();

}

//检测AutoCAD是否已经运行
void Autocadtest() 
{
    IAcadApplication m_autocad;
    IAcadDocuments m_acaddocs;
    IAcadDocument m_acaddoc;
    IAcadModelSpace m_acadmodel;

    LPDISPATCH pDisp;
    LPUNKNOWN pUnk;
    CLSID clsid;
    BeginWaitCursor();
    ::CLSIDFromProgID(L"AutoCAD.Application",&clsid);
    if(::GetActiveObject(clsid,NULL,&pUnk)==S_OK)
    {
        VERIFY(pUnk->QueryInterface(IID_IDispatch,(void**) &pDisp)==S_OK);
        m_autocad.AttachDispatch(pDisp);
        pUnk->Release();
    }
    else
    {
        if(!m_autocad.CreateDispatch("AutoCAD.Application"))
        {
            AfxMessageBox("Autocad program not found\n");
            exit(1);
        }
    }
    m_autocad.SetVisible(true);
    m_acaddocs.AttachDispatch(m_autocad.GetDocuments(),true);
    m_acaddoc.AttachDispatch(m_acaddocs.Add(vtMissing),true);
    m_acadmodel.AttachDispatch(m_acaddoc.GetModelSpace(),true);
    m_acadmodel.AddCircle(pVal,100);

    m_acadmodel.ReleaseDispatch();
    m_acaddoc.ReleaseDispatch();
    m_acaddocs.ReleaseDispatch();
    m_autocad.ReleaseDispatch();
}

//计算多边形的形心坐标
BOOL GetPolyCentroid(AcDbPolyline * pPline, ads_point CenPt)
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
        ads_printf("\n折线不封闭, 无法形成正确的区域。");
        return FALSE;
    }
    curveSegments.append((AcDbCurve *) pPline);

    if (AcDbRegion::createFromCurves(curveSegments, regions) != Acad::eOk) {
        ads_printf("\n创建临时区域对象失败!");
        //清除Region, 应第9 贴的指点，即使createFromCurves错误，也应清除之；
        iCount = regions.length();
        for(i = 0; i < iCount; i++)
            delete (AcDbRegion *)regions.at(i);

        return FALSE;
    }
    AcDbRegion * pRegion;
    if ((iCount = regions.length()) == 0) {
        ads_printf("\n创建临时区域对象为空!");
        return FALSE;
    }
    if (iCount > 1) {
        // 多个 AcDbRegion , 无法确定应该返回哪一个，干脆返回NULL;
        ads_printf("\n多个区域实体。");
        for(i = 0; i < iCount; i++)
            delete (AcDbRegion *)regions.at(i); 
        return FALSE;
    }
    pRegion = (AcDbRegion *) regions.at(0);

    origin.set(0,0,0); //设置原点坐标
    xAxis.set(1,0,0); //设置X Y轴，
    yAxis.set(0,1,0);

    if (pRegion->getAreaProp(
                origin, xAxis, yAxis,
                perimeter, area, centroid, momInertia, prodInertia, prinMoments, prinAxes, radiiGyration,
                extentsLow, extentsHigh) != Acad::eOk ){
        ads_printf("\n区域面积: %.3f, 周长:%.3f", area, perimeter);
        ads_printf("\n获取区域对象属性失败!");
        delete pRegion; 
        return FALSE;
    }
    XYZ_POINT(CenPt, centroid.x, centroid.y, 0); //得到形心坐标
    ads_printf("\n区域面积: %.3f, 周长:%.3f", area, perimeter);
    pRegion->close();
    delete pRegion;

    return TRUE;
}

AcDbObjectId CreateHatch(
        AcDbObjectId dbOId,
        char cLayer[], 
        char cPattern[] = "SOLID", 
        int nColor = 256, 
        double dAngle = 0.0, 
        double dScale = 1.0, 
        AcDbDatabase * pDbDatab = acdbHostApplicationServices()->workingDatabase())
{
    AcCmColor CmC;
    AcDbObjectId DbOId;
    AcDbObjectIdArray DbOIdA(0, 2);
    AcDbBlockTable * pDbBT;
    AcDbBlockTableRecord * pDbBTR;
    AcGeVector3d normal(0.0, 0.0, 1.0);
    DbOIdA.append(dbOId);
    AcDbHatch* pDbHat = new AcDbHatch();
    pDbHat->setDatabaseDefaults();
    pDbHat->setAssociative(Adesk::kTrue); // BUG: doesn't do squat! have to set the reactor yourself to get associativity!
    pDbHat->appendLoop(AcDbHatch::kExternal, DbOIdA);
    pDbHat->setPatternScale(dScale);
    pDbHat->setPatternAngle(dAngle);
    pDbHat->setPattern(AcDbHatch::kPreDefined, cPattern);
    pDbHat->setNormal(normal);
    pDbHat->evaluateHatch(); // crucial call or nothing gets displayed!
    pDbDatab->getSymbolTable(pDbBT, AcDb::kForRead);
    pDbBT->getAt(ACDB_MODEL_SPACE, pDbBTR, AcDb::kForWrite);
    pDbBTR->appendAcDbEntity(DbOId, pDbHat);
    pDbHat->setLayer(cLayer);
    CmC.setColorIndex(nColor);
    ((AcDbEntity *)pDbHat)->setColor(CmC);
    pDbBT->close();
    pDbBTR->close();
    pDbHat->close();

    return DbOId;
}

//=============================================
objectARX 常用功能实现集合

一、在ARX中禁用AutoCAD的某个命令
以LINE命令为例，在程序中加入下面的一句即可禁用LINE命令：

acedCommand(RTSTR, "undefine", RTSTR, "line",RTNONE);

下面的语句则可恢复LINE命令的定义：

acedCommand(RTSTR, "redefine", RTSTR, "line",RTNONE);

二、在对话框中预览DWG文件
使用acdbDisplayPreviewFromDwg函数，具体的方法为：
char fileName[100];
strcpy(fileName, "C:\\test.dwg");
bool es;
HWND pWnd;
CFrameWnd *pFrame = (CFrameWnd*)GetDlgItem(IDC_PICTURE);

es = acdbDisplayPreviewFromDwg(fileName, pFrame->m_hWnd);
上面的代码将在一个Picture控件中显示指定的图形。
另外，需要包含<dbmain.h>头文件。

三、通过ARX更改AutoCAD窗口的标题名称
CMDIFrameWnd *pp;
pp=acedGetAcadFrame();
pp->SetWindowText("yourName");
pp->UpdateWindow();

四、获得当前数据库
在ARX编程中，经常需要使用当前数据库，例如需要获得当前图形中设置的文字样式、标注样式等。
要获得当前数据库，都可以直接使用下面的方法：
AcDbTextStyleTable *pTextStyleTAble;
AcDbObjectId textstyleId;
textstyleId=acdbHostApplicationServices()->workingDatabase()->textstyle();

用acadCurDwg来代替acdbHostApplicationServices()->workingDatabase()，也能得到同样的结果。

五、将一个图插入另一个图(两种方法)
在实践中常常要将外部的图形文件source.dwg中插入到另外一个图形中或者当前图形中.以插入到当前图形dest.dwg为例.
为了将一个source.dwg插入dest.dwg中,首先要找到source.dwg这个文件. 
我们可以使用各种基本的技术和相应的规则从你的机器上或PDM数据库中检索到你要插入的source.dwg, 返回来一个字符窜sFileName代表整个文件及其路径.
然后创建一个空的数据库pNewDb读入source.dwg.
AcDbDatabase *pNewDb =new AcDbDatabase(Adesk::kFalse);
//在这里一定要用Adesk::kFalse
acDocManager->lockDocument(acDocManager->curDocument()) ;//如果确定你不需要LockDocument, 这一行可以不要

es=pNewDb->readDwgFile(sFileName , _SH_DENYNO,false);
if (es!=Acad::eOk)
{
    acutPrintf("\nThe file %s cannot be opend",sFileName);
    return;
}

这样,source.dwg以经用pNewDb来表示了. 我们用pDb来表示当前数据库
AcDbDatabase *pDb;
pDb =acdbHostApplicationServices ()->workingDatabase () ;

现在,我们用Insert来插入数据库. Insert有两种用法,一种是直接insert, source.dwg中的图元实体被分散地插入pDb中
pDb->insert( AcGeMatrix3d::kIdentity, pNewDb ); //这里假定不对source.dwg做比例和转角的变换. 如果我们在这里结束程序,我们能看到source.dwg已经被插入,但不是一个图块.

另外一种插入法是要求插入后source.dwg成为一个图块,图块的attribute也要从source.dwg中得到.这种方法要做大量的工作.首先运行insert()

CString pBlockName="TestBlock";
AcDbObjectId blockId;
if((es=pDb->insert(blockId, pBlockName,pNewDb, true))==Acad::eOk)
{
    acutPrintf("\ninsert ok\n");
}
else
{
    AfxMessageBox("Insert failed");
    delete pNewDb;
    return;
}

//这里blcokId是insert运行后产生的,它代表的是一个块表记录AcDbBlockRecord的ID. pBlockName是记录名,要在insert运行前设定其值.
如果我们在这里结束程序,我们看不到任何东西,因为source并没有真正被插入.
我们还要做一些事,首先是创建一个 AcDbBlockReference, 并将它指向blockId所代表的AcDbBlockRecord, 然后将这个AcDbBlockReference加入pDb所代表的图形数据库中.
AcDbBlockReference *pBlkRef = new AcDbBlockReference;
pBlkRef->setBlockTableRecord(blockId);  //指向blockId;
pBlkRef->setPosition(Pt);               //设定位置
pBlkRef->setRotation(Angle);            //设定转角
pBlkRef->setScaleFactors( XrefScale);   //设定放大比例

AcDbBlockTable *pBlockTable;
pDb->getSymbolTable(pBlockTable, AcDb::kForRead);

AcDbBlockTableRecord *pBlockTableRecord;
pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
pBlockTable->close();
AcDbObjectId newEntId;
pBlockTableRecord->appendAcDbEntity(newEntId, pBlkRef);
pBlockTableRecord->close();

如果我们在这里结束程序,我们将看到当前图形中source.dwg已经被作为图块插入.但是图块中没有source.dwg所定义的Attibute. 
因此我们还要做工作.后面的事情就简单了.

AcDbBlockTableRecord *pBlockDef;
acdbOpenObject(pBlockDef, blockId, AcDb::kForRead);

AcDbBlockTableRecordIterator *pIterator;
pBlockDef->newIterator(pIterator);
AcGePoint3d basePoint;
AcDbEntity *pEnt;
AcDbAttributeDefinition *pAttdef;
for (pIterator->start(); !pIterator->done(); pIterator->step())//将source.dwg中所有的Attibute进行遍历
{
    pIterator->getEntity(pEnt, AcDb::kForRead);
    pAttdef = AcDbAttributeDefinition::cast(pEnt);
    if (pAttdef != NULL && !pAttdef->isConstant()) {
        AcDbAttribute *pAtt = new AcDbAttribute();
        pAtt->setPropertiesFrom(pAttdef);
        pAtt->setInvisible(pAttdef->isInvisible());
        basePoint = pAttdef->position();
        basePoint += pBlkRef->position().asVector();
        pAtt->setPosition(basePoint);
        pAtt->setHeight(pAttdef->height());
        pAtt->setRotation(pAttdef->rotation());
        pAtt->setTag("Tag");
        pAtt->setFieldLength(25);
        char *pStr = pAttdef->tag();
        pAtt->setTag(pStr);
        acutDelString(pStr);
        pAtt->setFieldLength(pAttdef->fieldLength());
        pAtt->setTextString("-");

        AcDbObjectId attId;

        pBlkRef->appendAttribute(attId, pAtt);
        pAtt->close();
    }
    pEnt->close(); // use pEnt... pAttdef might be NULL
}
delete pIterator;


六、在ARX打开文件
在AutoCAD中打开图形，并且显示在图形窗口中，可以使用acedSyncFileOpen()函数。需要注意的是，这个函数只能在单文档模式中工作，

用户可以在AutoCAD“选项”对话框的“系统”选项卡中进行设置，或者在主函数中添加下面的语句：
acrxDynamicLinker->registerAppNotMDIAware(pkt);
具体的函数如下：

//加载模板文件
void LoadTemplate()
{
    char fname[50];
    strcpy(fname,"E:\\TMCAD\\TMCADtukuang\\A3.DWG");

#ifndef _ACAD2000
    Acad::ErrorStatuses; 
    es = acedSyncFileOpen(fname);
#else
    acDocManager->appContextOpenDocument(fname);
#endif
}

如果在多文档模式下，下面的方法可以在执行时弹出“选择文件”对话框，用户选择所要打开的文件后，在图形窗口中显示该图形。
void ZffOPENOpenDwg()
{
    // 使用“选择文件”对话框选择所要打开的文件
    acDocManager->openDocument();
}

下面的方法则打开指定位置的DWG文件。
void OpenDoc( void *pData)
{
    AcApDocument* pDoc = acDocManager->curDocument();
    if (acDocManager->isApplicationContext())
    {
        acDocManager->appContextOpenDocument((const char *)pData);
    }
    else
    {
        acutPrintf("\nERROR To Open Doc!\n");
    }
}

// This is command 'OPEN1'
void ZffOPENopen1()
{
    // 直接打开系统中存在的某个图形文件G:\AutoCAD图形\wen2.dwg
    static char pData[] = "G:\\AutoCAD图形\\wen2.dwg"; 
    acDocManager->executeInApplicationContext(OpenDoc, (void *)pData);
}
void ProgressBarTest()
{
    acutPrintf("启动进度条..........\n");
    acedSetStatusBarProgressMeter("测试进度条", 0, 100);
    for (int value=0; value<=100; value++) {
        Sleep(100);   // 暂停，释放控制权
        acedSetStatusBarProgressMeterPos(value);
    }
    acedRestoreStatusBar();

    return;
}

如何获得程序路径
struct resbuf rb;
char sTemp[1024],*str;
ads_getvar("acadprefix",&rb);
strcpy(sTemp,rb.resval.string);
acad_free(rb.resval.rstring);
str=strchr(sTemp,';');
*str='\0';
str=strrchr(sTemp,'\\');
*str='\0';

上段程序中,sTemp中存储了安装CAD的目录
AUTOCAD的系统变量存储了一些与安装有关的信息,虽然不多,在正常情况是够用的.与目录有关的主要有:
dwgprefix 当前dwg图形存储的目录
acadprefix   acad环境变量存储的目录
dwgname   当前dwg文件名
savefile 当前自动存储文件名

///从RGB得到cad颜色索引值 
int getNearestACI(COLORREF color)
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
        r =GetRValue(acirgb);
        g =GetGValue(acirgb);
        b =GetBValue(acirgb);

        dst = abs ( r-red) + abs ( g -green) + abs (b-blue);
        if ( dst < mindst ) {
            minndx = i;
            mindst = dst;
        }
    }
    return minndx;

}  

//功   能：从CAD的颜色得到RGB
COLORREF CGlobal::GetColorFromIndex(int colorIndex)
{
    if(colorIndex < 0 || colorIndex > 255)
    {
        ads_alert("传入的颜色号不在0~255之间!");
        return 0;
    }

    BYTE R, G, B;
#ifdef ARX_2002_dll
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

获取AcDbDimension里的属性信息
AcDbEntity *pEnt;
AcDbObjectId id;
AcGePoint3d ptPick;
ads_name eName;
if (acedEntSel ("Select a dimension: " , eName, asDblArray (ptPick)) != RTNORM )
return;
acdbGetObjectId (id, eName);
acdbOpenAcDbEntity (pEnt, id, AcDb::kForRead);
//----- Get the id of the block table record which owns the text entity
AcDbDimension *pDim =AcDbDimension::cast (pEnt);
if (pDim == NULL)
{
    pEnt->close ();
    return;
}

id =pDim->dimBlockId ();
pDim->close ();
AcDbBlockTableRecord *pr;
acdbOpenAcDbObject ((AcDbObject *&) pr, id, AcDb::kForRead);
//----- Iterating the block table record
AcDbBlockTableRecordIterator *pi; 
pr->newIterator (pi); 
while (!pi->done ()) 
{
    pi->getEntity (pEnt, AcDb::kForRead);
    if (pEnt->isKindOf (AcDbMText::desc ()))
    { 
        AcDbMText *pt = (AcDbMText *) pEnt;
        char *s = pt->contents ();
        acutPrintf (s);
        delete s;
    }
    pEnt->close();
    pi->step();
} 
pr->close();

插入定义的属性块

CString blockname;           ///获得要插入的块名 
CMyDraw my_draw;
my_draw.PoleBlockName(blockname,m_stat,m_type);
blockname=APPPATH+"blocks\\DG\\"+blockname+".DWG";
///////////////////////////////////////////////////////////////

//寻找是否已经定义 
AcDbDatabase *pCurDb = acdbHostApplicationServices()->workingDatabase(); 
AcDbBlockTable *pBlkTable; 
AcDbObjectId blockId ; 
pCurDb->getBlockTable(pBlkTable, AcDb::kForRead);

if(!pBlkTable->has(blockname))    //杳找块是否存在
{ 
    pBlkTable->close(); 
    AcDbDatabase *pDwg =new AcDbDatabase (Adesk::kFalse) ;

    pDwg->readDwgFile (blockname) ;       //读入块定义

    Acad::ErrorStatus es = pCurDb->insert (blockId, "blockname", pDwg);//, Adesk::kFalse) ; //

    delete pDwg ; 
    if ( es != Acad::eOk ) 
    { 
        acutPrintf ("\n插入块错误.") ; 
        return; 
    } 
} 
else 
	pBlkTable->getAt(blockname, blockId);

BeginEditorCommand(); 
AcGePoint3d pt; 
int rc=acedGetPoint(NULL,"\n选择插入点:",asDblArray(pt)); 
if (rc==RTCAN||rc==RTNONE) 
{ 
    acutPrintf("\n*取消了插入操作*"); 
    CancelEditorCommand(); 
    return; 
} 
//CompleteEditorCommand(); 
CancelEditorCommand();

double m_angle=0;
double m_scale=1;
//---- 设置插入点，旋转角度，比例等等 
AcDbBlockReference *pBlkRef =new AcDbBlockReference(AcGePoint3d(pt[0],pt[1],pt[2]),blockId) ; 
//pBlkRef->setBlockTableRecord (blockId) ; 
pBlkRef->setScaleFactors(AcGeScale3d(m_scale,m_scale,1)); 
//pBlkRef->setPosition(AcGePoint3d(pt[0],pt[1],pt[2])) ; 
pBlkRef->setRotation (m_angle) ;

////////////////////////////////////////////////////////////////////
pBlkRef->setLayer(NULL); //设置图层


////////////////////////////////////////////////////////获得模型空间块表记录 
AcDbBlockTable *pBlockTable ; 
acdbHostApplicationServices()->workingDatabase()->getBlockTable (pBlockTable, AcDb::kForRead) ; 
AcDbBlockTableRecord *pBlockTableRecord=new AcDbBlockTableRecord;
pBlockTable->getAt (ACDB_MODEL_SPACE, pBlockTableRecord,AcDb::kForWrite) ; 
pBlockTable->close () ;
pBlockTableRecord->appendAcDbEntity(pBlkRef);


//AcDbBlockTableRecord *pBlockDef;
//acdbOpenObject(pBlockDef, blockId, AcDb::kForRead);
//AcDbBlockTableRecordIterator *pInterator;
//pBlockDef->newIterator(pInterator);
//AcDbEntity *pEnt;
//AcDbAttributeDefinition *pAttdef;
//for(pInterator->start(); !pInterator->done(); pInterator->step())
//{
// pInterator->getEntity(pEnt, AcDb::kForRead);
// pAttdef = AcDbAttributeDefinition::cast(pEnt);
// if (pAttdef!=NULL && !pAttdef->isConstant())
// {
//   AcDbAttribute *pAtt = new AcDbAttribute();
//   pAtt->setPropertiesFrom(pAttdef);
//   pAtt->setInvisible(pAttdef->isInvisible());
//   pt = pAttdef->position();
//   pt +=pBlkRef->position().asVector();
//   pAtt->setPosition(pt);
//   pAtt->setHeight(pAttdef->height());
//   pAtt->setRotation(pAttdef->rotation());
//   pAtt->setTag("Tag");
//   pAtt->setFieldLength(25);
//   char *pStr = pAttdef->tag();
//   pAtt->setTag(pStr);
//   free(pStr);
//   pAtt->setTextString("Assigned Attribute Value");
//   AcDbObjectId attId;
//   pBlkRef->appendAttribute(attId, pAtt);
//   pAtt->close();
// }
// pEnt->close();
//}
//delete pInterator;
//pBlockDef->close();
//pBlkRef->close();


// 获得属性 
AcDbBlockTableRecord *pBlkDefRecord; 
acdbOpenObject(pBlkDefRecord, blockId, AcDb::kForRead); 

CStringArray ArrayAtt;
int i=0;
ArrayAtt.Add(m_bz);
ArrayAtt.Add(m_edit_num_val);         //////////////属性值
ArrayAtt.Add(m_stat);
ArrayAtt.Add(m_spec);
ArrayAtt.Add(m_type);

if(pBlkDefRecord->hasAttributeDefinitions()) 
{ 
    //AcDbObjectIterator* pBlkIterator = pBlkRef->attributeIterator(); //修改

    AcDbBlockTableRecordIterator *pIterator; 
    pBlkDefRecord->newIterator(pIterator);

    for(pIterator->start(); !pIterator->done();pIterator->step()) //遍历块的属性
    { 
        AcDbEntity *pEnt; 
        pIterator->getEntity(pEnt, AcDb::kForRead); 
        AcDbAttributeDefinition *pAttDef = AcDbAttributeDefinition::cast(pEnt);

        if(pAttDef != NULL && !pAttDef->isConstant()) 
        { 
            AcDbAttribute* pAtt = new AcDbAttribute(); 
            pAtt->setPropertiesFrom(pAttDef); 
            pAtt->setInvisible( pAttDef->isInvisible() );

            pAtt->setHorizontalMode(pAttDef->horizontalMode()); 
            pAtt->setVerticalMode(pAttDef->verticalMode()); 
            AcGePoint3d basePt = pAttDef->position(); 
            basePt[0]+=pt[0]; 
            basePt[1]+=pt[1]; 
            basePt[2]+=pt[2]; 
            pAtt->setPosition(basePt); 

            basePt = pAttDef->alignmentPoint(); 
            basePt[0]+=pt[0]; 
            basePt[1]+=pt[1]; 
            basePt[2]+=pt[2]; 
            pAtt->setAlignmentPoint(basePt);

            pAtt->setHeight(pAttDef->height()); 
            pAtt->setTextStyle(pAttDef->textStyle());

            pAtt->setTag(pAttDef->tag()); 
            pAtt->setTextString(ArrayAtt.GetAt(i++)); 
            pAtt->setFieldLength(25);


            AcDbObjectId attId;
            pBlkRef->appendAttribute(attId,pAtt); 
            pAtt->close(); 
            pEnt->close(); 
        } 
    } 
    delete pIterator; 
}//end if pBlkDefRecord->hasAttributeDefinitions

pBlkDefRecord->close();

pBlockTableRecord->close(); 
pBlkRef->close();


设置当前的layer和textstyle 

//  Function name    : SetCurTextStyle
//  Description        : 设置当前TextStyle
//  Return type        : Acad::ErrorStatus 
//  Argument         : const char* lpStyleName
//  Argument         : AcDbDatabase* pDb/* = NULL */ 
Acad::ErrorStatus SetCurTextStyle( const   char *  lpStyleName, AcDbDatabase *  pDb /*  = NULL  */ )
{
    AcDbDatabase *  pCurDb  =  pDb;
    if  (pCurDb  ==  NULL)
        pCurDb  =  acdbHostApplicationServices() -> workingDatabase();

    AcDbTextStyleTableRecordPointer spRecord(lpStyleName, pCurDb, AcDb::kForRead);
    Acad::ErrorStatus es  =  spRecord.openStatus();
    if  (es  ==  Acad::eOk)
    {
        es  =   pCurDb -> setTextstyle(spRecord -> objectId());
    } 
    return  es;
} 


//  Function name    : SetCurLayer
//  Description        : 设置当前层
//  Return type        : Acad::ErrorStatus 
//  Argument         : const char* lpLayerName
//  Argument         : AcDbDatabase* pDb/* = NULL */ 
Acad::ErrorStatus SetCurLayer( const   char *  lpLayerName, AcDbDatabase *  pDb /*  = NULL  */ )
{
    AcDbDatabase *  pCurDb  =  pDb;
    if  (pCurDb  ==  NULL)
        pCurDb  =  acdbHostApplicationServices() -> workingDatabase();

    AcDbLayerTableRecordPointer spRecord(lpLayerName, pCurDb, AcDb::kForRead);
    Acad::ErrorStatus es  =  spRecord.openStatus();
    if  (es  ==  Acad::eOk)
    {
        es  =   pCurDb -> setClayer(spRecord -> objectId());
    } 
    return  es;
} 



//功能描述:选择指定图层上的所有实体！

// ************************************************************************
// 函数名称:selectEntityInLayer
// 函数类型:Acad::ErrorStatus 
// 返回值:  正常：Acad::eOk
// 功能描述:选择指定图层上的所有实体！
// 函数作者:Darcy
// 创建日期:200X-XX-XX
// 参数列表:
// 变量名:nLayerName     变量类型:const char*           变量说明:(输入)图层名
// 变量名:nIDs           变量类型:AcDbObjectIdArray&    变量说明:(输出)图层中实体的对象标识符集合
// ************************************************************************ 
Acad::ErrorStatus selectEntityInLayer( const   char *  nLayerName,AcDbObjectIdArray &  nIDs)
{
    Acad::ErrorStatus es  =  Acad::eOk;

    ads_name ents;
    struct  resbuf  * rb;
    rb = acutNewRb(AcDb::kDxfLayerName);
    rb -> restype = 8 ;
    rb -> resval.rstring = ( char * )nLayerName;
    rb -> rbnext = NULL;
    acedSSGet( " X " ,NULL,NULL,rb,ents);
    long  entNums = 0 ;
    acedSSLength(ents, & entNums);
    if  (entNums  ==   0 )
        es  =  Acad::eInvalidInput;
    else 
    {
        for  ( long  a  =   0 ; a  <  entNums ; a  ++ )
        {
            AcDbObjectId  objId;
            ads_name      ent;
            acedSSName(ents,a,ent);
            acdbGetObjectId(objId, ent);
            nIDs.append(objId);
        } 
    } 
    acedSSFree(ents);
    acutRelRb(rb);

    return  es;
}

另一种实现

//************************************************************************
//函数名称:selectEntityInLayer
//函数类型:Acad::ErrorStatus 
//返回值:
//功能描述:选择指定层上的实体，得到其对象属性标识符！
//函数作者:Darcy
//创建日期:200X-XX-XX
//参数列表:
//变量名:nLayerName      变量类型:CString               变量说明:
//变量名:nIDs            变量类型:AcDbObjectIdArray&    变量说明:
//变量名:nModelSpace     变量类型:bool                  变量说明:
//************************************************************************
Acad::ErrorStatus    selectEntityInLayer(
        CString nLayerName,
        AcDbObjectIdArray& nIDs,
        bool nModelSpace
        )
{
    Acad::ErrorStatus es=Acad::eOk; 

    AcDbBlockTable*        pBlockTable=NULL;
    AcDbBlockTableRecord*  pSpaceRecord=NULL;
    if (acdbHostApplicationServices()->workingDatabase()==NULL)
        return Acad::eNoDatabase;
    if ((es = acdbHostApplicationServices()->workingDatabase()->getBlockTable(pBlockTable, AcDb::kForRead))==Acad::eOk)
    {
        char entryName[13];
        if (nModelSpace)
            strcpy(entryName,ACDB_MODEL_SPACE);
        else
            strcpy(entryName,ACDB_PAPER_SPACE);
        //Get the Model or Paper Space record and open it for read:  
        if ((es = pBlockTable->getAt((const char*)entryName, pSpaceRecord, AcDb::kForRead))==Acad::eOk)
        {
            AcDbBlockTableRecordIterator* pIter;
            if (pSpaceRecord->newIterator(pIter)==Acad::eOk)
            {
                for (pIter->start();!pIter->done();pIter->step())
                {
                    char *name=NULL;
                    AcDbEntity* pEntity;
                    if (pIter->getEntity(pEntity,AcDb::kForRead)==Acad::eOk)
                    {
                        name=pEntity->layer();
                        if (nLayerName.CompareNoCase(name)==0)
                            nIDs.append(pEntity->objectId());

                        pEntity->close();
                        acutDelString(name); 
                    }
                }
                delete pIter;
            }            
            pSpaceRecord->close();
        }   
        pBlockTable->close();
    }

    return es;
}




生成新组

//生成新组（sGroupName）
//追加数组中所有实体到该组中
//组名字 ,   Id数组
int createGroup(CString sGroupName,
        const AcDbObjectIdArray *idArr)
{
    AcDbGroup       *pGroup = new AcDbGroup((LPSTR)(LPCTSTR)sGroupName);
    AcDbObjectId     groupObjectId;
    AcDbDictionary  *pGroupDict = NULL;

    acdbHostApplicationServices()->workingDatabase()
        ->getGroupDictionary(pGroupDict, AcDb::kForWrite);
    pGroupDict->setAt(sGroupName, pGroup, groupObjectId);
    pGroupDict->close();
    pGroup->close();
    acdbOpenObject(pGroup, groupObjectId, AcDb::kForWrite);
    for (int i = 0; i < idArr->length(); i++)
    {
        groupObjectId = idArr->at(i);
        pGroup->append(groupObjectId);   
    }
    pGroup->close();
    return TRUE;
}


另一个

Acad::ErrorStatus CDrawFunction::createGroup(CString groupname,AcDbObjectIdArray IdArray)
{
    Acad::ErrorStatus es=Acad::eOk;
    AcDbDictionary *pGroupDict=NULL;
    AcDbGroup *pGroup=NULL;
    if((es=acdbHostApplicationServices()->workingDatabase()->getGroupDictionary(pGroupDict,AcDb::kForWrite))!=Acad::eOk)
    {
        return es;
    }
    AcDbObjectId groupId;
    es=pGroupDict->getAt(groupname,groupId);
    if(es==Acad::eInvalidKey)
    {
        acutPrintf("\n输入的词典名无效!");
        pGroupDict->close();
        return es;
    }
    else if(es==Acad::eKeyNotFound)
    {
        pGroup=new AcDbGroup("GroupDiscription");
        if((es=pGroupDict->setAt(groupname,pGroup,groupId))!=Acad::eOk)
        {
            pGroup->close();pGroupDict->close();return es;
        }
    }
    else if(es==Acad::eOk )
    {
        if((es=acdbOpenObject(pGroup,groupId,AcDb::kForWrite))!=Acad::eOk)
        {
            pGroupDict->close();return es;
        }
    }
    for(int i=0;i pGroup->append(IdArray[i]);
            pGroup->setSelectable(FALSE);
            pGroupDict->close();
            pGroup->close();
            return es;
            }


            旋转整个group

            void CMyDatabase::rotationGroup(CString strGroupName ,CReiPoint ptRotation,double rotationAngle) 
            {
            AcGePoint3d pt;
            AcDbDictionary *pGroupDict;
            acdbCurDwg()->getGroupDictionary(pGroupDict,AcDb::kForWrite);
            AcDbObjectId groupId;
            AcDbGroup *pGroup;
            pt.x=ptRotation.x;
            pt.y=ptRotation.y;
            pt.z=ptRotation.z;
            if(pGroupDict->getAt(strGroupName,groupId)==Acad::eOk)
                acdbOpenObject(pGroup,groupId,AcDb::kForWrite);
            else
            {
                pGroupDict->close();
                return;
            }
            pGroupDict->close();
            AcDbGroupIterator* pIter=pGroup->newIterator();
            AcDbEntity* pEnt;
            AcDbObjectId objId;
            pIter=pGroup->newIterator();
            for(;!pIter->done();pIter->next())
            {
                objId=pIter->objectId();
                acdbOpenAcDbEntity(pEnt,objId,AcDb::kForWrite);
                rotationEntity(pEnt,pt,rotationAngle);
                pEnt->close();
            }
            delete pIter;
            pGroup->close();
            }



新建一个图层

//==========================================================
功能：新建一个图层
参数：LayerName -- 图层名，LayerColor -- 颜色名
返回：图层ID
//==========================================================
AcDbObjectId CreateNewLayer(CString LayerName, AcCmColor LayerColor)
{
    // 获得当前图形数据库的符号表
    AcDbLayerTable *pLayerTable;
    acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pLayerTable,
            AcDb::kForWrite);
    // 生成新的图层表记录
    AcDbLayerTableRecord *pLayerTableRecord = new AcDbLayerTableRecord;
    pLayerTableRecord->setName(LayerName);        // 设置图层名
    pLayerTableRecord->setColor(LayerColor);    // 设置图层颜色
    AcDbObjectId layerId;
    pLayerTable->add(layerId,pLayerTableRecord);
    // 关闭图层表和图层表记录
    pLayerTable->close();
    pLayerTableRecord->close();
    return layerId;
}

//==========================================================
功能：在指定图层上新建一条直线
参数：见注释
返回：直线ID
//==========================================================
AcDbObjectId CreateLine( double x1,double y1,double z1,  // 起点坐标
        double x2,double y2,double z2,  // 终点坐标
        AcDbObjectId layer)                   // 直线所在图层
{
    AcGePoint3d StartPt(x1,y1,z1);    // 起点
    AcGePoint3d EndPt(x2,y2,z2);    // 终点
    AcDbLine *pLine = new AcDbLine(StartPt,EndPt);
    // 获得当前图形数据库的符号表
    AcDbBlockTable *pBlockTable;
    acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable,
            AcDb::kForRead);
    // 获得符号表中的模型空间块表记录指针，用于添加对象
    AcDbBlockTableRecord *pBlockTableRecord;
    pBlockTable->getAt(ACDB_MODEL_SPACE,pBlockTableRecord,AcDb::kForWrite);
    pBlockTable->close();
    // 将直线添加到模型空间块表记录中
    AcDbObjectId lineId;
    pLine->setLayer(layer,Adesk::kTrue);    // 设置直线所在图层
    pBlockTableRecord->appendAcDbEntity(lineId,pLine);
    // 关闭块表记录指针和直线指针
    pBlockTableRecord->close();
    pLine->close();
    // 返回直线ID号
    return lineId;
}

//已知一段弧的起点和终点以及其凸度,求其圆心 
int getCenter(ads_point startPoint,ads_point endPoint,double bulge,ads_point& center)
{
    if (bulge==0.0)
    {
        ads_point_set(startPoint,center);
        return 0;
    }
    ads_point startPt,endPt;
    if (bulge<0.0)
    {
        ads_point_set(endPoint,startPt);
        ads_point_set(startPoint,endPt);
        bulge=bulge*(-1.0);
    }
    else
    {
        ads_point_set(startPoint,startPt);
        ads_point_set(endPoint,endPt);
    }
    double distStartToEnd,distX,distY,radius;
    distStartToEnd=ads_distance(startPt,endPt);
    distX=distStartToEnd/2.0;
    distY=bulge*distX;
    radius=((distX*distX)+(distY*distY))/(2.0*distY);

    double tmpAng;
    ads_point tmpPt;

    tmpAng=ads_angle(startPt,endPt);
    ads_polar(startPt,tmpAng,distX,tmpPt);
    ads_polar(tmpPt,(tmpAng+(_PI/2.0)),(radius-distY),center);
    return 1;

}; 


//create hatch

AcDbObjectId CreateHatch(
        AcDbObjectId dbOId,
        char cLayer[], 
        char cPattern[] = "SOLID", 
        int nColor = 256, 
        double dAngle = 0.0, 
        double dScale = 1.0, 
        AcDbDatabase * pDbDatab = acdbHostApplicationServices()->workingDatabase())
{
    AcCmColor CmC;
    AcDbObjectId DbOId;
    AcDbObjectIdArray DbOIdA(0, 2);
    AcDbBlockTable * pDbBT;
    AcDbBlockTableRecord * pDbBTR;
    AcGeVector3d normal(0.0, 0.0, 1.0);

    DbOIdA.append(dbOId);

    AcDbHatch* pDbHat = new AcDbHatch();
    pDbHat->setDatabaseDefaults();
    pDbHat->setAssociative(Adesk::kTrue); // BUG: doesn't do squat! have to set the reactor yourself to get associativity!
    pDbHat->appendLoop(AcDbHatch::kExternal, DbOIdA);
    pDbHat->setPatternScale(dScale);
    pDbHat->setPatternAngle(dAngle);
    pDbHat->setPattern(AcDbHatch::kPreDefined, cPattern);
    pDbHat->setNormal(normal);
    pDbHat->evaluateHatch(); // crucial call or nothing gets displayed!
    pDbDatab->getSymbolTable(pDbBT, AcDb::kForRead);
    pDbBT->getAt(ACDB_MODEL_SPACE, pDbBTR, AcDb::kForWrite);
    pDbBTR->appendAcDbEntity(DbOId, pDbHat);
    pDbHat->setLayer(cLayer);
    CmC.setColorIndex(nColor);
    ((AcDbEntity *)pDbHat)->setColor(CmC);

    pDbBT->close();
    pDbBTR->close();
    pDbHat->close();
    return DbOId;
}

//前阵困扰我的问题解决了，现在共享给大家：
//************************************************************************
//函数名称:getPointAtDistInGeCurve
//函数类型:Acad::ErrorStatus 
//返回值:
//功能描述:返回曲线上距起点某距离值处的点。
//函数作者:Darcy
//创建日期:2003-XX-XX
//参数列表:
//变量名:pGeCurve 变量类型:const AcGeCurve3d * 变量说明:
//变量名:dist 变量类型:double 变量说明:
//变量名:point 变量类型:AcGePoint3d& 变量说明:
//备 注:
//************************************************************************
Acad::ErrorStatus getPointAtDistInGeCurve(const AcGeCurve3d * pGeCurve,double dist,AcGePoint3d& point)
{
    Acad::ErrorStatus es = Acad::eOk;

    if ( pGeCurve != NULL )
    {
        AcGePoint3d spt;
        double pa=0.,datumParam=0.;
        //距离从起点起算！
        Adesk::Boolean posParamDir=Adesk::kTrue;
        pGeCurve->hasStartPoint(spt); 
        datumParam = pGeCurve->paramOf(spt);;
        pa = pGeCurve->paramAtLength(datumParam, dist, posParamDir);
        point=pGeCurve->evalPoint(pa);
    }
    else
        es = Acad::eInvalidInput;

    return es;
} 


//判断点是否在圆弧上
/*****************************************************/
/* 判断某点是否在圆弧上
   firstPt 圆弧起点
   secondPt 圆弧终点 
   radius 半径
   direct 偏向( >=0__左偏 <0__右偏 )
   More (More<0__小圆弧,More>0__大圆弧)
   thePt 判断点
   返回：TRUE__在圆弧上 FALSE__不在圆弧上
   */
BOOL IsAtArc(CAD_POINT firstPt,CAD_POINT secondPt, double radius,double direct,int More,CAD_POINT thePt)
{
    CAD_POINT centerPt,sectionPt;
    CAD_POINT startPt,endPt;
    double startAngle,endAngle,chordAngle,vertAngle;
    double intLine,chordLine;
    /* centerPt 圆弧圆心
       sectionPt 弦线中心点
       startAngle 圆弧起点切线角度（弧度）
       endAngle 圆弧终点切线角度（弧度）
       chordAngle 弦线角度（弧度）
       vertAngle 与弦线垂直的垂线角度（弧度）
       intLine 弦线中心至圆心距离
       chordLine 弦线长度
       */ 
    sectionPt.x = (firstPt.x + secondPt.x)/2;
    sectionPt.y = (firstPt.y + secondPt.y)/2;

    chordLine = sqrt( pow( (secondPt.x-firstPt.x),2 ) + pow( (secondPt.y-firstPt.y),2 ) );
    intLine = sqrt((radius*radius-chordLine*chordLine/4) );

    chordAngle = ads_angle(asDblArray(firstPt),asDblArray(secondPt)); //弦线的角度

    if(direct>=0)//左偏
    {
        startPt=firstPt;
        endPt=secondPt;
        vertAngle=chordAngle+Pai/2;
    }
    else if(direct<0)//右偏
    {
        startPt=secondPt;
        endPt=firstPt;
        vertAngle=chordAngle-Pai/2;
    }

    if(More<=0)//小圆弧
    {
        centerPt.x=sectionPt.x+intLine*cos(vertAngle);
        centerPt.y=sectionPt.y+intLine*sin(vertAngle);
    }
    else//大圆弧
    {
        centerPt.x=sectionPt.x-intLine*cos(vertAngle);
        centerPt.y=sectionPt.y-intLine*sin(vertAngle);
    }

    if(fabs(centerPt.distanceTo(thePt)-radius)>1.0E-8)
        return FALSE;
    startAngle = ads_angle(asDblArray(centerPt),asDblArray(startPt)); 
    endAngle = ads_angle(asDblArray(centerPt),asDblArray(endPt)); 

    AcDbArc *pArc=new AcDbArc(centerPt,radius,startAngle,endAngle);
    AcDbLine *pLine=new AcDbLine(centerPt,thePt);
    AcGePoint3dArray Points;
    pLine->intersectWith(pArc,AcDb::kOnBothOperands,Points);
    if(Points.isEmpty()) return FALSE;

    return TRUE;
}


//相object加xdata

void affixXdata(char *appName, char *xData, AcDbObject *pObj)
{
    //向AcDbObject添加扩展数据Xdata
    struct resbuf *pRb, *pTemp;

    acdbRegApp(appName);
    pRb = acutNewRb(AcDb::kDxfRegAppName);
    pTemp = pRb;
    pTemp->resval.rstring = new char[strlen(appName)+1];
    strcpy(pTemp->resval.rstring, appName);

    pTemp->rbnext = acutNewRb(AcDb::kDxfXdAsciiString);
    pTemp = pTemp->rbnext;
    pTemp->resval.rstring = new char[strlen(xData)+1];
    strcpy(pTemp->resval.rstring, xData);

    pObj->setXData(pRb);
    acutRelRb(pRb);
}



//添加扩展数据
//实体添加扩展数据(字符串)
bool AddXData(CString appName, AcDbObjectId entId,CString data)
{
    //open entity for read
    AcDbEntity*pEnt;
    Acad::ErrorStatus es=acdbOpenAcDbEntity(pEnt,entId,AcDb::kForRead);
    if(es!=Acad::eOk)
    {
        ads_printf("error in open entity\n");
        return false;
    }
    //get XData buffer
    struct resbuf*pRb,*pTemp;
    pRb=pEnt->xData(appName);
    if(pRb!=NULL)//have XData
    {
        //pTemp移到表尾
        pTemp=pRb;
        for(pTemp=pRb;pTemp->rbnext!=NULL;pTemp=pTemp->rbnext){;}
    }
    else//NOT have XData
    {
        //create new xData
        ads_regapp(appName);
        pRb=ads_newrb(AcDb::kDxfRegAppName);
        pRb->resval.rstring=(char*)malloc(appName.GetLength()+1);
        strcpy(pRb->resval.rstring,appName);
        pTemp=pRb;
    }
    //fill xData string
    pTemp->rbnext=ads_newrb(AcDb::kDxfXdAsciiString);
    pTemp=pTemp->rbnext;
    pTemp->resval.rstring=(char*)malloc(data.GetLength()+1);
    strcpy(pTemp->resval.rstring,data);
    //add xData
    es=pEnt->upgradeOpen();
    if(es!=Acad::eOk)
    {
        ads_printf("\nError occur in updateOpen.");
        pEnt->close();
        ads_relrb(pRb);
        return false;
    }
    es=pEnt->setXData(pRb);
    if(es!=Acad::eOk)
    {
        ads_printf("\nError occur in setXData.");
        pEnt->close();
        ads_relrb(pRb);
        return false;
    }
    //
    pEnt->close();
    ads_relrb(pRb);
    return true;
}
