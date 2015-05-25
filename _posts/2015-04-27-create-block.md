---
layout: post
title: "创建块和块参照"
category: 
---

在讲解如何创建块/块参照前，先普及下AutoCAD图形数据库的基本知识。

通常我们打开或新建的一张dwg图纸就是一个图形数据库（Graphic Database），这是一个实实在在的数据库，由表（Table）和记录（TableRecord）组成。大家用AutoCAD画图，除了画线画实体外，还几乎必须用到图层表、标注样式表和线型表等，这就是在直接操作AutoCAD图形数据库的各种表。

具体来说，AutoCAD图形数据库由 9 个符号表和一个命名字典构成，每个表（table）下面是一条一条的记录（record）。这 9 个表分别是：

    
|符号表名|符号表功能
|-------|---------
|AcDbViewTable(视图表) | 适用于 AcDbViewTableRecord 类，表示在数据库中存储的视图。视图与CAD的"VIEW"命令相关联
|AcDbViewportTable(视口表) | 适用于 AcDbViewportTableRecord 类，表示在CAD中当前系统变量TILEMODE的值为1时的视口设置。视口配置由CAD的 VPORTS 命令创建。不要和 MVIEW 命令混淆，当系统变量 TILEMODE 的值为0是，该命令创建视口实体。
|AcDbLinetypeTable(线性表) | 适用于 AcDbLinetypeTableRecord 类，表示图形数据库中的线性。
|AcDbLayerTable(层表)     | 适用于 AcDbLayerTableRecord类，表示图层。
|AcDbTextStyleTable(文字样式表) | 适用于 AcDbTextStyleTable 类，表示文字样式。
|AcDbUCSTable(用户坐标系表) | 适用于 AcDbUCSTableRecord 类，表示图形数据库中的存储的用户坐标系。
|AcDbRegAppTable(应用程序名注册表) | 适用于 AcDbRegAppTableRecord 类，表示为图形数据库中对象的扩展实体数据而注册的应用程序名。
|AcDbDimStyleTable(尺寸标注样式表) | 适用于 AcDbDimStyleTableRecord 类，表示图形数据库中的尺寸标注样式。
|AcDbBlockTable(块表) | 适用于 AcDbDimStyleTableRecord 类，表示图形数据库中定义的块。此表含有两个非常重要的记录：模型空间和图纸空间。所有的实体(可见对象)均防御块表AcDbBlockTable中。


通常进行的绘图操作，从编程角度来说是这样一个流程：

    打开表 --> 从表里找到记录或创建新的纪录 --> 往记录里面添加实体或者修改里面的实体

新建一个实体的过程基本都是这样一个流程：

    创建一个新实体
    以读方式打开块表
    以写方式打开块表记录，并在其中查找 ACDB_MODEL_SPACE 或 ACDB_PAPER_SPACE 或一个布局
    关闭块表
    把实体添加到块表记录
    关闭块表记录
    关闭实体对象

今天要做的演示是：选择实体后创建一个命名块，然后利用这个块插入块参照。

程序流程如下：

1. 选择实体，得到一个选择集（SelectionSet）
2. new 一个块表记录指针（BlockTableRecord * pBlockRecord)
3. 提示用户输入块的名字，并赋给上面的块表记录
4. 把 块表记录 添加到块表（BlockTable）中，同时记下与块表记录关联的 ObjectID
5. 利用上面块表记录的ID（ObjectID），向块表记录中添加选择集中的实体（Entity）
6. 利用块表记录的ID随意创建一个块参照（BlockReference）
7. 提示用户输入块参照的插入位置，随后插入块参照

