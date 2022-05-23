
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 

#define CLIENT_PORT 50001
#define SERVER_PORT 50000
#define MSG "Hello, World!"

int main(void) {
	struct sockaddr_in sa,ca;
	socklen_t sin_size;
	int s,c;
	ssize_t bytes;

	s = socket(PF_INET,SOCK_DGRAM,0);
	if (s < 0) {
		perror("socket");
		return 1;
	}
	sin_size = sizeof(sa);
	sa.sin_family = PF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = ntohs(CLIENT_PORT);
	if (bind(s,(struct sockaddr *)&sa,sin_size) < 0) {
		perror("bind");
		return 1;
	}
	ca.sin_family = PF_INET;
	ca.sin_addr.s_addr = inet_addr("127.0.0.1");
	ca.sin_port = ntohs(SERVER_PORT);
	if (connect(s,(struct sockaddr *)&ca,sin_size) < 0) {
		perror("connect");
		return 1;
	}
	printf("Sending message: %s\n", MSG);
	bytes = send(s,MSG,strlen(MSG),0);
	/* printf("Sent bytes: %d\n", bytes); */
	close(s);
	return 0;
}
