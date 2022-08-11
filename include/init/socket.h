#include <netex/ifctl.h>

#define getPort(p1x, p2x) ((p1x * 256) + p2x)

static int ssend(int socket, const char *str)
{
	return send(socket, str, strlen(str), 0);
}

static int connect_to_server_ex(const char *server_ip, u16 port, u8 rcv_timeout)
{
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0)
	{
		return FAILED;
	}

	struct sockaddr_in sin;
	socklen_t sin_len = sizeof(sin);
	_memset(&sin, sin_len);

	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	unsigned int ip_address;

	if((ip_address = inet_addr(server_ip)) != (unsigned int)-1)
	{
		sin.sin_addr.s_addr = ip_address;
	}
	else
	{
		struct hostent *hp;

		if((hp = gethostbyname(server_ip)) == NULL)
		{
			return FAILED;
		}

		sin.sin_family = hp->h_addrtype;
		memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
	}

	struct timeval tv;
	tv.tv_usec = 0;
	tv.tv_sec = rcv_timeout ? rcv_timeout : 30;
	setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

	if(rcv_timeout)
		setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	if(connect(s, (struct sockaddr *)&sin, sin_len) < 0)
	{
		return FAILED;
	}

	tv.tv_sec = 60;
	setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	//setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	return s;
}

static int connect_to_server(const char *server_ip, u16 port)
{
	return connect_to_server_ex(server_ip, port, false);
}

static int slisten(int port, int backlog)
{
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0)
	{
		return FAILED;
	}

	//int optval = 1;
	//setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	//setsockopt(s, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
	//setsockopt(s, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));

	struct sockaddr_in sa;
	socklen_t sin_len = sizeof(sa);
	_memset(&sa, sin_len);

	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(s, (struct sockaddr *)&sa, sin_len);
	listen(s, backlog);

	return s;
}

static void sclose(int *socket_e)
{
	if(*socket_e != NONE)
	{
		shutdown(*socket_e, SHUT_RDWR);
		socketclose(*socket_e);
		*socket_e = NONE;
	}
}

#define MIN_KERNEL_FREE_SPACE 256*1024 // 256KiB
#define MIN_KERNEL_MAX_TRIES 10 // Try 10 times
#define MIN_KERNEL_RETRY_PAUSE 2 // 2 seconds

static bool wait_for_new_socket(void)
{
	int failures = 0;
	while(true)
	{
		unsigned int kernel_free_current = 0;
		sys_net_if_ctl(0, SYS_NET_CC_GET_MEMORY_FREE_CURRENT, &kernel_free_current, sizeof(kernel_free_current));

		if(kernel_free_current >= MIN_KERNEL_FREE_SPACE)
		{
			break; // There's enough space for new socket
		}
		else
		{
			failures++;

			if(failures >= MIN_KERNEL_MAX_TRIES)
			{
				return true; // Kernel is full: Not enough space found
			}

			sys_ppu_thread_sleep(MIN_KERNEL_RETRY_PAUSE); // Give kernel more time to free up space
		}
	}
	return false;
}
