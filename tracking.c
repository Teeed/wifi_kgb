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

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "ieee80211.h"
#include "tracking.h"
#include "utils.h"


void *ssid_tracking_new_fun(void **_key, const void *_original_key)
{
	(*_key) = xmalloc(sizeof(struct ssid_tracking_key));
	struct ssid_tracking_key *original_key = (struct ssid_tracking_key *)_original_key;
	struct ssid_tracking_key *key = (struct ssid_tracking_key *)*_key;

	key->ssid = xmalloc(original_key->ssid_length);
	memcpy(key->ssid, original_key->ssid, original_key->ssid_length);
	key->ssid_length = original_key->ssid_length;

	struct ssid_tracking_entry *new_entry = xmalloc(sizeof(struct ssid_tracking_entry));
	new_entry->packets = 1;
	new_entry->last_at = time(NULL);

	return new_entry;
}

int ssid_tracking_cmp_fun(const void *_key1, const void *_key2)
{
	const struct ssid_tracking_key *key1 = _key1;
	const struct ssid_tracking_key *key2 = _key2;

	return memcmp(key1->ssid, key2->ssid, MIN(key1->ssid_length, key2->ssid_length));
}

void ssid_tracking_free_fun(void *_key, void *data)
{
	struct ssid_tracking_key *key = (struct ssid_tracking_key *)_key;

	free(key->ssid);
	free(key);
	free(data);
}

void ssid_tracking_init(struct ssid_tracking *ssid_tracking)
{
	defaultdict_init(&ssid_tracking->dict, ssid_tracking_new_fun, ssid_tracking_cmp_fun, ssid_tracking_free_fun);
}

void ssid_tracking_free(struct ssid_tracking *ssid_tracking)
{
	defaultdict_free(&ssid_tracking->dict);
}

void ssid_tracking_add(struct ssid_tracking *ssid_tracking, const uint8_t *ssid, uint8_t ssid_length)
{
	ssid_length = MIN(ssid_length, MAX_SSID_LENGTH);

	struct ssid_tracking_key key = {
		.ssid = ssid,
		.ssid_length = ssid_length
	};

	struct ssid_tracking_entry *entry = defaultdict_getset(&ssid_tracking->dict, &key);
	entry->last_at = time(NULL);
	entry->packets ++;
}


void tracking_track(struct tracking *tracking, struct process_result *process_result)
{
	int8_t power = 0;
	uint16_t frequency = 0;

	if(!process_result->ieee80211_info.flags.from_addr_is_valid)
	{
		return;
	}

	const uint8_t *mac = process_result->ieee80211_info.from_addr;
	struct tracking_entry *entry = defaultdict_getset(&tracking->mac_dict, mac);
	entry->last_seen = time(NULL);
	entry->packets++;

	if(process_result->radiotap_rx_info.flags.antenna_signal_valid)
	{
		power = process_result->radiotap_rx_info.antenna_signal.power;
	}

	if(process_result->radiotap_rx_info.flags.channel_valid)
	{
		frequency = process_result->radiotap_rx_info.channel.frequency;
	}

	if(process_result->ieee80211_info.flags.is_beacon)
	{
		entry->flags.is_ap = 1;
		ssid_tracking_add(&entry->ap_ssid_tracking, process_result->ieee80211_info.beacon_info.ssid,
			process_result->ieee80211_info.beacon_info.ssid_length);
	}
	else if(process_result->ieee80211_info.flags.is_probe)
	{
		entry->flags.is_probing = 1;
		ssid_tracking_add(&entry->probe_tracking, process_result->ieee80211_info.probe_request_info.ssid,
			process_result->ieee80211_info.probe_request_info.ssid_length);
	}


	// if(__builtin_expect((entry->flags.is_new == 1), 0))
	if(entry->flags.is_new == 1)
	{
		entry->average_power = power;
		entry->flags.is_new = 0;
	}
	else
	{
		entry->average_power = (entry->average_power + power)/2;
		if(entry->last_frequency != frequency)
		{
			entry->flags.switched_channels = 1;
		}
	}
	entry->last_frequency = frequency;
}

