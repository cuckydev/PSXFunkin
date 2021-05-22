#include "object.h"

//Object functions
void ObjectList_Add(ObjectList *list, Object *obj)
{
	if (*list != NULL)
	{
		//Link to list's head
		obj->prev = NULL;
		obj->next = *list;
		(*list)->prev = obj;
	}
	else
	{
		//Nothing to link to
		obj->prev = NULL;
		obj->next = NULL;
	}
	
	//Set as head of list
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
	free3(obj);
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
	//Free all contained objects
	for (Object *obj = *list; obj != NULL;)
	{
		//Free object and iterate on next linked object
		Object *next = obj->next;
		obj->free(obj);
		free3(obj);
		obj = next;
	}
	
	//Clear list pointer
	*list = NULL;
}
