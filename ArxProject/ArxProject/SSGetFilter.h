#pragma once

typedef AcArray<AcDbIntArray, AcArrayObjectCopyReallocator<AcDbIntArray> > 
AcDbArrayIntArray;

/*********************************************************
 * ѡ�񼯹�����
 * Ŀ��������
 *     1. ѡ��ʵ�岻��������ʾ������һ����ɫ��ʾ��Ĭ��Ʒ�죩
 *     2. ���԰��������ʾ
 *     3. ����ָ��������������Ҫ��ͼ��
 * ����ʵ��ѡ�������ָ���������ɫ
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

	//����CAD2008֮ǰ��ARX
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
	//�������ʵ�����������������������˱�Filter������
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

//������ɫ��ͬʱ����ԭ����ɫֵ
int set_color_index(AcDbObjectId id, int color_index);
int set_color_index(AcDbEntity * pEnt, int color_index);