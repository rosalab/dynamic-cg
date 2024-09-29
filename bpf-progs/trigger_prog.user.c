#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

int main() {
	
	while(1) {
		int sockfd = socket(AF_UNSPEC, SOCK_DGRAM, 0);	
		close(sockfd);
	}
	return 0;
}
