#include "volc_socket.h"

typedef struct {
    int count;
    int capacity;
    struct pollfd *pollfds;
} volc_poll_impl_t;

static uint32_t _volc_ip_addr_to_socket_addr(const volc_ip_addr_t *p_ip_address, struct sockaddr_in *p_addr)
{
    uint32_t ret = VOLC_STATUS_SUCCESS;
    VOLC_CHK(p_ip_address != NULL, VOLC_STATUS_NULL_ARG);
    if (p_ip_address->family == VOLC_IP_FAMILY_TYPE_IPV4) {
        p_addr->sin_family = AF_INET;
        p_addr->sin_port = (p_ip_address->port);
        memcpy(&p_addr->sin_addr, p_ip_address->address, VOLC_IPV4_ADDRESS_LENGTH);
    } else if (p_ip_address->family == VOLC_IP_FAMILY_TYPE_IPV6) {
        p_addr->sin_family = AF_INET6;
    } else {
        ret = VOLC_STATUS_INVALID_ARG;
    }
err_out_label:
    return ret;
}

static uint32_t _volc_ip_addr_from_socket_addr(volc_ip_addr_t *p_ip_address, const struct sockaddr_in *p_addr)
{
    uint32_t ret = VOLC_STATUS_SUCCESS;
    VOLC_CHK(p_ip_address != NULL && p_addr != NULL, VOLC_STATUS_NULL_ARG);
    if (p_addr->sin_family == AF_INET) {
        p_ip_address->family = VOLC_IP_FAMILY_TYPE_IPV4;
        p_ip_address->port = (p_addr->sin_port);
        memcpy(p_ip_address->address, &p_addr->sin_addr, VOLC_IPV4_ADDRESS_LENGTH);
    } else if (p_addr->sin_family == AF_INET6) {
        p_ip_address->family = VOLC_IP_FAMILY_TYPE_IPV6;
        VOLC_CHK(0, VOLC_STATUS_FAILURE);
    } else {
        ret = VOLC_STATUS_INVALID_ARG;
    }
err_out_label:
    return ret;
}

int volc_socket(int __domain, int __type, int __protocol)
{
    // volc_debug("%s\n", __FUNCTION__);
    volc_debug("__type:%d, protocol:%d", __type, __protocol);
    int domain = (__domain == VOLC_IP_FAMILY_TYPE_IPV4) ? AF_INET : AF_INET6;
    int type = (__type == VOLC_SOCK_DGRAM) ? SOCK_DGRAM : SOCK_STREAM;
    return socket(domain, type, __protocol);
}

int volc_bind(int __fd, const volc_ip_addr_t *__addr)
{
    // volc_debug("%s\n", __FUNCTION__);
    struct sockaddr_in addr;
    _volc_ip_addr_to_socket_addr(__addr, &addr);
    return bind(__fd, (struct sockaddr *)&addr, sizeof(addr));
}

int volc_listen(int __fd, int __n)
{
    // volc_debug("%s\n", __FUNCTION__);
    return listen(__fd, __n);
}

int volc_accept(int __fd, volc_ip_addr_t *__addr, int *__addr_len)
{
    // volc_debug("%s\n", __FUNCTION__);
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int ret = accept(__fd, (struct sockaddr *)&addr, &addr_len);
    _volc_ip_addr_from_socket_addr(__addr, &addr);
    if (__addr_len) {
        *__addr_len = addr_len;
    }
    return ret;
};

