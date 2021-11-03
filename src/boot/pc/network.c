/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../network.h"

#include "../menu.h"

#ifdef PSXF_WIN32

#define RECT RECT_unconflict
#define POINT POINT_unconflict
#define boolean boolean_unconflict
#include <winsock2.h>
#include <ws2tcpip.h>
#undef boolean
#undef POINT
#undef RECT

#else

#include <sys/socket.h>

#define closesocket close
#define ioctlsocket ioctl

#endif

//Network state
#ifdef PSXF_WIN32
static boolean wsa_init;
static WSADATA wsa_data;
#endif

static int sock;
static struct sockaddr_storage sock_peer;
static int sock_peerlen;

static boolean is_host, is_allowed, is_ready;
static char net_pass[0x21];

#define NETWORK_PINGTIME FIXED_DEC(1,1)
#define NETWORK_DCTIME FIXED_DEC(20,1)

static fixed_t ping_time; //Time until we send a ping packet
static fixed_t ping_dctime; //Time until we DC from lack of traffic

//Internal network interface
static void Network_CloseSock(void)
{
	sock_peerlen = 0;
	closesocket(sock);
	sock = -1;
}

//Network interface
void Network_Init(void)
{
	//Initialize WSA
	#ifdef PSXF_WIN32
		wsa_init = WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0;
	#endif
	
	//Initialize network state
	sock = -1;
	sock_peerlen = 0;
	
	is_host = is_allowed = is_ready = false;
	net_pass[0] = '\0';
	ping_time = ping_dctime = 0;
}

void Network_Quit(void)
{
	//Send disconnect packet
	Packet disconnect;
	disconnect[0] = PacketType_Disconnect;
	Network_Send(&disconnect);
	
	//Close sock and deinitialize WSA
	Network_CloseSock();
	#ifdef PSXF_WIN32
		if (wsa_init)
			WSACleanup();
	#endif
}

boolean Network_HostPort(const char *port, const char *pass)
{
	//Close previous sock
	Network_CloseSock();
	
	//Get port number
	int port_int;
	if (sscanf(port, "%d", &port_int) <= 0 || port_int < 0x00000 || port_int >= 0x10000)
		return true;
	
	//Create socket
	if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
	{
		Network_CloseSock();
		return true;
	}
	
	struct sockaddr_in6 ip;
	memset(&ip, 0, sizeof(ip));
	ip.sin6_family = AF_INET6;
	ip.sin6_port = htons(port_int);
	ip.sin6_addr = in6addr_any;
	
	//Prepare socket for reading
	#ifdef PSXF_WIN32
		u_long one = 1;
		ioctlsocket(sock, FIONBIO, &one);
		DWORD zero = 0;
		setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&zero, sizeof(zero));
	#endif
	
	//Bind socket
	if (bind(sock, (struct sockaddr*)&ip, sizeof(ip)) != 0)
	{
		Network_CloseSock();
		return true;
	}
	
	//Initialize network state
	sock_peerlen = 0;
	is_host = is_allowed = true;
	is_ready = false;
	strncpy(net_pass, pass, 0x20);
	net_pass[0x20] = '\0';
	ping_time = ping_dctime = 0;
	
	return false;
}

boolean Network_Join(const char *ip, const char *port, const char *pass)
{
	//Close previous sock
	Network_CloseSock();
	
	//Create socket
	struct addrinfo *address;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_protocol = 0;
	hints.ai_flags = 0;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if (getaddrinfo(ip, port, &hints, &address) != 0)
	{
		Network_CloseSock();
		return true;
	}
	
	if ((sock = socket(address->ai_family, address->ai_socktype, address->ai_protocol)) < 0)
	{
		Network_CloseSock();
		return true;
	}
	
	memcpy(&sock_peer, address->ai_addr, sock_peerlen = address->ai_addrlen);
	
	//Prepare socket for reading
	#ifdef PSXF_WIN32
		u_long one = 1;
		ioctlsocket(sock, FIONBIO, &one);
	#endif
	
	//Initialize network state
	is_host = is_allowed = is_ready = false;
	strncpy(net_pass, pass, 0x20);
	net_pass[0x20] = '\0';
	
	//Send allow packet
	Packet allow_packet;
	allow_packet[0] = PacketType_Access;
	strncpy((char*)&allow_packet[1], pass, 0x20);
	allow_packet[1 + 0x20] = '\0';
	Network_Send(&allow_packet);
	ping_time = 0;
	ping_dctime = NETWORK_DCTIME;
	
	return false;
}

