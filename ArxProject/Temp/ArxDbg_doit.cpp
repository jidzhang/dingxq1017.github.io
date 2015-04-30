static void ArxDbg_doit(void)
  {
    TCHAR blockName[133];    // buffer for our block name
    AcDbVoidPtrArray ents;   // buffer for holding entities 
    AcDbObjectId newBlockId; 

    //get an instance of the database
    AcDbDatabase *pDatabase = acdbHostApplicationServices()->workingDatabase();

    //get the name of our new block
    if(acedGetString(1,_T("\nPlease Enter name of new Block: "),blockName) != Acad::eNormal){
      acutPrintf(_T("\nError: Get Block Name Failed: "));
      return;
    }

    //validate the block name
    if(acdbSymUtil()->validateSymbolName(blockName,false) != Acad::eOk){
      acutPrintf(_T("\nError: Symbol %s is invalid: "),blockName);
      return;
    }

    //Open the Block Table using a smart Pointer
    AcDbBlockTablePointer pBlockTable(pDatabase->blockTableId(),AcDb::kForWrite);
    if(pBlockTable.openStatus() != eOk){
      acutPrintf(_T("\nError: Failed to open Block Table: "));
      return;
    }

    //test if our block name already exists
    if(pBlockTable->has(blockName)){
      acutPrintf(_T("\nError: Block Name (%s) Already Exists: "),blockName);
      return;
    }

    //create our block using a smart pointer
    AcDbBlockTableRecordPointer pNewBlockTableRecord;
    pNewBlockTableRecord.create();
    pNewBlockTableRecord->setName(blockName);

    //add the new block to the Block Table
    if(pBlockTable->add(newBlockId,pNewBlockTableRecord)!= Acad::eOk){
      acutPrintf(_T("\nError: Failed to add block (%s): "),blockName);
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
      acutPrintf(_T("\nError: Failed to Current Space: "));
      return;
    }

    //add the insert to the current space;
    if(pSpace->appendAcDbEntity(pNewBlockReference) != eOk)
    {
      acutPrintf(_T("\nError: Failed to add Reference: "));
      return;
    }

    //smart pointer do their mojo here.
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