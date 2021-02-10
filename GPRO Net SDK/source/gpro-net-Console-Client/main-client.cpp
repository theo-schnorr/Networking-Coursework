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

	Authors: Theo Schnorrenberg and Dante Xystus
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

const char USER_BUFFER[] = ": ";

enum GameMessages
{
	ID_NEW_CHAT_MESSAGE = ID_USER_PACKET_ENUM + 1,
	ID_NEW_USERNAME,
	ID_GET_USERS
};

int main(void)
{
	const char SERVER_IP[] = "172.16.2.63";
	const short SERVER_PORT = 7777;

	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;
	RakNet::SocketDescriptor sd;
	RakNet::SystemAddress server;


	peer->Startup(1, &sd, 1);
	peer->SetMaximumIncomingConnections(0);
	peer->Connect(SERVER_IP, SERVER_PORT, 0, 0);

	printf("Starting the client. \n");

	char username[32];
	char message[32];
	char finalMessage[64];

	//Asks the user to enter a username, this will help identify the user when chatting
	printf("Please enter a username: ");
	gets_s(username);
	strcat(username, USER_BUFFER);


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
				//When the client connects to the user it sends the username over to add itself to the user list.
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
				//When the client recieves a chat message it writes it to the screen
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
		strcat(message, "\0"); //Adding a zero-terminate to the string cause it might not have one when it gets here

		if (!strcmp(message, "/users"))
		{
			//If the user types "/users", it sends a packet to ask the user for the user list
			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_GET_USERS);
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, server, false);
		}
		else if (!strcmp(message, "/exit"))
		{
			//If the user types in /exit, it will let the server know it wants to disconnect then close the program
			peer->Shutdown(300);
			break;
		}
		else if (strcmp(message, ""))
		{
			//If the message is not empty, send the chat message to the server
			strcpy(finalMessage, username);
			strcat(finalMessage, message);

			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_NEW_CHAT_MESSAGE);
			bsOut.Write(finalMessage);
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, server, false);
		}
		else
		{
			//If the message is blank, then dont send a message
			//This will allow the client to update the chat without sending a message
		}

	}

	RakNet::RakPeerInterface::DestroyInstance(peer);
	printf("\n\n");
	system("pause");

}
