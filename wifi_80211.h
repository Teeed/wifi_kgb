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

#ifndef _WIFI_80211_H
#define _WIFI_80211_H


#include <linux/types.h>

#include "stdint.h"

#define ETH_ALEN 6
#define MAX_SSID_LENGTH 32 // is 32 in linux kernel (4.xx)


struct ieee80211_header {
	uint16_t frame_control;
	uint16_t duration;
	uint8_t da[ETH_ALEN];
	uint8_t sa[ETH_ALEN];
	uint8_t bssid[ETH_ALEN];
	uint16_t seq_ctrl;
} __attribute__((packed, aligned(2)));


struct ieee80211_mgmt {
	struct ieee80211_header header;
	union {
		struct {
			uint16_t auth_alg;
			uint16_t auth_transaction;
			uint16_t status_code;
			/* possibly followed by Challenge text */
			uint8_t variable[0];
		} __attribute__((__packed__)) auth;
		struct {
			uint16_t reason_code;
		} __attribute__((__packed__)) deauth;
		struct {
			uint16_t capab_info;
			uint16_t listen_interval;
			/* followed by SSID and Supported rates */
			uint8_t variable[0];
		} __attribute__((__packed__)) assoc_req;
		struct {
			uint16_t capab_info;
			uint16_t status_code;
			uint16_t aid;
			/* followed by Supported rates */
			uint8_t variable[0];
		} __attribute__((__packed__)) assoc_resp, reassoc_resp;
		struct {
			uint16_t capab_info;
			uint16_t listen_interval;
			uint8_t current_ap[ETH_ALEN];
			/* followed by SSID and Supported rates */
			uint8_t variable[0];
		} __attribute__((__packed__)) reassoc_req;
		struct {
			uint16_t reason_code;
		} __attribute__((__packed__)) disassoc;
		struct {
			uint64_t timestamp;
			uint16_t beacon_int;
			uint16_t capab_info;
			/* followed by some of SSID, Supported rates,
				* FH Params, DS Params, CF Params, IBSS Params, TIM */
			uint8_t variable[0];
		} __attribute__((__packed__)) beacon;
		struct {
			/* only variable items: SSID, Supported rates */
			uint8_t variable[0];
		} __attribute__((__packed__)) probe_req;
		struct {
			uint64_t timestamp;
			uint16_t beacon_int;
			uint16_t capab_info;
			/* followed by some of SSID, Supported rates,
				* FH Params, DS Params, CF Params, IBSS Params */
			uint8_t variable[0];
		} __attribute__((__packed__)) probe_resp;
	};
} __attribute__((packed, aligned(2)));


// from linux/ieee80211.h
struct ieee80211_information_element {
	uint8_t ID;
	uint8_t len;
	uint8_t info[0];
} __attribute__((packed));




#ifndef __ORDER_LITTLE_ENDIAN__
	#error "Only little endian systems are supported"
#endif
struct ieee80211_frame_control {
	uint8_t version : 2;
	uint8_t type : 2;
	uint8_t subtype : 4;

	uint8_t to_ds : 1;
	uint8_t from_ds : 1;
	uint8_t more_fragments : 1;
	uint8_t retry : 1;

	uint8_t power_management : 1;
	uint8_t more_data : 1;
	uint8_t wep : 1;
	uint8_t order : 1;
} __attribute__ ((packed));




#endif
