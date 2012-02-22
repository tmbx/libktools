/* Copyright (C) 2006-2012 Opersys inc., All rights reserved. */

#ifndef __KSOCK_H__
#define __KSOCK_H__

#include <kutils.h>

/* Note: For all functions, the error stack is updated when code -1 is returned. */

/* This function creates a socket and sets it in 'fd' (which must be initialized
 * to -1).
 * This function returns -1 on failure, 0 on success.
 */
int ksock_create(int *fd);

/* This function closes a socket, if required. The descriptor closed is set to
 * -1.
 */
void ksock_close(int *fd);

/* This function sets the socket unblocking.
 * This function returns -1 on failure, 0 on success.
 */
int ksock_set_unblocking(int fd);

/* This function sets the socket blocking.
 * This function returns -1 on failure, 0 on success.
 */
int ksock_set_blocking(int fd);

/* This function binds the socket to the port specified on the interface
 * specified. This function returns -1 on failure, 0 on success.
 */
int ksock_bind(int fd, char *iface, int port);

/* This function makes the socket listen for connections on the port it is bound
 * to.
 * This function returns -1 on failure, 0 on success.
 */
int ksock_listen(int fd);

/* This function accepts a connection on the socket specified. On success,
 * 'conn_fd' is set to the newly created socket and this function returns 0. The
 * new socket is set unblocking. If 'accept()' failed because there is no
 * established connection at this time, this function returns -2. Otherwise,
 * this function returns -1.
 *
 * Note: this function should never block if 'accept_fd' is set non-blocking. To
 * wait for connections, use the read set of select(). Note that it is possible
 * that no connection is available even if select() says that there is one.
 */
int ksock_accept(int accept_fd, int *conn_fd);

/* This function resolves the host name 'host' and copies the addresses found to
 * 'addr_array'. Up to '*nb_addr' addresses will be copied. On success,
 * '*nb_addr' is set to the number of addresses actually copied.
 * This function returns -1 on failure, 0 on success.
 */
int ksock_get_host_addr_list(char *host, unsigned long *addr_array, int *nb_addr);

/* This function sends a connection request to the host and port specified.
 * This function returns -1 on failure, 0 on success.
 *
 * Note: this function should never block if 'fd' is set non-blocking. Thus, the
 * connection may or may not be established during the connect() call. To wait
 * for the connection to go through, use the write set and the error set of
 * select(). When select() says that the socket is ready, call
 * ksock_connect_check() to make sure the connection has been established
 * successfully.
 */
int ksock_connect(int fd, char *host, int port);

/* As above, but the address specified is used for connecting to 'host'. */
int ksock_connect_addr(int fd, char *host, unsigned long addr, int port);

/* This function checks if the connection request initiated with ksock_connect()
 * has been completed.
 * This function returns -1 on failure, 0 on success.
 */
int ksock_connect_check(int fd, char *host);

/* This function reads data from the remote side. It takes as argument a file
 * descriptor, a buffer where the data is read and an integer which specify the
 * requested number of bytes to transfer on input and the actual number of bytes
 * transferred on output. This function returns 0 on success. If no data is
 * available for reading, this function returns -2. On failure, this function
 * returns -1.
 */
int ksock_read(int fd, char *buf, uint32_t *len);

/* Same as above. */
int ksock_write(int fd, char *buf, uint32_t *len);

/* This function enables keepalives on the socket specified. keepalive_time is
 * the number of seconds of inactivity before sending a keepalive probe.
 * keepalive_intvl is the interval in seconds at which keepalives are sent until
 * a response is received. keepalive_probes is the number of sequential probes
 * that must be lost before the connection is declared dead. The
 * keepalive_probes value is ignored on Windows; it is hard-coded to 10.
 * This function returns -1 on failure, 0 on success.
 */
int ksock_enable_keepalive(int fd, int keepalive_time, int keepalive_intvl, int keepalive_probes);

#endif /*__K_SOCK_H__*/
