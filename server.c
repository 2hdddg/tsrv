#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "log.h"
#include "server.h"

int _listenfd = -1;
bool _is_shutting_down = false;

static bool create_socket(int port)
{
	_listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenfd == -1){
		log_print(LOG_CRIT, "Failed to create socket: %s", strerror(errno));
		return false;
	}
	return true;
} 

static void close_socket()
{
	if (_listenfd >= 0){
		close(_listenfd);
	}
}

static bool bind_to_socket(int port)
{
	struct sockaddr_in localaddress;

	memset(&localaddress, 0, sizeof(struct sockaddr_in));
	localaddress.sin_family = AF_INET;
	localaddress.sin_addr.s_addr = INADDR_ANY;
	localaddress.sin_port = htons(port);
	if (bind(_listenfd, 
			(struct sockaddr*)&localaddress, 
			sizeof(struct sockaddr_in)) == -1){
		log_print(LOG_CRIT, "Failed to bind socket: %s", strerror(errno));
		return false;
	}
	return true;
}

static bool listen_on_socket()
{
	if (listen(_listenfd, 1) == -1){ 
		log_print(LOG_CRIT, "Failed to listen on socket: %s", strerror(errno));
		return false;
	}
	return true;
}

static bool accept_on_socket(int *acceptedfd_out)
{
	struct sockaddr_in remoteaddress;
	socklen_t remoteaddress_len = sizeof(struct sockaddr_in);
	int acceptedfd = accept(_listenfd, 
		(struct sockaddr*)&remoteaddress, 
		&remoteaddress_len);
	if (acceptedfd == -1){
		if (!_is_shutting_down){
			log_print(LOG_CRIT, "Failed to accept: %s", strerror(errno));
		}
		return false;
	}
	else{
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &remoteaddress.sin_addr, addr, INET_ADDRSTRLEN);
		log_print(LOG_INFO, "Got a connection from %s", addr);

		*acceptedfd_out = acceptedfd;
		return true;
	}
}

bool server_start(int port)
{
	int error;
	int failed;

	bool success = 
		create_socket(port) &&
		bind_to_socket(port) &&
		listen_on_socket();

	if (!success){
		close_socket();
		return false;	
	}

	log_print(LOG_INFO, "Accepting connections on port %d", port);
	while(success){
		int acceptedfd;
		success = accept_on_socket(&acceptedfd);
		if (!success){
			break;
		} 
	}
	close_socket();
	return true;
}

void server_stop()
{
	log_print(LOG_INFO, "Initiating shutdown...");
	_is_shutting_down = true;
	close_socket();
}
