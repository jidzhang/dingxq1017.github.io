#pragma once

typedef AcArray<AcDbIntArray, AcArrayObjectCopyReallocator<AcDbIntArray> > 
AcDbArrayIntArray;

/*********************************************************
 * 选择集过滤器
 * 目的有三：
 *     1. 选中实体不仅高亮显示，还以一个颜色显示（默认品红）
 *     2. 可以按组高亮显示
 *     3. 可以指定过滤条件，主要是图层
 * 并且实现选择结束后恢复本来的颜色
 ********************************************************/
class MySSGetFilter : public AcEdSSGetFilter
{
public:
	//init
	MySSGetFilter(int highlight_color_index=6, bool by_group=false) : m_color_index(highlight_color_index), m_select_by_group(by_group)
	{ }

	virtual void
		ssgetAddFilter(
		int ssgetFlags,
		AcEdSelectionSetService &service,
		const AcDbObjectIdArray& selectionSet,
		const AcDbObjectIdArray& subSelectionSet);

	virtual void
		ssgetRemoveFilter(
		int ssgetFlags,
		AcEdSelectionSetService &service,
		const AcDbObjectIdArray& selectionSet,
		const AcDbObjectIdArray& subSelectionSet,
		AcDbIntArray& removeIndexes,
		AcDbArrayIntArray& removeSubentIndexes);

	//兼容CAD2008之前的ARX
	virtual void
		ssgetRemoveFilter(
		int ssgetFlags,
		AcEdSelectionSetService &service,
		const AcDbObjectIdArray& selectionSet,
		const AcDbObjectIdArray& subSelectionSet,
		AcDbIntArray& removeIndexes);

	virtual void
		ssgetAddFailed(
		int ssgetFlags,
		int ssgetMode,
		AcEdSelectionSetService &service,
		const AcDbObjectIdArray& selectionSet,
		resbuf *rbPoints)
	{  }

	virtual void
		ssgetRemoveFailed(
		int ssgetFlags,
		int ssgetMode,
		AcEdSelectionSetService &service,
		const AcDbObjectIdArray& selectionSet,
		resbuf *rbPoints,
		AcDbIntArray& removeIndexes,
		AcDbArrayIntArray& removeSubentIndexes)
	{  }

	virtual void
		endSSGet(
		Acad::PromptStatus returnStatus,
		int ssgetFlags,
		AcEdSelectionSetService &service,
		const AcDbObjectIdArray& selectionSet);

	virtual void
		endEntsel(
		Acad::PromptStatus       returnStatus,
		const AcDbObjectId&      pickedEntity,
		const AcGePoint3d&       pickedPoint,
		AcEdSelectionSetService& service);

	virtual bool canBeSelected(AcDbObjectId id) const;
	//子类务必实现这个函数，这个函数决定了本Filter的特性
	virtual bool canBeSelected(const AcDbEntity *  pEnt) const;
private:
	void high_light(AcDbObjectId id);
	void un_high_light(AcDbObjectId id);
	void reset_color();
private:
	int  m_color_index;
	bool m_select_by_group;
	//std::set<AcDbObjectId> m_selection_set;
	std::map<AcDbObjectId, int> m_obj_origin_color_map;
};

//设置颜色，同时返回原来颜色值
int set_color_index(AcDbObjectId id, int color_index);
int set_color_index(AcDbEntity * pEnt, int color_index);