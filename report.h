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

#ifndef _REPORT_H
#define _REPORT_H

#include <stdint.h>

#include "tracking.h"
#include "report.pb-c.h"


struct report
{
	Report__ClientReport report;
	
	uint8_t *buffer;
	uint32_t buffer_length;
};

void report_init(struct report *report);
void report_free(struct report *report);
void report_add_mac(struct report *report, uint64_t mac, struct tracking_entry *tracking_entry);
void report_serialize(struct report *report, uint64_t ap_mac);

#endif

