#include "wsn.h"

int getAddress(char *out)
{
	struct ifaddrs *ifaddr, *tem;
	int val;
	val = getifaddrs(&ifaddr);
	if (val == 0) {
		tem = ifaddr;
		sprintf(out, "%s", inet_ntoa(((struct sockaddr_in*)ifaddr->ifa_addr)->sin_addr));
	}
	freeifaddrs(ifaddr);
	return 0;
}