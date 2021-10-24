/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_NETWORK_H
#define PSXF_GUARD_NETWORK_H
#ifdef PSXF_NETWORK

#include "psx.h"
#include "timer.h"

//Packet enums
typedef enum
{
	PacketType_Ping,       //Client <-> Server
	PacketType_Access,     //Client -> Server -> Client (response)
	PacketType_Disconnect, //Client <-> Server
	PacketType_Ready,      //Server -> Client -> Server (response)
	PacketType_NoteHit,    //Client <-> Server
	PacketType_NoteMiss,   //Client <-> Server
} PacketType;

#define PACKET_SIZE 0x40
typedef u8 Packet[PACKET_SIZE];
/*
PACKET STRUCTURE
0x40 BYTES
off  - size - desc
	0x00 - 0x01 - packet type
//0x00 - PacketType_Ping (Client <-> Server)
	Nothing
//0x01 - PacketType_Access (Client -> Server)
	0x01 - 0x20 - password
//0x01 - PacketType_Access (Server -> Client)
	0x01 - 0x01 - allowed
//0x02 - PacketType_Disconnect (Client <-> Server)
	Nothing
//0x03 - PacketType_Ready (Server -> Client)
	0x01 - 0x01 - stage id
	0x02 - 0x01 - stage difficulty
	0x03 - 0x01 - is player 2
//0x03 - PacketType_Ready (Client -> Server)
	Nothing
//0x04 - PacketType_NoteHit (Client <-> Server)
	0x01 - 0x02 - note id
	0x03 - 0x04 - score
	0x07 - 0x01 - judgement
	0x08 - 0x02 - combo
//0x05 - PacketType_NoteMiss (Client <-> Server)
	0x01 - 0x01 - type
	0x02 - 0x04 - score
*/

//Network interface
void Network_Init(void);
void Network_Quit(void);
boolean Network_HostPort(const char *port, const char *pass);
boolean Network_Join(const char *ip, const char *port, const char *pass);
void Network_Disconnect(void);
void Network_Send(Packet *packet);
void Network_Process(void);
boolean Network_Inited(void);
boolean Network_Connected(void);
boolean Network_Allowed(void);
boolean Network_IsHost(void);
boolean Network_HasPeer(void);
boolean Network_IsReady(void);
void Network_SetReady(boolean ready);

#else
	#define Network_Init()
	#define Network_Quit()
	#define Network_Process()
	#define Network_Inited() false
	#define Network_Connected() false
	#define Network_Allowed() false
	#define Network_IsHost() false
	#define Network_HasPeer() false
	#define Network_IsReady() true
#endif

#endif
