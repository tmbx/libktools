/* Copyright (C) 2006-2012 Opersys inc., All rights reserved. */

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "ksock.h"
#include "kutils.h"
#include "kerror.h"

/* This function calls KTOOLS_ERROR_SET with a string describing the last socket
 * error that occurred.
 */
static void ksock_seterror() {
    KTOOLS_ERROR_SET("%s", kerror_syserror());
}

int ksock_create(int *fd) {
    assert(*fd == -1);
    int error = socket(AF_INET, SOCK_STREAM, 0);
    
    if (error < 0) {
	ksock_seterror();
	KTOOLS_ERROR_PUSH("cannot create socket");
	return -1;
    }
    
    *fd = error;
    return 0;
}

void ksock_close(int *fd) {
    if (*fd == -1) return;
    close(*fd);
    *fd = -1;
}

int ksock_set_unblocking(int fd) {
    int flag;
    
    if ((flag = fcntl(fd, F_GETFL)) == -1 || fcntl(fd, F_SETFL, flag | O_NONBLOCK) == -1) {
	ksock_seterror();
        KTOOLS_ERROR_PUSH("cannot set socket unblocking");
	return -1;
    }
    
    return 0;
}

int ksock_set_blocking(int fd) {
    int flag;
    
    if ((flag = fcntl(fd, F_GETFL)) == -1 || fcntl(fd, F_SETFL, flag & ~O_NONBLOCK) == -1) {
	ksock_seterror();
        KTOOLS_ERROR_PUSH("cannot set socket blocking");
	return -1;
    }
    
    return 0;
}

int ksock_bind(int fd, char *iface, int port) {
    struct sockaddr_in addr;
    int reuse_flag = 1;
    
    /* Allow us to reuse the port. */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse_flag, sizeof(reuse_flag)) != 0) {
	ksock_seterror();
        KTOOLS_ERROR_PUSH("cannot set socket reuse-port flag");
	return -1;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (! inet_aton(iface, &addr.sin_addr)) {
        KTOOLS_ERROR_SET("cannot bind socket to invalid address %s", iface);
	return -1;
    }

    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
	ksock_seterror();
        KTOOLS_ERROR_PUSH("cannot bind socket to port %d", port);
        return -1;
    }
    
    return 0;
}

int ksock_listen(int fd) {
    if (listen(fd, 10) != 0) {
	ksock_seterror();
    	KTOOLS_ERROR_PUSH("cannot listen on socket");
	return -1;
    }
    
    return 0;
}

int ksock_accept(int accept_fd, int *conn_fd) {
    assert(*conn_fd == -1);
    int error = accept(accept_fd, NULL, NULL);
    
    if (error < 0) {
    	if (errno == EWOULDBLOCK) {
    	    return -2;
	}
	 
	ksock_seterror();
	KTOOLS_ERROR_PUSH("cannot accept connection");
	return -1;
    }
    
    if (ksock_set_unblocking(error)) {
    	ksock_close(&error);
	return -1;
    }
    
    *conn_fd = error;
    return 0;
}

int ksock_get_host_addr_list(char *host, unsigned long *addr_array, int *nb_addr)
{
    /* Resolve the server address. */
    struct hostent *he = gethostbyname(host);   
    int i = 0;
    
    if (he == NULL) {
	KTOOLS_ERROR_SET("%s", hstrerror(h_errno));
    	KTOOLS_ERROR_PUSH("cannot resolve %s", host);
	return -1;
    }
    
    /* Copy the addresses. */
    while (he->h_addr_list[i] && i < *nb_addr)
    {
	addr_array[i] = ((struct in_addr *) (he->h_addr_list[i]))->s_addr;
	i++;
    }
    
    assert(i);
    *nb_addr = i;
    return 0;
}

int ksock_connect(int fd, char *host, int port) {
    int nb_addr = 1;
    unsigned long addr;
    
    if (ksock_get_host_addr_list(host, &addr, &nb_addr)) return -1;
    return ksock_connect_addr(fd, host, addr, port);
}

int ksock_connect_addr(int fd, char *host, unsigned long addr, int port)
{
    struct sockaddr_in server_addr;
    
    /* Setup the server address. */
    server_addr.sin_addr.s_addr = addr;
    server_addr.sin_port = htons(port);
    server_addr.sin_family = AF_INET;
    
    /* Try to connect. */
    if (connect(fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    	
	/* Connection in progress. */
	if (errno == EINPROGRESS || errno == EWOULDBLOCK) {
	    return 0;
	}
	
	ksock_seterror();
	KTOOLS_ERROR_PUSH("cannot connect to %s", host);
    	return -1;
    }
    
    /* We succeeded faster than we expected. Wonderful! */
    return 0;
}

int ksock_connect_check(int fd, char *host) {
    int error;
    unsigned int len = sizeof(error);

    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *) &error, &len)) {
	ksock_seterror();
        KTOOLS_ERROR_PUSH("cannot get socket option");
	return -1;
    }

    if (error != 0) {
        errno = error;
	ksock_seterror();
        KTOOLS_ERROR_PUSH("cannot connect to %s", host);
	return -1;
    }

    return 0;
}

int ksock_read(int fd, char *buf, uint32_t *len) {
    assert(*len > 0);
    int nb = read(fd, buf, *len);
    
    if (nb == 0) {
	KTOOLS_ERROR_SET("remote side closed connection");
    	KTOOLS_ERROR_PUSH("cannot read data");
	return -1;
    }
    
    else if (nb < 0) {
    	if (errno == EAGAIN) {
	    return -2;
	}
	
	ksock_seterror();
    	KTOOLS_ERROR_PUSH("cannot read data");
	return -1;
    }
    
    *len = nb;
    return 0;
}

int ksock_write(int fd, char *buf, uint32_t *len) {
    assert(*len > 0);
    int nb = write(fd, buf, *len);
    
    if (nb == 0) {
	KTOOLS_ERROR_SET("remote side closed connection");
    	KTOOLS_ERROR_PUSH("cannot send data");
	return -1;
    }
    
    else if (nb < 0) {
    	if (errno == EAGAIN) {
	    return -2;
	}
	
	ksock_seterror();
    	KTOOLS_ERROR_PUSH("cannot send data");
	return -1;
    }
    
    *len = nb;
    return 0;
}

int ksock_enable_keepalive(int fd, int keepalive_time, int keepalive_intvl, int keepalive_probes) {
   int enable = 1;
   
    if (setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &keepalive_time, sizeof(int)) == -1 ||
        setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &keepalive_intvl, sizeof(int)) == -1 ||
        setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &keepalive_probes, sizeof(int)) == -1 ||
        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(int)) == -1) {
        ksock_seterror();
    	KTOOLS_ERROR_PUSH("cannot enable keepalives");
        return -1;
    }
    
    return 0;
}