int volc_connect(int __fd, volc_ip_addr_t *__addr)
{
    // volc_debug("%s\n", __FUNCTION__);
    uint32_t ret_status = VOLC_STATUS_SUCCESS;
    int r = 0;
    struct sockaddr_in addr = {0};
    _volc_ip_addr_to_socket_addr(__addr, &addr);
    do {
        errno = 0;
        char ip[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
        r = connect(__fd, (struct sockaddr *)&addr, (socklen_t)sizeof(addr));
    } while (r == -1 && errno == EINTR);

    if (r == -1 && errno != EINPROGRESS)  {
        ret_status =  VOLC_STATUS_EVLOOP_PERFORM_FAILED;
    }

    if (r == -1 && errno == EINPROGRESS) {
        ret_status =  VOLC_STATUS_EVLOOP_PERFORM_NEED_RETRY;
    }

    return ret_status;
}

ssize_t volc_recv_msg(int __fd, void *data, size_t size, volc_ip_addr_t *p_addr, uint32_t *p_status)
{
    // volc_debug("%s\n", __FUNCTION__);
    uint32_t ret_status = VOLC_STATUS_SUCCESS;
    struct msghdr msg = {0};
    msg.msg_flags = 0;
    struct iovec iov = {.iov_base = data, .iov_len = size};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    struct sockaddr_storage peer;
    if (p_addr != NULL) {
        msg.msg_name = (struct sockaddr *)&peer;
        msg.msg_namelen = sizeof(peer);
    }

    int r = 0;
    int times = 0;
    do {
        r = recvmsg(__fd, &msg, 0);
    } while (r < 0 && errno == EINTR);
    if (r > 0) {
        ret_status = VOLC_STATUS_SUCCESS;
        if (p_addr != NULL) {
            _volc_ip_addr_from_socket_addr(p_addr, &peer);
        }
    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        ret_status = VOLC_STATUS_EVLOOP_PERFORM_NEED_RETRY;
    } else if (r < 0) {
        ret_status = VOLC_STATUS_EVLOOP_PERFORM_FAILED;
    }
    if (p_status  != NULL) {
        *p_status = ret_status;
    };
    return r;
};

ssize_t volc_send_msg(int __fd, void *data, size_t size, volc_ip_addr_t *__addr, uint32_t *p_status)
{
    struct msghdr msg = {0};
    msg.msg_flags = 0;
    struct iovec iov = {.iov_base = data, .iov_len = size};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    struct sockaddr_in addr;
    _volc_ip_addr_to_socket_addr(__addr, &addr);

    int r = 0;
    int times = 0;
    do {
        char ip[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
        r =  sendto(__fd, data, size, 0, (struct sockaddr *)&addr, (socklen_t)sizeof(addr));
    } while (r < 0 && errno == EINTR);

    uint32_t ret_status = VOLC_STATUS_SUCCESS;
    if (r > 0) {
//        printf("%s %d\n", __FUNCTION__, __LINE__);
        ret_status = VOLC_STATUS_SUCCESS;
    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
//        printf("%s %d\n", __FUNCTION__, __LINE__);
        ret_status = VOLC_STATUS_EVLOOP_PERFORM_NEED_RETRY;
    } else if (r < 0) {
//        printf("%s %d\n", __FUNCTION__, __LINE__);
        ret_status = VOLC_STATUS_EVLOOP_PERFORM_FAILED;
    };

    if (p_status  != NULL) {
        *p_status = ret_status;
    };
//    printf("%s %d\n", __FUNCTION__, __LINE__);
    return r;
}


int volc_close(int __fd)
{
    // volc_debug("%s\n", __FUNCTION__);
    return close(__fd);
};

int volc_set_nonblocking(int __fd)
{
    // volc_debug("%s\n", __FUNCTION__);
    uint32_t ret = VOLC_STATUS_SUCCESS;
    int flags = fcntl(__fd, F_GETFL, 0);
    VOLC_CHK(flags != -1, VOLC_STATUS_INVALID_ARG);
    flags = fcntl(__fd, F_SETFL, flags | O_NONBLOCK);
    VOLC_CHK(flags != -1, VOLC_STATUS_INVALID_ARG);
err_out_label:
//    volc_debug("set nonblocking error");
    return ret;
}

int volc_make_pipe(int fds[2])
{
    volc_debug("%s\n", __FUNCTION__);
    uint32_t ret = VOLC_STATUS_SUCCESS;
    fds[0] = -1;
    fds[1] = -1;
    int err = 0, opt = 0, write_fd = -1, read_fd = -1;
    volc_debug("%s %d", __FUNCTION__, __LINE__);
    read_fd =  socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    volc_debug("%s %d, read_fd:%d", __FUNCTION__, __LINE__, read_fd);

    VOLC_CHK(read_fd >= 0, VOLC_STATUS_INVALID_ARG);
//    volc_debug("%s %d", __FUNCTION__, __LINE__);
    struct sockaddr_in servaddr;
    uint32_t addr_len = sizeof(servaddr);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = 0;

    volc_debug("%s %d", __FUNCTION__, __LINE__);
    int bind_ret = bind(read_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    VOLC_CHK(bind_ret >= 0, VOLC_STATUS_SOCKET_BIND_FAILED);
    // get port bind by system
    int getsockname_ret = getsockname(read_fd, (struct sockaddr *)&servaddr, &addr_len);
    VOLC_CHK(getsockname_ret >= 0, VOLC_STATUS_SOCKET_GETSOCKETNAME_FAILED);

    if (servaddr.sin_addr.s_addr == htonl(INADDR_ANY)) {
        servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }

    write_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int conn_ret = connect(write_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    VOLC_CHK(conn_ret >= 0, VOLC_STATUS_INVALID_ARG);
    volc_set_nonblocking(write_fd);
    volc_set_nonblocking(read_fd);
    fds[0] = read_fd;
    fds[1] = write_fd;
    volc_debug("ret:%d fd0:%d, fd1:%d", ret, fds[0], fds[1]);
err_out_label:
    if (VOLC_STATUS_FAILED(ret)) {
        if (read_fd > 0) {
            close(read_fd);
        }
        if (write_fd > 0) {
            close(write_fd);
        }
    }
    return ret;
}

int volc_poll(struct volc_pollfd *fds, int nfds, int timeout)
{
    // 目前我们最多用3个，该值定义大于2即可
    struct pollfd inner_fds[10] = {0};
    int poll_ret = 0;
    for (int i = 0; i < nfds; i++) {
        inner_fds[i].fd = fds[i].fd;
        if (fds[i].events & VOLC_EVLOOP_POLLIN) {
            inner_fds[i].events |= POLLIN;
        }
        if (fds[i].events & VOLC_EVLOOP_POLLOUT) {
            inner_fds[i].events |= POLLOUT;
        }
        if (fds[i].events & VOLC_EVLOOP_POLLERR) {
            inner_fds[i].events |= POLLERR;
        }
        if (fds[i].events & VOLC_EVLOOP_POLLHUP) {
            inner_fds[i].events |= POLLHUP;
        }
    }

    poll_ret = poll(inner_fds, nfds, timeout);

    for (int i = 0; i < nfds; i++) {
        fds[i].revents = 0;
        if (inner_fds[i].revents & POLLIN) {
            fds[i].revents |= VOLC_EVLOOP_POLLIN;
        }
        if (inner_fds[i].revents & POLLOUT) {
            fds[i].revents |= VOLC_EVLOOP_POLLOUT;
        }
        if (inner_fds[i].revents & POLLERR) {
            fds[i].revents |= VOLC_EVLOOP_POLLERR;
        }
        if (inner_fds[i].revents & POLLHUP) {
            fds[i].revents |= VOLC_EVLOOP_POLLHUP;
        }
    }
    return poll_ret;
}


int volc_write(int __fd, void *data, size_t size)
{
//    printf("%s %d\n", __FUNCTION__, __LINE__);
    // volc_debug("%s\n", __FUNCTION__);
    uint32_t ret_status = VOLC_STATUS_SUCCESS;
    int r = 0;
    int times = 0;
    do {
        errno = 0;
        r = write(__fd, data, size);
    } while (r == -1 && errno == EINTR);
    if (r == size) {
        ret_status = VOLC_STATUS_SUCCESS;
    }

    if (r == -1 && errno != EINPROGRESS)  {
        ret_status =  VOLC_STATUS_EVLOOP_PERFORM_FAILED;
    }

    if (r == -1 && errno == EINPROGRESS) {
        ret_status =  VOLC_STATUS_EVLOOP_PERFORM_NEED_RETRY;
    }

    return ret_status;
}

int volc_read(int __fd, void *data, size_t size)
{
//    printf("%s %d\n", __FUNCTION__, __LINE__);
    // volc_debug("%s\n", __FUNCTION__);
    int r = 0;
    int times = 0;
    do {
        errno = 0;
        r = read(__fd, data, size);
    } while (r == -1 && errno == EINTR);

    return r;
}

int volc_sockopt_set_buffer_size(int __fd, bool _is_send_buffer, int buffer_size)
{
    int len = sizeof(buffer_size);
    int opt_name = SO_SNDBUF;
    if (!_is_send_buffer) {
        opt_name = SO_RCVBUF;
    }
    return setsockopt(__fd, SOL_SOCKET, opt_name, &buffer_size, len);
};

int volc_sockopt_get_buffer_size(int __fd, bool _is_send_buffer)
{
    int opt_name = SO_SNDBUF;
    if (!_is_send_buffer) {
        opt_name = SO_RCVBUF;
    }
    int buffer_size = 0;
    int len = sizeof(buffer_size);
    return getsockopt(__fd, SOL_SOCKET, opt_name, &buffer_size, &len);
} ;

int volc_getaddrinfo(const char *host, uint16_t port, volc_ip_addr_t **addrs, int *count)
{
    int index = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    struct addrinfo *res = NULL;
    struct addrinfo *cur = NULL;

    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_protocol   = IPPROTO_TCP;
    hints.ai_flags = AI_CANONNAME | AI_ADDRCONFIG;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[10] = {0};
    if (NULL == count) {
        return VOLC_FAILED;
    }

    snprintf(port_str, sizeof(port_str), "%d", (int)port);
    int ret = getaddrinfo(host, port_str, &hints, &res);
    if (ret < 0) {
        return VOLC_FAILED;
    }
    cur = res;
    while (cur) {
        (*count)++;
        cur = cur->ai_next;
    }
    *addrs = (volc_ip_addr_t *)volc_malloc(sizeof(volc_ip_addr_t) * (*count));
    cur = res;
    while (cur) {
        _volc_ip_addr_from_socket_addr(&(*addrs)[index], (struct sockaddr_in *)cur->ai_addr);
        index++;
        cur = cur->ai_next;
    }
    return VOLC_SUCCESS;
}

int volc_freeaddrinfo(volc_ip_addr_t *addrs)
{
    volc_debug("%s %d\n", __FUNCTION__, __LINE__);
        if (addrs) {
                volc_free(addrs);
           
    }
        return 0;
}
