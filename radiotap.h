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

#ifndef _RADIOTAP_H
#define _RADIOTAP_H


#include <stdint.h>

#define RADIOTAP_CHANNEL 3
#define RADIOTAP_ANT_SIGNAL 5

struct ieee80211_radiotap_header
{
		uint8_t        it_version;     /* set to 0 */
		uint8_t        it_pad;
		uint16_t       it_len;         /* entire length */
		uint32_t       it_present;     /* fields present */
} __attribute__((__packed__));


struct radiotap_field_channel
{
	uint16_t frequency;
	uint16_t flags;
};

struct radiotap_field_ant_signal
{
	int8_t power;
};

struct radiotap_rx_info
{
	struct radiotap_field_ant_signal antenna_signal;
	struct radiotap_field_channel channel;

	struct {
		uint8_t antenna_signal_valid : 1;
		uint8_t channel_valid : 1;	
		uint8_t is_valid : 1;	
	} flags;
};


void radiotap_get_rxpower_channel(struct radiotap_rx_info *radiotap_rx_info, struct ieee80211_radiotap_header *radiotap);
void radiotap_clear(struct radiotap_rx_info *radiotap_rx_info);


#endif
