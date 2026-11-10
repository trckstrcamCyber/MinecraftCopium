#include "stdafx.h"
#include "GrindstoneMenu.h"
#include "GrindstoneContainer.h"

GrindstoneContainer::GrindstoneContainer(GrindstoneMenu *menu, int name, bool customName, int size) : SimpleContainer(name, L"", customName, size)
{
	m_menu = menu;
}

void GrindstoneContainer::setChanged()
{
	SimpleContainer::setChanged();
	m_menu->slotsChanged(shared_from_this());
}

bool GrindstoneContainer::canPlaceItem(int slot, shared_ptr<ItemInstance> item)
{
	return true;
}
