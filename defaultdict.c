/*
	WiFi KGB
	https://github.com/Teeed/wifi_kgb


	Copyright (C) 2017 Tadeusz Magura-Witkowski

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "defaultdict.h"
#include "utils.h"

void defaultdict_init(struct defaultdict *defaultdict, defaultdict_new_element_function new_element_function, defaultdict_element_compare_function compare_function, defaultdict_element_free_function element_free_function)
{
	defaultdict->new_element_function = new_element_function;
	defaultdict->element_compare_function = compare_function;
	defaultdict->element_free_function = element_free_function;

	defaultdict->list = NULL;
	defaultdict->count = 0;
}


void * defaultdict_getset(struct defaultdict *defaultdict, const void *key)
{
	struct defaultdict_entry **pentry = &defaultdict->list;

	while(*pentry)
	{
		if(defaultdict->element_compare_function((*pentry)->key, key) == 0)
		{
			return (*pentry)->data;
		}

		pentry = &((*pentry)->next);
	}

	defaultdict->count++;

	struct defaultdict_entry *entry = xmalloc(sizeof(struct defaultdict_entry));

	entry->data = defaultdict->new_element_function(&(entry->key), key);
	entry->next = NULL;

	(*pentry) = entry;

	return entry->data;
}


struct defaultdict_entry *defaultdict_iter(const struct defaultdict *defaultdict, struct defaultdict_entry *current_entry)
{
	if(current_entry == NULL)
	{
		return defaultdict->list;
	}

	return current_entry->next;
}



void defaultdict_free(struct defaultdict *defaultdict)
{
	struct defaultdict_entry *entry = defaultdict->list;

	while(entry)
	{
		struct defaultdict_entry *curr = entry;
		entry = curr->next;

		defaultdict->element_free_function(curr->key, curr->data);

		free(curr);
	}

	defaultdict->list = NULL;
}
