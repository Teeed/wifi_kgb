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

#ifndef _DEFAULTDICT_H
#define _DEFAULTDICT_H

#include <stdint.h>

struct defaultdict_entry
{
	void *key;
	void *data;

	struct defaultdict_entry *next;
};

typedef void*(*defaultdict_new_element_function)(void **key_ptr, const void *searched_key);
typedef int(*defaultdict_element_compare_function)(const void *key1, const void *key2);
typedef void(*defaultdict_element_free_function)(void *key_ptr, void *data);


struct defaultdict
{
	defaultdict_new_element_function new_element_function;
	defaultdict_element_compare_function element_compare_function;
	defaultdict_element_free_function element_free_function;

	struct defaultdict_entry *list;
	uint32_t count;
};



void defaultdict_init(struct defaultdict *defaultdict, defaultdict_new_element_function new_element_function, defaultdict_element_compare_function element_compare_function, defaultdict_element_free_function element_free_function);
void * defaultdict_getset(struct defaultdict *defaultdict, const void *key);
struct defaultdict_entry *defaultdict_iter(const struct defaultdict *defaultdict, struct defaultdict_entry *current_entry);

void defaultdict_free(struct defaultdict *defaultdict);

#endif
