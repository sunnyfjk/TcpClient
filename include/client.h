/*************************************************************************
   > File Name: include/client.h
   > Author: 范京凯
   > Mail: sunnyfjk@gmail.com
   > Created Time: 2017年12月29日 星期五 15时36分50秒
 ************************************************************************/

#ifndef _INCLUDE_CLIENT_H
#define _INCLUDE_CLIENT_H

#include <event.h>

struct Client_t {
        const char *ip;
        int port;
        struct event_base *base;
        struct event *signal_event;
};
int ClientCreate(struct Client_t *c);
void ClientClose(struct Client_t *c);
void cmd_msg_cb(int fd, short events, void * arg);
void event_cb( struct bufferevent *bev, short event, void *arg);
void server_msg_cb( struct bufferevent* bev, void * arg);
void ServerSignalEvent(evutil_socket_t sig, short events, void * arg);
#endif
