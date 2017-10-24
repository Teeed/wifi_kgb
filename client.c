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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>


#include <linux/wireless.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <pcap.h>

#include "client.h"

#include "wifi_80211.h"
#include "ieee80211.h"
#include "tracking.h"
#include "utils.h"

#define BUFFERSIZE 2048
#define MACADDR_LEN 6




struct wifi_kgb
{
	char *device_name;

	char *server_str;
	int server_port;


	uint8_t mac_addr[MACADDR_LEN];
	struct sockaddr_in server_addr;
	int socket;


	struct tracking tracking;

	pcap_t *pcap_handle;
};



static void print_process_result(const struct radiotap_rx_info *radiotap_rx_info, const struct ieee80211_info *ieee80211_info)
{

	printf("Channel info: ");
	printf("channel_valid %d, antenna_signal_valid %d, freq %d, power %d\n", radiotap_rx_info->flags.channel_valid, radiotap_rx_info->flags.antenna_signal_valid, 
		radiotap_rx_info->channel.frequency, radiotap_rx_info->antenna_signal.power);


	if(!ieee80211_info->flags.is_valid)
	{
		printf("Invalid frame\n");
		return;
	}

	if(ieee80211_info->flags.from_addr_is_valid)
	{
		printf(" Client: ");
		hexdump(ieee80211_info->from_addr, 6);
	}
	if(ieee80211_info->flags.dest_addr_is_valid)
	{
		printf(" AP: ");
		hexdump(ieee80211_info->dest_addr, 6);
	}

	printf("Flags:");

	if(ieee80211_info->flags.is_probe)
		printf(" IS_PROBE");

	if(ieee80211_info->flags.is_beacon)
		printf(" IS_BEACON");

	if(ieee80211_info->flags.is_ssid_valid)
		printf(" IS_SSID_VALID");

	putchar('\n');


	if(ieee80211_info->flags.is_ssid_valid)
	{
		char probe_ssid[MAX_SSID_LENGTH + 1];
		memcpy(probe_ssid, ieee80211_info->ssid_info.ssid, ieee80211_info->ssid_info.ssid_length);
		probe_ssid[ieee80211_info->ssid_info.ssid_length] = '\0';

		printf(" SSID: %s\n", probe_ssid);
	}
}




static void pass_result_to_tracking(struct wifi_kgb *kgb, struct process_result *process_result)
{
	if(!process_result->ieee80211_info.flags.is_valid)
	{
		return; 
	}
	
	tracking_track(&kgb->tracking, process_result);
	tracking_print(&kgb->tracking);
}

static void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
	struct wifi_kgb *kgb = (struct wifi_kgb *)args;

	if(header->len < sizeof(struct ieee80211_radiotap_header))
	{
		return;
	}

	struct ieee80211_radiotap_header *radiotap_header = (struct ieee80211_radiotap_header*)packet;
	// strip radiotap header
	const u_char *packet_data = packet + radiotap_header->it_len;
	const uint32_t packet_len = header->len - radiotap_header->it_len;

	struct process_result process_result;


	radiotap_clear(&process_result.radiotap_rx_info);
	radiotap_get_rxpower_channel(&process_result.radiotap_rx_info, radiotap_header);

	ieee80211_clear(&process_result.ieee80211_info);
	ieee80211_process_packet(&process_result.ieee80211_info, packet_data, packet_len);

	if(process_result.ieee80211_info.flags.is_valid)
	{
		putchar('\n');
		print_process_result(&process_result.radiotap_rx_info, &process_result.ieee80211_info);
	}

	pass_result_to_tracking(kgb, &process_result);
}


static int reconnect(struct wifi_kgb *kgb)
{

	if (connect(kgb->socket, (struct sockaddr *) &kgb->server_addr, sizeof(kgb->server_addr)) < 0)
	{
		fprintf(stderr, "Could not connect to server\n");

		return 1;
	}

	return 0;
}


void usage(const char *argv0)
{
	fprintf(stderr, "Program to spy on users using multiple APs\n");
	fprintf(stderr, "Usage: %s -d device -s server -p port\n", argv0);
}


int main(int argc, char * const argv[])
{
	struct wifi_kgb kgb;

	bzero(&kgb, sizeof(struct wifi_kgb));

	int c;
	while((c=getopt(argc, argv, "hd:s:p:")) != -1)
	{
		switch(c)
		{
			case 'd':
				kgb.device_name = optarg;
				break;
			case 's':
				kgb.server_str = optarg;
				break;
			case 'p':
				kgb.server_port = atoi(optarg);
				break;
			case 'h':
				usage(argv[0]);
				return 0;
		}
	}

	if(kgb.device_name == NULL)
	{
		fprintf(stderr, "Which device? (-d wlan0)\n");
		usage(argv[0]);

		return 1;
	}

	if(kgb.server_str == NULL)
	{
		fprintf(stderr, "Server name is empty\n");
		usage(argv[0]);

		return 1;
	}

	if(kgb.server_port <= 0)
	{
		fprintf(stderr, "Invalid server port\n");
		usage(argv[0]);

		return 1;
	}

	char errbuf[BUFFERSIZE];
	kgb.pcap_handle = pcap_open_live(kgb.device_name, BUFFERSIZE, 1, 0, errbuf);
	if (kgb.pcap_handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", kgb.device_name, errbuf);

		return 1;
	}


	kgb.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(kgb.socket < 0)
	{
		perror("inet sock");
		
		return 1;
	}

	// get mac addr
	struct ifreq ifr;
	strncpy(ifr.ifr_name, kgb.device_name, IFNAMSIZ);
	if(ioctl(kgb.socket, SIOCGIFHWADDR, &ifr) < 0)
	{
		fprintf(stderr, "Failed to get card's mac addr\n");

		return 1;
	}	
	memcpy(kgb.mac_addr, ifr.ifr_hwaddr.sa_data, MACADDR_LEN);


	kgb.server_addr.sin_family = AF_INET;
	kgb.server_addr.sin_port = htons(kgb.server_port);
	if(inet_aton(kgb.server_str, &kgb.server_addr.sin_addr) == 0) 
	{
		fprintf(stderr, "Invalid server address\n");

		return 1;
	}

	// if(reconnect(&kgb))
	// {
	// 	return 1;
	// }


	tracking_init(&kgb.tracking);


	pcap_loop(kgb.pcap_handle, -1, got_packet, (void*)&kgb);

	pcap_close(kgb.pcap_handle);

	tracking_free(&kgb.tracking);


	// while(process_data(&kgb))
	// {
	// 	//
	// }

	// if(set_wifi_channel(kgb.socket, kgb.device_name, 1) > 0)
	// {
	// 	close(kgb.socket);
	// 	return 1;
	// }


	close(kgb.socket);

	return 0;
}
