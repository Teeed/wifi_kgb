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

#include "radiotap.h"

#include <assert.h>
#include <string.h>
#include <endian.h>

const uint8_t radiotap_header_field_lengths[] = { // lenthts of radiotap fields in bytes
	8, // TSFT,
	1, // FLAGS,
	1, // RATE,
	4, // CHANNEL    <- there are the ones we really care
	2, // FHSS
	1, // ANT SIGNAL <-
};

// crude (but fast) radiotap parsing to get what we need
void radiotap_get_rxpower_channel(struct radiotap_rx_info *radiotap_rx_info, struct ieee80211_radiotap_header *radiotap)
{
	if(radiotap->it_version != 0)
	{
		radiotap_rx_info->flags.is_valid = 0;

		return;
	}

	void *radiotap_fields = (uint8_t*)radiotap + sizeof(struct ieee80211_radiotap_header);
	void *radiotap_end = (uint8_t*)radiotap + le16toh(radiotap->it_len);

	if(le32toh(radiotap->it_present) & (1<<31)) // bit 31 is set, we will have additional it_present field
	{
		radiotap_fields += sizeof(radiotap->it_present);
	}

	radiotap_rx_info->flags.antenna_signal_valid = 0;
	radiotap_rx_info->flags.channel_valid = 0;

	int i;
	for(i = 0; i < sizeof(radiotap_header_field_lengths)/sizeof(radiotap_header_field_lengths[0]); i++)
	{
		assert(radiotap_fields < radiotap_end);

		if(le32toh(radiotap->it_present) & (1<<i)) // field present
		{
			switch(i)
			{
				case RADIOTAP_CHANNEL:
					memcpy(&radiotap_rx_info->channel, radiotap_fields, sizeof(radiotap_rx_info->channel));
					radiotap_rx_info->channel.frequency = le16toh(radiotap_rx_info->channel.frequency);
					radiotap_rx_info->channel.flags = le16toh(radiotap_rx_info->channel.flags);
					radiotap_rx_info->flags.channel_valid = 1;
					break;
				case RADIOTAP_ANT_SIGNAL:
					memcpy(&radiotap_rx_info->antenna_signal, radiotap_fields, sizeof(radiotap_rx_info->antenna_signal));
					// TODO: check if it even delivers...
					radiotap_rx_info->antenna_signal.power = le16toh(radiotap_rx_info->antenna_signal.power);
					radiotap_rx_info->flags.antenna_signal_valid = 1;
					break;
			}

			radiotap_fields += radiotap_header_field_lengths[i];
		}
	}
}


void radiotap_clear(struct radiotap_rx_info *radiotap_rx_info)
{
	radiotap_rx_info->flags.is_valid = 0;
}
