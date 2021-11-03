/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_OBJECT_H
#define PSXF_GUARD_OBJECT_H

#include "psx.h"

//Object types
typedef struct Object
{
	//Object linked list
	struct Object *prev, *next;
	
	//Object functions
	boolean (*tick)(struct Object*);
	void (*free)(struct Object*);
} Object;

typedef Object* ObjectList;

//Object functions
void ObjectList_Add(ObjectList *list, Object *obj);
void ObjectList_Remove(ObjectList *list, Object *obj);
void ObjectList_Tick(ObjectList *list);
void ObjectList_Free(ObjectList *list);

#endif
