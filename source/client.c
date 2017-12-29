/*************************************************************************
   > File Name: source/client.c
   > Author: 范京凯
   > Mail: sunnyfjk@gmail.com
   > Created Time: 2017年12月29日 星期五 15时36分50秒
 ************************************************************************/

#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <event2/thread.h>
#include <event2/listener.h>
#include <client.h>

#ifndef PERR

#define PERR(fmt,arg ...) fprintf(stderr,"[%s:%d]"fmt,__FUNCTION__,__LINE__, ## arg)

#endif

int ClientCreate(struct Client_t *c)
{
        int ret=-1;
        struct sockaddr_in saddr;

        evthread_use_pthreads();

        memset(&saddr,0,sizeof(saddr));
        saddr.sin_family=AF_INET;
        saddr.sin_port=htons(c->port);
        inet_pton(AF_INET,c->ip,&saddr.sin_addr);

        c->base=event_base_new();
        if(c->base==NULL) {
                ret=-1;
                PERR("event_base_new_err\n");
                goto event_base_new_err;
        }
        struct bufferevent* bev =bufferevent_socket_new(c->base,-1,BEV_OPT_CLOSE_ON_FREE);
        if(bev==NULL) {
                ret=-2;
                PERR("bufferevent_socket_new_err\n");
                goto bufferevent_socket_new_err;
        }
        //监听终端输入事件
        struct event* ev_cmd = event_new(c->base, STDIN_FILENO,EV_READ | EV_PERSIST, cmd_msg_cb, (void*)bev);
        if(ev_cmd==NULL)
        {
                ret=-3;
                PERR("cmd_event_new_err\n");
                goto cmd_event_new_err;
        }
        ret=event_add(ev_cmd, NULL);
        if(ret<0) {
                ret=-4;
                PERR("cmd_event_add_err\n");
                goto cmd_event_add_err;
        }
        ret=bufferevent_socket_connect(bev,(struct sockaddr *)&saddr,sizeof(saddr));
        if(ret<0) {
                ret=-5;
                PERR("bufferevent_socket_connect_err\n");
                goto bufferevent_socket_connect_err;
        }
        bufferevent_setcb(bev, server_msg_cb, NULL, event_cb, (void *)ev_cmd);
        bufferevent_enable(bev, EV_READ | EV_PERSIST);
        c->signal_event=evsignal_new(c->base, SIGINT, ServerSignalEvent, c);
        if(c->signal_event==NULL) {
                PERR("evsignal_new_err\n");
                goto evsignal_new_err;
        }
        ret=event_add(c->signal_event, NULL);
        if(ret<0) {
                PERR("event_add_err\n");
                goto event_add_err;
        }

        ret=event_base_dispatch(c->base);
        if(ret<0) {
                ret=-7;
                PERR("event_base_dispatch_err\n");
                goto event_base_dispatch_err;
        }
        return 0;
event_base_dispatch_err:
        event_del(c->signal_event);
event_add_err:
        event_free(c->signal_event);
evsignal_new_err:
bufferevent_socket_connect_err:
        event_del(ev_cmd);
cmd_event_add_err:
        event_free(ev_cmd);
cmd_event_new_err:
        bufferevent_free(bev);
bufferevent_socket_new_err:
        event_base_free(c->base);
event_base_new_err:
        return ret;
}
void ClientClose(struct Client_t *c)
{
        event_del(c->signal_event);
        event_free(c->signal_event);
        event_base_free(c->base);
}
void cmd_msg_cb(int fd, short events, void * arg)
{
        char msg[1024]={0};
        struct bufferevent *bev=(struct bufferevent *)arg;
        int ret=read(fd,msg,sizeof(msg));
        if(ret<0)
        {
                PERR("file read err\n");
                return;
        }
        bufferevent_write(bev,msg,ret);
}
void event_cb( struct bufferevent *bev, short event, void *arg)
{

        if (event & BEV_EVENT_EOF)
                printf("connection closed\n");
        else if (event & BEV_EVENT_ERROR)
                printf("some other error\n");
        else if (event & BEV_EVENT_CONNECTED) {
                printf("the client has connected to server\n");
                return;
        }
        bufferevent_free(bev);
        struct event *ev = ( struct event*)arg;
        event_free(ev);

}
void server_msg_cb( struct bufferevent* bev, void * arg)
{
        char msg[1024]={0};
        size_t len = bufferevent_read(bev, msg, sizeof (msg));
        if(len<0)
                return;
        msg[1024] = '\0';
        printf("recv [%s] from server\n", msg);
}
void ServerSignalEvent(evutil_socket_t sig, short events, void * arg){
        struct Client_t *c=arg;
        struct timeval delay={2,0};
        event_base_loopexit(c->base,&delay);
}
