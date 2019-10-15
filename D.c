#include <ifaddrs.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char* argv[])
{
	struct ifaddrs *ifaddr, *tem;
	int val;
	val = getifaddrs(&ifaddr);
	if (val == 0) {
		tem = ifaddr;
		while (tem != NULL) {
			// if (tem -> ifa_addr -> sa_family == AF_INET) {
				// if (strcmp(tem->ifa_name, "en0") == 0) {
			printf("%s\n", inet_ntoa(((struct sockaddr_in*)ifaddr->ifa_addr)->sin_addr));
				// }
			// }
			tem = tem->ifa_next;
		}
	}
	freeifaddrs(ifaddr);
	return 0;
}