#include "StdAfx.h"
#include "ArxCookbook.h"
#include "ArxUtility.h"
#include "SSGetFilter.h"

namespace arxcookbook
{

	void SquareOfLines(AcDbVoidPtrArray &ents, double size);
/* �����飺
 *   1. ��ʾ�û�����Ҫ�����Ŀ������
 *   2. �򿪿��BlockTable����newһ���µĿ���¼��BlockTableRecord�����������BlockTableRecord�����ԣ���Ҫ�����֣�
 *   3. �������BlockTableRecord��ӵ�BlockTable�У�ͬʱ��¼��ID��ObjectID��
 *   4. ѡ��ʵ�壨����㴴����һЩʵ�壩��ӵ�BlockTableRecord��
 *   5. �ر�BlockTableRecord �� BlockTable��
 *   ����������˿�Ĵ��������Ϳ�����CAD����������Ŀ�����ˡ�
 */
void create_block()
{
	TCHAR blockName[133];    // buffer for our block name
	AcDbVoidPtrArray ents;   // buffer for holding entities 
	AcDbObjectId newBlockId; 

	//get an instance of the database
	AcDbDatabase *pDatabase = acdbHostApplicationServices()->workingDatabase();

	//get the name of our new block
	if(acedGetString(1,_T("\nΪ�鴴������: "),blockName) != Acad::eNormal){
		acutPrintf(_T("\nError: ��ȡ������ʧ�� "));
		return;
	}

	//validate the block name
	if(acdbSymUtil()->validateSymbolName(blockName,false) != Acad::eOk){
		acutPrintf(_T("\nError: ���� %s ������CAD�Ĺ淶"),blockName);
		return;
	}

	//Open the Block Table using a smart Pointer
	AcDbBlockTablePointer pBlockTable(pDatabase->blockTableId(),AcDb::kForWrite);
	if(pBlockTable.openStatus() != eOk){
		acutPrintf(_T("\nError: �򿪿��ʧ��"));
		return;
	}

	//test if our block name already exists
	if(pBlockTable->has(blockName)){
		acutPrintf(_T("\nError: ������ (%s) �Ѿ�����"),blockName);
		return;
	}

	//create our block using a smart pointer
	AcDbBlockTableRecordPointer pNewBlockTableRecord;
	pNewBlockTableRecord.create();
	pNewBlockTableRecord->setName(blockName);

	//add the new block to the Block Table
	if(pBlockTable->add(newBlockId,pNewBlockTableRecord)!= Acad::eOk){
		acutPrintf(_T("\nError: ��ӿ� (%s) ʧ��"), blockName);
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
		acutPrintf(_T("\nError: �򿪵�ǰ�ռ�ʧ��"));
		return;
	}

	//add the insert to the current space;
	if(pSpace->appendAcDbEntity(pNewBlockReference) != eOk)
	{
		acutPrintf(_T("\nError: ��ӿ����ʧ��"));
		return;
	}

	//smart pointer do their mojo here.
	//����ָ�븺��ر����ݿ���αꡢ�ͷ���Դ��
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
	acedEntSel(_T("\nѡ��ʵ��"), ent, pt);
	acedRedraw(ent, 3);

}

void ssget_test_demo()
{
	MySSGetFilter myFilter;
	addSSgetFilterInputContextReactor(curDoc(), &myFilter);

	removeSSgetFilterInputContextReactor(curDoc(), &myFilter);
}

}

