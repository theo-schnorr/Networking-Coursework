/*
   Copyright 2021 Daniel S. Buckstein

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/*
	GPRO Net SDK: Networking framework.
	By Daniel S. Buckstein

	main-client.c/.cpp
	Main source for console client application.
*/

#include "gpro-net/gpro-net.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "RakNet/RakPeerInterface.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/GetTime.h"

#define MAX_CLIENTS 10;

const char USER_BUFFER[] = ": ";
enum GameMessages
{
	ID_NEW_CHAT_MESSAGE = ID_USER_PACKET_ENUM + 1,
	ID_NEW_USERNAME,
	ID_GET_USERS
};

//Packs the data to take up less space
#pragma pack (push)
struct GameMessage
{
	char msgID; //The ID

	char msg[512]; //The Message

	//Timestamp will go here
};
#pragma pack (pop)

int main(void)
{
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;
	RakNet::SocketDescriptor sd;
	const char SERVER_IP[] = "172.16.2.186";
	const short SERVER_PORT = 7777;
	RakNet::SystemAddress server;

	peer->Startup(1, &sd, 1);
	peer->SetMaximumIncomingConnections(0);
	peer->Connect(SERVER_IP, SERVER_PORT, 0, 0);

	printf("Starting the client. \n");

	char username[32];
	char message[32];
	char finalMessage[64];

	printf("Please enter a username: ");
	gets_s(username);
	strcat(username, USER_BUFFER);

	printf("Please enter a chat message: ");
	gets_s(message);

	strcpy(finalMessage, username);
	strcat(finalMessage, message);
	

	//Network Loop
	while (1)
	{
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			RakNet::MessageID msg = packet->data[0];
			server = packet->systemAddress;

			if (msg == ID_TIMESTAMP)
			{
				//Handle the timestamp
			}

			switch (msg)
			{
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			{
				printf("Another client has disconnected.\n");
				break;
			}
			case ID_REMOTE_CONNECTION_LOST:
			{
				printf("Another client has lost the connection.\n");
				break;
			}
			case ID_REMOTE_NEW_INCOMING_CONNECTION:
			{
				printf("Another client has connected.\n");
				break;
			}
			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				printf("Our connection request has been accepted \n");
			
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_NEW_USERNAME);
				bsOut.Write(username);
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);

				break;
			}
			case ID_NEW_INCOMING_CONNECTION:
			{
				printf("A connection is incoming.\n");
				break;
			}
			case ID_NO_FREE_INCOMING_CONNECTIONS:
			{
				printf("The server is full.\n");
				break;
			}
			case ID_DISCONNECTION_NOTIFICATION:
			{
				printf("We have been disconnected.\n");
				break;
			}
			case ID_CONNECTION_LOST:
			{
				printf("Connection Lost");
				break;
			}
			case ID_NEW_CHAT_MESSAGE:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());


				break;
			}
			default:
			{
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
			}
		}

		//Typing Loop
		printf("Please enter a chat message: ");
		gets_s(message);

		if (!strcmp(message, "/users"))
		{
			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_GET_USERS);
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, server, false);
		}

		if (strcmp(message, ""))
		{
			strcpy(finalMessage, username);
			strcat(finalMessage, message);

			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_NEW_CHAT_MESSAGE);
			bsOut.Write(finalMessage);
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, server, false);
		}
		
	}

	RakNet::RakPeerInterface::DestroyInstance(peer);
	printf("\n\n");
	system("pause");	

}
