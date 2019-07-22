/*************************************************************************
   > File Name: main.c
   > Author: 范京凯
   > Mail: sunnyfjk@gmail.com
   > Created Time: 2017年12月29日 星期五 15时46分21秒
 ************************************************************************/

#include <stdio.h>
#include <client.h>
int main(void)
{
        struct Client_t c;
        c.ip="104.224.158.153";
        c.port=9999;
        ClientCreate(&c);
        ClientClose(&c);
	



        return 0;
}
