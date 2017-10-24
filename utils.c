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

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

void hexdump(const uint8_t *data, int len)
{
	int i;

	for(i = 0 ; i < len; i++)
	{
		printf("%02x ", data[i]);
	}
}

void *xmalloc(int len)
{
	void *ptr = malloc(len);
	
	if(ptr == NULL)
	{
		printf("stderr: malloc failed\n");
		exit(1);
	}

	return ptr;
}
