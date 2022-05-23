
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 

#define PORT 50000

int main(void) {
	struct sockaddr_in sa,ca;
	socklen_t sin_size;
	int s,c;
	ssize_t bytes;
	unsigned char buf[1024];

	s = socket(PF_INET,SOCK_DGRAM,0);
	if (s < 0) {
		perror("socket");
		return 1;
	}
	listen(s,5);
	sa.sin_family = PF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = ntohs(PORT);
	sin_size = sizeof(sa);
	if (bind(s,(struct sockaddr *)&sa,sin_size) < 0) {
		perror("bind");
		return 1;
	}
	bytes = recv(s,buf,sizeof(buf),MSG_WAITALL);
	/* printf("Bytes: %d\n", bytes); */
	if (bytes) {
		buf[bytes] = 0;
		printf("Received message: %s\n", buf);
	}
	close(s);
	return 0;
}
