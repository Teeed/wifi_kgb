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

#include "ieee80211.h"
#include "wifi_80211.h"

#include <string.h>

static void process_80211_mgmt(struct ieee80211_info *ieee80211_info, const struct ieee80211_mgmt *mgmt, uint32_t packet_len)
{
	const struct ieee80211_frame_control *frame_control = (const struct ieee80211_frame_control*)&(mgmt->header.frame_control);

	const void *data_end = (const void*)mgmt + packet_len;
	const void *element_ptr;

	switch(frame_control->subtype)
	{
		case FRAME_CONTROL_SUBTYPE_PROBE_REQ:
			element_ptr = mgmt->probe_req.variable;
			ieee80211_info->flags.is_probe = 1;
			break;

		case FRAME_CONTROL_SUBTYPE_BEACON:
			element_ptr = mgmt->beacon.variable;
			ieee80211_info->flags.is_beacon = 1;

			// ieee80211_info->dest_addr = ieee80211_info->from_addr;
			// ieee80211_info->flags.dest_addr_is_valid = 1;
			// ieee80211_info->flags.from_addr_is_valid = 0;
			break;

		default:
			ieee80211_info->flags.is_ssid_valid = 0;
			return;
	}

	while(element_ptr < data_end)
	{
		const struct ieee80211_information_element *element = (const struct ieee80211_information_element *)element_ptr;

		// printf("ID: %d, len: %d\n", element->ID, element->len);

		if(element->ID == INFORMATION_ELEMENT_SSID)
		{
			if(element->len > MAX_SSID_LENGTH || element->len < 0)
			{
				ieee80211_info->flags.is_valid = 0;

				return;
			}
			ieee80211_info->ssid_info.ssid = element->info;
			ieee80211_info->ssid_info.ssid_length = element->len;

			ieee80211_info->flags.is_ssid_valid = 1;
		}

		element_ptr += sizeof(struct ieee80211_information_element) + element->len; // get next
	}

}

void ieee80211_process_packet(struct ieee80211_info *ieee80211_info, const uint8_t *packet, uint32_t packet_len)
{
	ieee80211_info->flags.is_valid = 0;

	if(packet_len < sizeof(struct ieee80211_header)) // To short
	{
		return;
	}

	const struct ieee80211_header *header = (const struct ieee80211_header*)packet;
	const struct ieee80211_frame_control *frame_control = (const struct ieee80211_frame_control*)&(header->frame_control);

	if(!frame_control->to_ds && frame_control->from_ds)      // AP -> STATION, address 1 is station's address
	{
		ieee80211_info->from_addr = header->da;
		ieee80211_info->dest_addr = header->sa;

		ieee80211_info->flags.from_addr_is_valid = 1;
		ieee80211_info->flags.dest_addr_is_valid = 1;
	}
	else if(frame_control->to_ds && !frame_control->from_ds) // STATION -> AP, address 2 is client's address
	{
		ieee80211_info->dest_addr = header->da;
		ieee80211_info->from_addr = header->sa;

		ieee80211_info->flags.from_addr_is_valid = 1;
		ieee80211_info->flags.dest_addr_is_valid = 1;
	}
	else if(!frame_control->to_ds && !frame_control->from_ds)// IBSS/DLS
	{
		ieee80211_info->from_addr = header->sa;

		ieee80211_info->flags.from_addr_is_valid = 1;
	}

	ieee80211_info->flags.is_valid = 1;

	if(frame_control->type == 0) // management frame
	{
		process_80211_mgmt(ieee80211_info, (const struct ieee80211_mgmt *)packet, packet_len);
	}


	// switch(packet[0])
}


void ieee80211_clear(struct ieee80211_info *ieee80211_info)
{
	bzero(&ieee80211_info->flags, sizeof(ieee80211_info->flags));
}