void tracking_print(const struct tracking *tracking)
{
	printf("Tracking entries: %d\n", tracking->mac_dict.count);
	struct defaultdict_entry *dentry = NULL;

	while((dentry = defaultdict_iter(&tracking->mac_dict, dentry)))
	{
		struct tracking_entry *entry = (struct tracking_entry *)dentry->data;

		printf("Entry: ");
		hexdump(dentry->key, 6);
		putchar('\n');

		printf(" packets: %d\n", entry->packets);
		printf(" last_seen: %d\n", entry->last_seen);
		printf(" average_power: %d\n", entry->average_power);
		printf(" last_frequency: %d\n", entry->last_frequency);
		printf(" flags:");

		if(entry->flags.switched_channels)
			printf(" switched_channels");

		if(entry->flags.is_ap)
			printf(" is_ap");

		if(entry->flags.is_probing)
			printf(" is_probing");

		if(entry->flags.is_new)
			printf(" is_new");

		putchar('\n');

		struct defaultdict_entry *pt_dentry = NULL;
		char ssid[33];

		printf(" Probe tracked: \n");
		while((pt_dentry = defaultdict_iter(&entry->probe_tracking.dict, pt_dentry)))
		{
			struct ssid_tracking_entry *pt_entry = pt_dentry->data;
			struct ssid_tracking_key *pt_entry_key = pt_dentry->key;

			memcpy(ssid, pt_entry_key->ssid, pt_entry_key->ssid_length);
			ssid[pt_entry_key->ssid_length] = 0;

			printf("  SSID: '%s' packets: %d\n", ssid, pt_entry->packets);
		}
		putchar('\n');

		printf(" Network 'beaconed': \n");
		while((pt_dentry = defaultdict_iter(&entry->ap_ssid_tracking.dict, pt_dentry)))
		{
			struct ssid_tracking_entry *pt_entry = pt_dentry->data;
			struct ssid_tracking_key *pt_entry_key = pt_dentry->key;

			memcpy(ssid, pt_entry_key->ssid, pt_entry_key->ssid_length);
			ssid[pt_entry_key->ssid_length] = 0;

			printf("  SSID: '%s' packets: %d\n", ssid, pt_entry->packets);
		}
		putchar('\n');
	}
}

void *tracking_new_fun(void **_key, const void *searched_mac)
{
	*_key = xmalloc(ETH_ALEN);
	memcpy(*_key, searched_mac, ETH_ALEN);

	struct tracking_entry *new_entry = xmalloc(sizeof(struct tracking_entry));

	new_entry->last_seen = time(NULL);
	new_entry->packets = 1;
	new_entry->average_power = 0;
	new_entry->last_frequency = 0;
	bzero(&new_entry->flags, sizeof(new_entry->flags));
	new_entry->flags.is_new = 1;

	ssid_tracking_init(&new_entry->probe_tracking);
	ssid_tracking_init(&new_entry->ap_ssid_tracking);

	return new_entry;
}

int tracking_cmp_fun(const void *mac1, const void *mac2)
{
	return memcmp(mac1, mac2, ETH_ALEN);
}

void tracking_free_fun(void *key, void *data)
{
	struct tracking_entry *entry = (struct tracking_entry *)data;

	ssid_tracking_free(&entry->probe_tracking);
	ssid_tracking_free(&entry->ap_ssid_tracking);


	free(key);
	free(data);
}

void tracking_init(struct tracking *tracking)
{
	defaultdict_init(&tracking->mac_dict, tracking_new_fun, tracking_cmp_fun, tracking_free_fun);
}


void tracking_free(struct tracking *tracking)
{
	defaultdict_free(&tracking->mac_dict);
}
