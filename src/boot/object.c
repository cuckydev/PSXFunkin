/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "object.h"

#include "mem.h"

//Object functions
void ObjectList_Add(ObjectList *list, Object *obj)
{
	//Link to list
	obj->prev = NULL;
	if ((obj->next = *list) != NULL)
		(*list)->prev = obj;
	*list = obj;
}

void ObjectList_Remove(ObjectList *list, Object *obj)
{
	//Unlink object
	if (obj->prev != NULL)
		obj->prev->next = obj->next;
	else
		*list = obj->next;
	if (obj->next != NULL)
		obj->next->prev = obj->prev;
	
	//Free object
	obj->free(obj);
	Mem_Free(obj);
}

void ObjectList_Tick(ObjectList *list)
{
	//Tick all contained objects
	for (Object *obj = *list; obj != NULL;)
	{
		//Tick object and iterate on next linked object
		Object *next = obj->next;
		if (obj->tick(obj))
			ObjectList_Remove(list, obj);
		obj = next;
	}
}

void ObjectList_Free(ObjectList *list)
{
	//Check if list is already free'd
	if (*list == NULL)
		return;
	
	//Free all contained objects
	for (Object *obj = *list; obj != NULL;)
	{
		//Free object and iterate on next linked object
		Object *next = obj->next;
		obj->free(obj);
		Mem_Free(obj);
		obj = next;
	}
	
	//Clear list pointer
	*list = NULL;
}
