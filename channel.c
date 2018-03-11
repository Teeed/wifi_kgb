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

#include <math.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>


#include "channel.h"

// wifi channel setting
static void channel_to_iwfreq(struct iw_freq *freq, const int channel)
{
	freq->m = channel;
	freq->e = 0;
	freq->i = 0;
	freq->flags = 0;
}

int channel_set(const int socket, const char *device_name, const int channel)
{
	
	struct iwreq wrq;
	strncpy(wrq.ifr_name, device_name, IFNAMSIZ);

	channel_to_iwfreq(&wrq.u.freq, channel);

	if(ioctl(socket, SIOCSIWFREQ, &wrq) < 0)
	{
		return 1;
	}

	return 0;
}

void channel_traverse(const int socket,  const char *device_name)
{
	struct iwreq wrq;
	char buf[sizeof(struct iw_range) * 2];
	int ret;

	/* Prepare request. */
	bzero(buf, sizeof(buf));
	wrq.u.data.pointer = buf;
	wrq.u.data.length = sizeof(buf);
	wrq.u.data.flags = 0;

	/* Get range. */
	strncpy(wrq.ifr_name, device_name, IFNAMSIZ);
	if ((ret = ioctl(socket, SIOCGIWRANGE, &wrq)) >= 0)
	{
		struct iw_range *range = (struct iw_range *) buf;
		int k;

		/* Compare the frequencies as double to ignore differences in encoding.
		* Slower, but safer... */
		for (k = 0; k < range->num_frequency; k++)
		{
			// struct iw_freq *freq = &
			wrq.u.freq = range->freq[k];
			ioctl(socket, SIOCSIWFREQ, &wrq);
			printf("%d\n", range->freq[k].m);

			usleep(200000);

		}
	}
}