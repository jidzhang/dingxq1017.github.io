---
layout: post
title: "(转载)ObjectARX智能指针"
category: 
---

ObjectArx 利用 C++ 栈上变量作用域/生命周期约定，重载 operator->() 操作符、虚析构函数、禁止对象拷贝复制等语义定义了大量智能指针。
这些指针使用方便安全，大大简化了代码的书写。
这些类的实例在销毁时，自动关闭 Arx 实体/对象，完全不用担心忘记关闭实体而导致的抛出异常问题。

针对AcDbObject：

	class AcDbObjectPointer ;

	typedef AcDbObjectPointer<AcDbDictionary> AcDbDictionaryPointer;
	typedef AcDbObjectPointer<AcDbEntity>     AcDbEntityPointer;

针对符号表：

	class AcDbSymbolTablePointer;

	typedef AcDbSymbolTablePointer<AcDbBlockTable>     AcDbBlockTablePointer;
	typedef AcDbSymbolTablePointer<AcDbDimStyleTable>  AcDbDimStyleTablePointer;
	typedef AcDbSymbolTablePointer<AcDbLayerTable>     AcDbLayerTablePointer;
	typedef AcDbSymbolTablePointer<AcDbLinetypeTable>  AcDbLinetypeTablePointer;
	typedef AcDbSymbolTablePointer<AcDbRegAppTable>    AcDbRegAppTablePointer;
	typedef AcDbSymbolTablePointer<AcDbTextStyleTable> AcDbTextStyleTablePointer;
	typedef AcDbSymbolTablePointer<AcDbUCSTable>       AcDbUCSTablePointer;
	typedef AcDbSymbolTablePointer<AcDbViewTable>      AcDbViewTablePointer;
	typedef AcDbSymbolTablePointer<AcDbViewportTable>  AcDbViewportTablePointer;

针对符号表记录：

	class AcDbSymbolTableRecordPointer;

	typedef AcDbSymbolTableRecordPointer<AcDbBlockTableRecord>     AcDbBlockTableRecordPointer;
	typedef AcDbSymbolTableRecordPointer<AcDbDimStyleTableRecord>  AcDbDimStyleTableRecordPointer;
	typedef AcDbSymbolTableRecordPointer<AcDbLayerTableRecord>     AcDbLayerTableRecordPointer;
	typedef AcDbSymbolTableRecordPointer<AcDbLinetypeTableRecord>  AcDbLinetypeTableRecordPointer;
	typedef AcDbSymbolTableRecordPointer<AcDbRegAppTableRecord>    AcDbRegAppTableRecordPointer;
	typedef AcDbSymbolTableRecordPointer<AcDbTextStyleTableRecord> AcDbTextStyleTableRecordPointer;
	typedef AcDbSymbolTableRecordPointer<AcDbUCSTableRecord>       AcDbUCSTableRecordPointer;
	typedef AcDbSymbolTableRecordPointer<AcDbViewTableRecord>      AcDbViewTableRecordPointer;
	typedef AcDbSymbolTableRecordPointer<AcDbViewportTableRecord>  AcDbViewportTableRecordPointer;
 
下面的例子目的是在AcDbEntity添加扩展字典数据，使用了AcDbEntityPointer 、AcDbDictionaryPointer 、AcDbObjectPointer<AcDbXrecord>等智能指针。

{% lighlight cpp %}
bool SetEntityDictFromRbChain(AcDbObjectId entId,ACHAR *strDictName,resbuf* pRbValue)
{
	AcDbEntityPointer pObj(entId,AcDb::kForWrite);
	if (pObj.openStatus()!=Acad::eOk ) 
		return false;

	AcDbObjectId extDictId = pObj->extensionDictionary();
	if (extDictId == AcDbObjectId::kNull)
	{
		Acad::ErrorStatus es = pObj->createExtensionDictionary();
		if (es != Acad::eOk)
			return false;
		if ((extDictId = pObj->extensionDictionary()) == AcDbObjectId::kNull)
			return false;
	}

	AcDbDictionaryPointer pDict(extDictId,AcDb::kForRead);
	if(pDict.openStatus() != Acad::eOk)
		return false;

	AcDbObjectId xRecId;
	if(pDict->getAt(strDictName,xRecId) != Acad::eOk)
	{
		AcDbXrecord* pXrec = new AcDbXrecord();
		//pXrec 可以使用 scoped_ptr
		pDict->upgradeOpen();
		if(pDict->setAt(strDictName,pXrec,xRecId) != Acad::eOk)
		{
			delete pXrec;
			return false;
		}
		pXrec->close();
	}

	AcDbObjectPointer<AcDbXrecord> pXrcord(xRecId,AcDb::kForWrite);
	if(pXrcord.openStatus() != Acad::eOk) 
		return false; 

	if(pXrcord->setFromRbChain(*pRbValue) != Acad::eOk) 
		return false;

	acutRelRb(pRbValue);
	return true;
}
{% endhighlight %}
