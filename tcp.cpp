/*  mDownloader - a multiple-threads downloading accelerator program that is based on Myget.
 *  Homepage: http://qinchuan.me/article.php?id=100
 *  2015 By Richard (qc2105@qq.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Myget - A download accelerator for GNU/Linux
 *  Homepage: http://myget.sf.net
 *  2005 by xiaosuo
 */

#include <errno.h>
#include <cstring>
#include <stdio.h>

#include "tcp.h"
#include "debug.h"
#include <iostream>
using namespace std;

/********************************
 * class TcpSockAddr implement
 * ******************************/
TcpSockAddr::TcpSockAddr(int family)
{
	ai_family = family;
    memset((void *)&ai_addr, 0x0,sizeof(ai_addr));
	if(ai_family == AF_INET){
		ai_addrlen = sizeof(struct sockaddr_in);
		((struct sockaddr_in*)&ai_addr)->sin_family = AF_INET;
	}else if(ai_family == AF_INET6){
		ai_addrlen = sizeof(struct sockaddr_in6);
		((struct sockaddr_in6*)&ai_addr)->sin6_family = AF_INET6;
	}else{
		ai_addrlen = sizeof(struct sockaddr);
	}
};

void
TcpSockAddr::set_family(int family)
{
	ai_family = family;
	if(ai_family == AF_INET){
		ai_addrlen = sizeof(struct sockaddr_in);
		((struct sockaddr_in*)&ai_addr)->sin_family = AF_INET;
	}else if(ai_family == AF_INET6){
		ai_addrlen = sizeof(struct sockaddr_in6);
		((struct sockaddr_in6*)&ai_addr)->sin6_family = AF_INET6;
	}else{
		ai_addrlen = sizeof(struct sockaddr);
	}
};

void
TcpSockAddr::set_port(int port)
{
	if(ai_family == AF_INET)
		((struct sockaddr_in*)&ai_addr)->sin_port = htons(port);
	else
		((struct sockaddr_in6*)&ai_addr)->sin6_port = htons(port);
};

int
TcpSockAddr::get_port()
{
	if(ai_family == AF_INET)
		return ntohs(((struct sockaddr_in*)&ai_addr)->sin_port);
	else
		return ntohs(((struct sockaddr_in6*)&ai_addr)->sin6_port);
};

int
TcpSockAddr::set_addr(const char *addr)
{
	if(addr == NULL) return -1;
	if(ai_family == AF_INET){
		if(inet_pton(AF_INET, addr,
					&((struct sockaddr_in*)&ai_addr)->sin_addr) <= 0)
			return -1;
	}else if(ai_family == AF_INET6){
		if(inet_pton(AF_INET6, addr,
					&((struct sockaddr_in6*)&ai_addr)->sin6_addr) <=0)
			return -1;
	}else
		return -1;
	return 0;
};

int
TcpSockAddr::get_addr(char *addr, int size)
{
	if(addr == NULL) return -1;
	if(ai_family == AF_INET && size >= INET_ADDRSTRLEN){
		if(inet_ntop(AF_INET,
					&((struct sockaddr_in*)&ai_addr)->sin_addr,
					addr, INET_ADDRSTRLEN) == NULL)
			return -1;
	}else if(ai_family == AF_INET6 && size >= INET6_ADDRSTRLEN){
		if(inet_ntop(AF_INET6,
					&((struct sockaddr_in6*)&ai_addr)->sin6_addr,
					addr, INET6_ADDRSTRLEN) == NULL)
			return -1;
	}else
		return -1;
	return 0;
};

/**********************************
 * class Address implement
 * *******************************/
int
Address::resolve(const char *dns_name, int port, int family)
{
	char serv[12];
	int n;
	struct addrinfo hints;

	freeaddrinfo(addr);

    if(_snprintf(serv, 12, "%d", port) < 0) return -1;
    memset(&hints, 0x0, sizeof(struct addrinfo));
	hints.ai_family = family; 
	hints.ai_socktype = SOCK_STREAM;

	if((n = getaddrinfo(dns_name, serv, &hints, &addr)) != 0){
		debug_log("getaddrinfo error for %s, %s, %s\n",
				dns_name, serv, gai_strerror(n));
		return -1;
	}

	return 0;
};

/********************************
 * class TcpConnection implement
 * *****************************/
#define IPTOS_THROUGHPUT 0x8
int
TcpConnection::set_tos(void)
{
	int tos = IPTOS_THROUGHPUT;
    return setsockopt(fd, IPPROTO_IP, IP_TOS, (char *) &tos, sizeof(tos));
}