void Network_Disconnect(void)
{
	//Close sock and reset network state
	Network_CloseSock();
	is_host = is_allowed = is_ready = false;
	net_pass[0] = '\0';
}

void Network_KickPeer(void)
{
	//Send kick packet and reset network state
	Packet kick;
	kick[0] = PacketType_Disconnect;
	Network_Send(&kick);
	
	sock_peerlen = 0;
	ping_time = ping_dctime = 0;
}

void Network_Send(Packet *packet)
{
	//Send packet
	if (sock >= 0 && sock_peerlen != 0)
		sendto(sock, (char*)packet, sizeof(Packet), 0, (const struct sockaddr*)&sock_peer, sock_peerlen);
}

void Network_Process(void)
{
	if (sock >= 0)
	{
		//Receive packets
		Packet packet;
		size_t len = 0;
		
		for (;;)
		{
			//Read more data from socket
			struct sockaddr_storage recv_peer;
			int recv_peerlen;
			
			int status = recvfrom(sock, (char*)&packet + len, sizeof(Packet) - len, 0, (struct sockaddr*)&recv_peer, &recv_peerlen);
			if (status < 0)
				break;
			else
				len += status;
			
			//Check if full packet's been read
			if (len >= sizeof(packet))
			{
				//Process packet
				switch (packet[0])
				{
					case PacketType_Access:
					{
						if (is_host)
						{
							//Tell client if they're allowed
							Packet response;
							response[0] = PacketType_Access;
							
							if (sock_peerlen != 0)
							{
								//Another peer already in the server
								response[1] = false;
							}
							else if (strncmp((const char*)&packet[1], net_pass, 0x20) != 0)
							{
								//Password does not match
								response[1] = false;
							}
							else
							{
								//Allowed
								response[1] = true;
								memcpy(&sock_peer, &recv_peer, sock_peerlen = recv_peerlen);
								ping_time = NETWORK_PINGTIME;
							}
							
							sendto(sock, (char*)response, sizeof(Packet), 0, (const struct sockaddr*)&recv_peer, recv_peerlen);
						}
						else
						{
							//Update state
							if (!is_allowed)
							{
								if (packet[1])
								{
									//Allowed
									is_allowed = true;
									ping_time = NETWORK_PINGTIME;
								}
								else
								{
									//Disallowed
									Network_Disconnect();
								}
							}
						}
						break;
					}
					case PacketType_Disconnect:
					{
						//Disconnect from peer
						if (is_host)
							Network_KickPeer();
						else
							Network_Disconnect();
						break;
					}
					case PacketType_Ready:
					{
						if (is_host)
						{
							//Client is ready
							is_ready = true;
						}
						else
						{
							//Load requested stage
							if (packet[1] < StageId_Max)
							{
								is_ready = false;
								stage.mode = packet[3] ? StageMode_Net2 : StageMode_Net1;
								Menu_ToStage(packet[1], packet[2], false);
							}
						}
						break;
					}
					case PacketType_NoteHit:
					{
						//Hit note
						Stage_NetHit(&packet);
						break;
					}
					case PacketType_NoteMiss:
					{
						//Miss note
						Stage_NetMiss(&packet);
						break;
					}
					default:
						break;
				}
				if (sock_peerlen != 0)
					ping_dctime = NETWORK_DCTIME;
				
				//Reset length for next packet
				len = 0;
			}
		}
		
		//Handle pings and timeout disconnects
		if (ping_time > 0 && (ping_time -= timer_dt) <= 0)
		{
			Packet ping;
			ping[0] = PacketType_Ping;
			Network_Send(&ping);
			ping_time = NETWORK_PINGTIME;
		}
		if (ping_dctime > 0 && (ping_dctime -= timer_dt) <= 0)
		{
			if (is_host)
				Network_KickPeer(); //Disconnect peer
			else
				Network_Disconnect(); //Disconnected from server
		}
	}
}

boolean Network_Inited(void)
{
	#ifdef PSXF_WIN32
		return wsa_init;
	#else
		return true;
	#endif
}

boolean Network_Connected(void)
{
	return sock >= 0;
}

boolean Network_Allowed(void)
{
	return is_allowed;
}

boolean Network_IsHost(void)
{
	return is_host;
}

boolean Network_HasPeer(void)
{
	return sock_peerlen != 0;
}

boolean Network_IsReady(void)
{
	return is_ready;
}

void Network_SetReady(boolean ready)
{
	is_ready = ready;
}
