#include "StdAfx.h"
#include "SSGetFilter.h"

void MySSGetFilter::ssgetAddFilter(int ssgetFlags, AcEdSelectionSetService &service, const AcDbObjectIdArray& selectionSet, const AcDbObjectIdArray& subSelectionSet)
{
	if (m_color_index < 0)
		return;
	for (int i = 0; i < subSelectionSet.length(); ++i) {
		if (!canBeSelected(subSelectionSet[i])) {
			service.remove(i);
			continue;
		}
		if (!m_select_by_group) {
			high_light(subSelectionSet[i]);
		} else {

		}
	}
}

void MySSGetFilter::ssgetRemoveFilter(int ssgetFlags, AcEdSelectionSetService &service, const AcDbObjectIdArray& selectionSet, const AcDbObjectIdArray& subSelectionSet, AcDbIntArray& removeIndexes, AcDbArrayIntArray& removeSubentIndexes)
{
	ssgetRemoveFilter(ssgetFlags, service, selectionSet, subSelectionSet, removeIndexes);
}

void MySSGetFilter::ssgetRemoveFilter(int ssgetFlags, AcEdSelectionSetService &service, const AcDbObjectIdArray& selectionSet, const AcDbObjectIdArray& subSelectionSet, AcDbIntArray& removeIndexes)
{
	if (m_color_index < 0)
		return;
	for (int i = 0; i < subSelectionSet.length(); ++i) {
		if (!canBeSelected(subSelectionSet[i])) {
			service.remove(i);
			continue;
		}
		if (!m_select_by_group) {
			un_high_light(subSelectionSet[i]);
		} else {

		}
	}
}

void MySSGetFilter::endSSGet(Acad::PromptStatus returnStatus, int ssgetFlags, AcEdSelectionSetService &service, const AcDbObjectIdArray& selectionSet)
{
	if (m_color_index < 0)
		return;
	reset_color();
}

void MySSGetFilter::endEntsel(Acad::PromptStatus returnStatus, const AcDbObjectId& pickedEntity, const AcGePoint3d& pickedPoint, AcEdSelectionSetService& service)
{
	if (m_color_index < 0)
		return;
	reset_color();
}

bool MySSGetFilter::canBeSelected(AcDbObjectId id) const
{
	bool ans = true;
	AcDbEntity * pEnt;
	if (acdbOpenAcDbEntity(pEnt, id, AcDb::kForRead) == Acad::eOk) {
		ans = canBeSelected(pEnt);
		pEnt->close();
	}
	return ans;
}

bool MySSGetFilter::canBeSelected(const AcDbEntity * pEnt) const
{
	return true;
}

void MySSGetFilter::high_light(AcDbObjectId id)
{
	if (m_obj_origin_color_map.find(id) != m_obj_origin_color_map.end())
		return;
	ads_name name;
	if (acdbGetAdsName(name, id) == Acad::eOk) {
		int orign = set_color_index(id, m_color_index);
		if (orign >= 0) {
			acedRedraw(name, 3);
			m_obj_origin_color_map[id] = orign;
		}
	}
}

void MySSGetFilter::un_high_light(AcDbObjectId id)
{
	if (m_obj_origin_color_map.find(id) == m_obj_origin_color_map.end())
		return;
	ads_name name;
	if (acdbGetAdsName(name, id) == Acad::eOk) {
		set_color_index(id, m_obj_origin_color_map[id]);	//»Ö¸´ÑÕÉ«
		acedRedraw(name, 4);
		m_obj_origin_color_map.erase(id);
	}
}

void MySSGetFilter::reset_color()
{
	std::map<AcDbObjectId, int>::iterator itr = m_obj_origin_color_map.begin();
	for (; itr != m_obj_origin_color_map.end(); ++itr) {
		set_color_index(itr->first, itr->second);
	}
	m_obj_origin_color_map.clear();
}

int set_color_index(AcDbObjectId id, int color_index)
{
	AcDbEntity * pEnt;
	if (acdbOpenAcDbEntity(pEnt, id, AcDb::kForWrite) == Acad::eOk) {
		return set_color_index(pEnt, color_index);
	}
	return -1;
}

int set_color_index(AcDbEntity * pEnt, int color_index)
{
	int ans = pEnt->colorIndex();
	pEnt->setColorIndex(color_index);
	return ans;
}