bool
TcpConnection::is_connected()
{
    char error;
	socklen_t errorlen = sizeof(error);

	// not open any connection
	if(fd < 0) return false;
	// so_error will be cleared by calling read and write
	if(errno == EPIPE) return false;
	// some other async errors ?
	if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &errorlen) < 0){
		return false;
	}
	if(error == 0) return true;
	return false;
}

int
TcpConnection::get_remote_addr(TcpSockAddr& sockaddr)const
{
	if(getpeername(fd,
				(struct sockaddr*)&sockaddr.ai_addr,
				&sockaddr.ai_addrlen) < 0)
		return -1;
	if(sockaddr.ai_addrlen == 16)// if addrlen == 16bytes ipv4
		sockaddr.set_family(AF_INET);
	else // if addrlen == 24bytes ipv6
		sockaddr.set_family(AF_INET6);
	return 0;

};

int
TcpConnection::get_local_addr(TcpSockAddr& sockaddr)const
{
	if(getsockname(fd,
				(struct sockaddr*)&sockaddr.ai_addr,
				&sockaddr.ai_addrlen) < 0)
		return -1;
	if(sockaddr.ai_addrlen == 16)// if addrlen == 16bytes ipv4
		sockaddr.set_family(AF_INET);
	else // if addrlen == 24bytes ipv6
		sockaddr.set_family(AF_INET6);
	return 0;
};

/*************************************
 * class TcpConnector implement
 * **********************************/

// this function will used in class TcpConnector
int
timeout_connect(int sock_fd, struct sockaddr* serv_addr, int addrlen, long tsec)
{

    int flag, ret;
    char *error;
	socklen_t len = sizeof(error);
	struct timeval timeout;
	fd_set wrset;
	timeout.tv_sec = tsec;
	timeout.tv_usec = 0;
	ret = -1;

	while(1)
	{
        ret=connect(sock_fd, serv_addr, addrlen);
		if(ret<0){
            cerr << "timeout_connect connect failed: ret" << ret << endl;
			if(errno == EINTR)
				continue;
			else if(errno != EINPROGRESS){
                cerr << "errno != EINPROGRESS";
				break;
			}
		}

		while(1){
			FD_ZERO(&wrset);
			FD_SET(sock_fd, &wrset);
			switch(select(sock_fd+1, NULL, &wrset, NULL, &timeout)){
				case -1:
					if(errno == EINTR) continue;
					break;
				case 0:
					ret = -2;
					break;
				default:
					if(FD_ISSET(sock_fd, &wrset)){
                        int rt;
                        error = (char *)calloc(20, sizeof(char));
                        if((rt = getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, error, &len)) <0){
                            cerr << "getsocket rt: " << rt << endl;
                            cerr << error << endl;
                            ret = -1;
                        }else{
                            if(rt == 0){/* connected */
                                ret = 0;
                            }else
                            {
                                cerr << "getsockopt ret: " << rt << endl;
                                ret = -1;
                            }
                        }

						break;
					}else
						continue;
			}
			break;
		}
		break;
	};

    cerr << "timeout_connet ret: " << ret << endl;
	return ret;
};

TcpConnection*
TcpConnector::connect(const Address& addr, int& ret, long timeout)
{

	int sockfd;
	struct addrinfo *res;
	
	res = addr.addr;
	ret = -1;
    if(res == NULL)
    {
        cerr << "TcpConnector::connect res NULL" << endl;
        return 0L;
    }

    int i = 0;
	do{
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        //cerr << "sockfd: " << sockfd << endl;
		if(sockfd < 0)
        {
            ++i;
            cerr << endl <<  i << "th res socket failed" << endl;
            continue;
        }

		if(timeout < 0){
			if((ret=::connect(sockfd, res->ai_addr, res->ai_addrlen)) == 0)
            {
				break;
            }
            else
            {
                 cerr << "upper tcp connect ret: " << ret << endl;
            }
		}else{
			if((ret=timeout_connect(sockfd, res->ai_addr, res->ai_addrlen, timeout)) == 0)
            {

				break;
            }
            else
            {
               // cerr << "upper tcp timeout_connect ret: " << ret << endl;
            }
		}
        closesocket(sockfd);
	}while((res = res->ai_next) != NULL);

	if(res){
		TcpConnection *tcon;
		tcon = new TcpConnection;
		if(tcon->set_fd(sockfd) < 0){
			delete tcon;
            cerr << "Fucking upper tcon" << endl;
			return 0L;
		}else{
              cerr << "time out connect tcon: " << tcon << endl;
             return tcon;
		}
	}else{
        //cerr << "fucking final branch" << endl;
		return 0L;
	}
};

