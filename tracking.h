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

#ifndef _TRACKING_H
#define _TRACKING_H

#include "wifi_80211.h"
#include "client.h"


#define MAX_TRACKING_MAC 1024
#define MAX_TRACKING_SSIDS 200


#include "defaultdict.h"


struct ssid_tracking_key
{
	// uint8_t ssid[MAX_SSID_LENGTH];
	uint8_t *ssid;
	uint8_t ssid_length;
};

struct ssid_tracking_entry
{
	uint32_t packets;
	time_t last_at;
};

struct ssid_tracking
{
	struct defaultdict dict;
};

struct tracking_entry_flags
{
	uint8_t switched_channels : 1;
	uint8_t is_ap : 1;
	uint8_t is_probing : 1;
	uint8_t is_new : 1;
};

struct tracking_entry
{
	time_t last_seen;
	uint32_t packets;
	int8_t average_power;
	uint16_t last_frequency;
	struct tracking_entry_flags flags;

	struct ssid_tracking probe_tracking;
	struct ssid_tracking ap_ssid_tracking;
};


struct tracking
{
	struct defaultdict mac_dict;
};

uint64_t tracking_mac_to_int(uint8_t *mac);

void ssid_tracking_init(struct ssid_tracking *ssid_tracking);
void ssid_tracking_free(struct ssid_tracking *ssid_tracking);
void ssid_tracking_add(struct ssid_tracking *ssid_tracking, const uint8_t *ssid, uint8_t ssid_length);

void tracking_track(struct tracking *tracking, struct process_result *process_result);
void tracking_clear(struct tracking *tracking, time_t older_than);
void tracking_init(struct tracking *tracking);
void tracking_print(const struct tracking *tracking);
void tracking_free(struct tracking *tracking);
const struct tracking_entry * tracking_iter(struct tracking *tracking, const struct tracking_entry *last_entry);


#endif
