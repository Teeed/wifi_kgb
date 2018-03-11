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


#include "report.h"
#include "utils.h"
#include "defaultdict.h"


void report_init(struct report *report)
{
	report->buffer = NULL;
	report->buffer_length = 0;

	report__client_report__init(&report->report);
}


void report_add_mac(struct report *report, uint64_t mac, struct tracking_entry *tracking_entry)
{
	Report__ClientReport *client_report = &report->report;

	Report__ClientReport__Report *report_report = xmalloc(sizeof(Report__ClientReport__Report));
	report__client_report__report__init(report_report);

	report_report->mac = mac;

	report_report->packets = tracking_entry->packets;
	report_report->has_packets = 1;
	report_report->average_power = tracking_entry->average_power;
	report_report->has_average_power = 1;
	report_report->last_frequency = tracking_entry->last_frequency;
	report_report->has_last_frequency = 1;

	report_report->flags = xmalloc(sizeof(Report__ClientReport__Report__Flags));
	report__client_report__report__flags__init(report_report->flags);
	report_report->flags->switched_channels = tracking_entry->flags.switched_channels;
	report_report->flags->is_ap = tracking_entry->flags.is_ap;
	report_report->flags->is_probing = tracking_entry->flags.is_probing;

	report_report->n_probe_tracking = 0;
	report_report->n_ap_ssid_tracking = 0;

	struct defaultdict_entry *pt_dentry = NULL;
	while((pt_dentry = defaultdict_iter(&tracking_entry->probe_tracking.dict, pt_dentry)))
	{
		struct ssid_tracking_entry *pt_entry = (struct ssid_tracking_entry *)pt_dentry->data;
		struct ssid_tracking_key *pt_entry_key = (struct ssid_tracking_key *)pt_dentry->key;

		Report__SSIDEntry *probe_tracking_entry = xmalloc(sizeof(Report__SSIDEntry));
		report__ssidentry__init(probe_tracking_entry);

		probe_tracking_entry->packets = pt_entry->packets;
		probe_tracking_entry->ssid.data = pt_entry_key->ssid;
		probe_tracking_entry->ssid.len = pt_entry_key->ssid_length;

		size_t report_no = report_report->n_probe_tracking;
		report_report->n_probe_tracking += 1;
		report_report->probe_tracking = xrealloc(report_report->probe_tracking, 
			report_report->n_probe_tracking*sizeof(Report__SSIDEntry*));
		report_report->probe_tracking[report_no] = probe_tracking_entry;
	}

	pt_dentry = NULL;
	while((pt_dentry = defaultdict_iter(&tracking_entry->ap_ssid_tracking.dict, pt_dentry)))
	{
		struct ssid_tracking_entry *pt_entry = (struct ssid_tracking_entry *)pt_dentry->data;
		struct ssid_tracking_key *pt_entry_key = (struct ssid_tracking_key *)pt_dentry->key;

		Report__SSIDEntry *ap_ssid_tracking_entry = xmalloc(sizeof(Report__SSIDEntry));
		report__ssidentry__init(ap_ssid_tracking_entry);

		ap_ssid_tracking_entry->packets = pt_entry->packets;
		ap_ssid_tracking_entry->ssid.data = pt_entry_key->ssid;
		ap_ssid_tracking_entry->ssid.len = pt_entry_key->ssid_length;

		size_t report_no = report_report->n_ap_ssid_tracking;
		report_report->n_ap_ssid_tracking += 1;
		report_report->ap_ssid_tracking = xrealloc(report_report->ap_ssid_tracking, 
			report_report->n_ap_ssid_tracking*sizeof(Report__SSIDEntry*));
		report_report->ap_ssid_tracking[report_no] = ap_ssid_tracking_entry;
	}

	size_t rep_id = client_report->n_report;
	client_report->n_report += 1;
	client_report->report = xrealloc(client_report->report, client_report->n_report * sizeof(Report__ClientReport__Report*));
	client_report->report[rep_id] = report_report;
}

void report_free(struct report *report)
{
	int i, j;

	for(i = 0; i < report->report.n_report; i++)
	{
		Report__ClientReport__Report *report_report = report->report.report[i];

		for(j = 0; j < report_report->n_probe_tracking; j++)
		{
			free(report_report->probe_tracking[j]);
		}

		for(j = 0; j < report_report->n_ap_ssid_tracking; j++)
		{
			free(report_report->ap_ssid_tracking[j]);
		}

		free(report_report->flags);
		free(report_report->probe_tracking);
		free(report_report->ap_ssid_tracking);

		free(report_report);
	}

	free(report->report.report);

	if(report->buffer)
	{
		free(report->buffer);
		report->buffer = NULL;
	}
} 



void report_serialize(struct report *report, uint64_t ap_mac)
{
	report->report.ap_mac = ap_mac;

	report->buffer_length = report__client_report__get_packed_size(&report->report);

	if(report->buffer != NULL)
	{
		free(report->buffer);
	}
	report->buffer = xmalloc(report->buffer_length);

	report__client_report__pack(&report->report, report->buffer);
}