TcpConnection*
TcpConnector::connect(const TcpSockAddr& addr, int &ret, long timeout)
{
	int sockfd;

	ret = -1;
	sockfd = socket(addr.ai_family, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        cerr << "TcpConnector::connect failed" << endl;
        return 0L;
    }
	
    if(timeout < 0) {
		ret = ::connect(sockfd, (struct sockaddr*)&addr.ai_addr, addr.ai_addrlen);
        cerr << "tcp connectret ret: " << ret << endl;
    }
	else
    {
		ret = timeout_connect(sockfd, (struct sockaddr*)&addr.ai_addr, addr.ai_addrlen, timeout);
        cerr << "tcp timeout_connect ret: " << ret << endl;
    }
	
	if(ret < 0){
        cerr << "will closesocket in TcpConnector::connect" << endl;
        closesocket(sockfd);
		return 0L;
	}else{
		TcpConnection *tcon;
		tcon = new TcpConnection;
        int rt;
        if( (rt=tcon->set_fd(sockfd)) < 0){
			delete tcon;
            cerr << "tcon->set fd faild, rt" << rt << endl;
			return 0L;
		}else{
            cerr << "connect tcon: " << tcon << endl;
			return tcon;
		}
	}

};

/**************************************
 * class TcpAcceptor implement
 * *****************************/

// if bind port == 0, when binded it will return a random port */
int
TcpAcceptor::get_bind_port()
{
	socklen_t len;
	char buf[24];

	len = 24;
	if(getsockname(listen_fd, (struct sockaddr*)&buf, &len) < 0)
		return 0;
	if(len == 16) //ipv4
		return ntohs(((struct sockaddr_in*)&buf)->sin_port);
	else if(len == 24) //ipv6
		return ntohs(((struct sockaddr_in6*)&buf)->sin6_port);
	else
		return 0;
};

int
TcpAcceptor::listen(const TcpSockAddr& local, int backlog)
{
    char reuse = 1;

	listen_fd = socket(local.ai_family, SOCK_STREAM, 0);
	if(listen_fd < 0) return -1;

    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	if(bind(listen_fd, (struct sockaddr *)&(local.ai_addr), local.ai_addrlen) < 0)
		return -1;

	if(::listen(listen_fd, backlog) < 0)
		return -1;

	return 0;
};

TcpConnection* 
TcpAcceptor::accept(int &ret, long timeout)
{
	struct timeval *ptimeout;
	struct timeval time;
	fd_set r_fdset;
	int conn_fd;
	TcpConnection *conn;

	if(timeout < 0)//block
		ptimeout = NULL;
	else{
		time.tv_sec = timeout;
		time.tv_usec = 0;
		ptimeout = &time;
	}

	while(1){
		FD_ZERO(&r_fdset);
		FD_SET(listen_fd, &r_fdset);
		switch(select(listen_fd + 1, &r_fdset, NULL, NULL, ptimeout)){
			case 0:
				ret = E_TIMEOUT;
				return 0L;
			case -1:
				ret = -1;
				return 0L;
			default:
				if(FD_ISSET(listen_fd, &r_fdset))
					break;
				else
					continue;
		}
		break;
	}

	if((conn_fd=::accept(listen_fd, NULL, NULL)) < 0){
		ret = -1;
		return 0L;
	}

	conn = new TcpConnection;
	if(conn->set_fd(conn_fd) < 0){
		delete conn;
		return 0L;
	}else{
		return conn;
	}
};

TcpConnection*
TcpAcceptor::accept(const TcpSockAddr& expect, int &ret, long timeout)
{
	TcpConnection* conn;
	TcpSockAddr peer_sock;

	conn = accept(ret, timeout);
	if(!conn) return 0L;

	conn->get_remote_addr(peer_sock);

	if(peer_sock.ai_family == expect.ai_family){
		if(peer_sock.ai_family == AF_INET){
			// so dirty ~_~
			if(!memcmp(&(((struct sockaddr_in*)&(peer_sock.ai_addr))->sin_addr),
					&(((struct sockaddr_in*)&(expect.ai_addr))->sin_addr),
                    sizeof(struct in_addr))) {
                ;
            }
				return conn;
		}else if(peer_sock.ai_family == AF_INET6){
			// so dirty ~_~
			if(!memcmp(&(((struct sockaddr_in6*)&(peer_sock.ai_addr))->sin6_addr),
					&(((struct sockaddr_in6*)&(expect.ai_addr))->sin6_addr),
                    sizeof(struct in6_addr))) {
                ;
            }
				return conn;
		}
	}

	delete conn;

	return 0L;
};