简化的源程序如下，完整程序请到我们的 [Github 主页](https://github.com/dingxq1017/dingxq1017.github.io/tree/master/ArxProject)上获取。

{% highlight cpp %}
// 随便创建一些实体
static void SquareOfLines(AcDbVoidPtrArray &ents, double size)
{
    AcGePoint3dArray pts;
    pts.append(AcGePoint3d(-size, -size, 0));
    pts.append(AcGePoint3d(size, -size, 0));
    pts.append(AcGePoint3d(size, size, 0));
    pts.append(AcGePoint3d(-size, size, 0));

    size_t max = pts.length()-1;
    for (size_t i = 0; i <= max; i++) {
        int j = (i == max ? 0 : i + 1);
        ents.append(new AcDbLine(pts[i],pts[j]));
    }
}

/* 创建块：
 *   1. 提示用户输入要创建的块的名字
 *   2. 打开块表（BlockTable），new一个新的块表记录（BlockTableRecord），设置这个BlockTableRecord的属性（主要是名字）
 *   3. 把上面的BlockTableRecord添加到BlockTable中，同时记录下ID（ObjectID）
 *   4. 选择实体（或随便创建出一些实体）添加到BlockTableRecord中
 *   5. 关闭BlockTableRecord 和 BlockTable。
 *   这样就完成了块的创建，随后就可以在CAD里插入这个块的块参照了。
 */
void create_block()
{
    TCHAR blockName[133];    // buffer for our block name
    AcDbVoidPtrArray ents;   // buffer for holding entities 
    AcDbObjectId newBlockId; 

    //get an instance of the database
    AcDbDatabase *pDatabase = acdbHostApplicationServices()->workingDatabase();

    //get the name of our new block
    if(acedGetString(1,_T("\n为块创建名字: "),blockName) != Acad::eNormal){
        acutPrintf(_T("\nError: 获取块名字失败 "));
        return;
    }

    //validate the block name
    if(acdbSymUtil()->validateSymbolName(blockName,false) != Acad::eOk){
        acutPrintf(_T("\nError: 符号 %s 不符合CAD的规范"),blockName);
        return;
    }

    //Open the Block Table using a smart Pointer
    AcDbBlockTablePointer pBlockTable(pDatabase->blockTableId(),AcDb::kForWrite);
    if(pBlockTable.openStatus() != eOk){
        acutPrintf(_T("\nError: 打开块表失败"));
        return;
    }

    //test if our block name already exists
    if(pBlockTable->has(blockName)){
        acutPrintf(_T("\nError: 块名字 (%s) 已经存在"),blockName);
        return;
    }

    //create our block using a smart pointer
    AcDbBlockTableRecordPointer pNewBlockTableRecord;
    pNewBlockTableRecord.create();
    pNewBlockTableRecord->setName(blockName);

    //add the new block to the Block Table
    if(pBlockTable->add(newBlockId,pNewBlockTableRecord)!= Acad::eOk){
        acutPrintf(_T("\nError: 添加块 (%s) 失败"), blockName);
        return;
    }
    //make a few lines
    SquareOfLines(ents,5);

    //add the lines to our block
    for(size_t i = 0 ; i < ents.length();i++)
    {
        AcDbEntity *pTmp = (AcDbEntity*)ents[i];
        if(pNewBlockTableRecord->appendAcDbEntity(pTmp) != eOk)
            delete pTmp;
        else
            pTmp->close();
    }

    //create a reference of our block
    AcDbObjectPointer<AcDbBlockReference> pNewBlockReference;
    pNewBlockReference.create();
    pNewBlockReference->setPosition(AcGePoint3d::kOrigin);
    pNewBlockReference->setBlockTableRecord(newBlockId);

    //open up the btr of the current space
    AcDbBlockTableRecordPointer pSpace(pDatabase->currentSpaceId(),AcDb::kForWrite);
    if(pSpace.openStatus() != eOk)
    {
        acutPrintf(_T("\nError: 打开当前空间失败"));
        return;
    }

    //add the insert to the current space;
    if(pSpace->appendAcDbEntity(pNewBlockReference) != eOk)
    {
        acutPrintf(_T("\nError: 添加块参照失败"));
        return;
    }

    //smart pointer do their mojo here.
    //智能指针负责关闭数据库的游标、释放资源等
}

{% endhighlight %}

    
    