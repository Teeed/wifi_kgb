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

#ifndef _IEEE80211_H
#define _IEEE80211_H

#include <stdint.h>

#define FRAME_CONTROL_SUBTYPE_PROBE_REQ 4
#define FRAME_CONTROL_SUBTYPE_BEACON 8
#define INFORMATION_ELEMENT_SSID 0

struct ieee80211_info_flags
{
	uint8_t is_valid : 1;
	uint8_t from_addr_is_valid : 1;
	uint8_t dest_addr_is_valid : 1;

	uint8_t is_probe : 1;
	uint8_t is_beacon : 1;
	uint8_t is_ssid_valid : 1;
} __attribute__((__packed__));	

struct ieee80211_info
{
	const uint8_t *from_addr;
	const uint8_t *dest_addr;

	struct ieee80211_info_flags flags;

	union {
		struct
		{
			const uint8_t *ssid;
			uint8_t ssid_length;
		} ssid_info;
		struct
		{
			const uint8_t *ssid;
			uint8_t ssid_length;
		} probe_request_info;
		struct
		{
			const uint8_t *ssid;
			uint8_t ssid_length;
		} beacon_info;
	};
};




void ieee80211_process_packet(struct ieee80211_info *ieee80211_info, const uint8_t *packet, uint32_t packet_len);
void ieee80211_clear(struct ieee80211_info *ieee80211_info);



#endif /* _IEEE80211_H */
