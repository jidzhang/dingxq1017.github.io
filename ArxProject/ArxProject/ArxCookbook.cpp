#include "StdAfx.h"
#include "ArxCookbook.h"
#include "ArxUtility.h"

namespace arxcookbook
{

	void SquareOfLines(AcDbVoidPtrArray &ents, double size);
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
	for(int i = 0 ; i < ents.length();i++)
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

static void SquareOfLines(AcDbVoidPtrArray &ents, double size)
{
	AcGePoint3dArray pts;
	pts.append(AcGePoint3d(-size, -size, 0));
	pts.append(AcGePoint3d(size, -size, 0));
	pts.append(AcGePoint3d(size, size, 0));
	pts.append(AcGePoint3d(-size, size, 0));

	size_t max = pts.length()-1;

	for (size_t i = 0; i <= max; i++)
	{
		int j = (i == max ? 0 : i + 1);
		ents.append(new AcDbLine(pts[i],pts[j]));
	}
}

void user_define_highlight()
{
	ads_name ent;
	ads_point pt;
	acedEntSel(_T("\n选择实体"), ent, pt);
	acedRedraw(ent, 3);

}
}

