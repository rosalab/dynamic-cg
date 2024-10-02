#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

int main() {
	
//	while(1) {
//	    
        unsigned long dummy = 0xdeaddead;
        printf("%p\n", &dummy);
        getchar();
		int sockfd = socket(AF_UNSPEC, SOCK_DGRAM, 0);	
		close(sockfd);
//	}
	return 0;
}
