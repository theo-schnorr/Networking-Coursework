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
	main-server.c/.cpp
	Main source for console server application.

		Authors: Theo Schnorrenberg and Dante Xystus
*/

#include "gpro-net/gpro-net.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"  

#include "string"
#include "vector"
#include "iostream"

enum GameMessages
{
	ID_NEW_CHAT_MESSAGE = ID_USER_PACKET_ENUM + 1,
	ID_NEW_USERNAME,
	ID_GET_USERS,
	ID_NEW_CHAT_MESSAGE_WITH_TIME

};

//Struct to hold the the current users
struct User {
	std::string username;
	RakNet::SystemAddress address;
};

struct Message {
	RakNet::Time timestamp;
	std::string message;
};

int main(void)
{

	const unsigned short SERVER_PORT = 7777;
	const unsigned short MAX_CLIENTS = 10;

	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;
	RakNet::SocketDescriptor sd(SERVER_PORT, 0);
	peer->Startup(MAX_CLIENTS, &sd, 1);

	printf("Starting the server.\n");
	peer->SetMaximumIncomingConnections(MAX_CLIENTS);

	std::vector<User> UserList;

	std::vector<Message> Messages;
	std::ofstream file;

	while (1)
	{
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			RakNet::MessageID msg = packet->data[0];

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
				printf("Our connection request has been accepted.\n");
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
				//If a client disconnects we should remove it from the userlist
				printf("A client has disconnected.\n");
				for (int i = 0; i < UserList.size(); i++)
				{
					if (packet->systemAddress == UserList[i].address)
					{
						UserList.erase(UserList.begin() + i);
					}
				}
				break;
			}
			case ID_CONNECTION_LOST:
			{
				//If a client disconnects we should remove it from the userlist
				printf("A client lost the connection.\n");

				for (int i = 0; i < UserList.size(); i++)
				{
					if (packet->systemAddress == UserList[i].address)
					{
						UserList.erase(UserList.begin() + i);
					}
				}
				break;
			}
			case ID_NEW_CHAT_MESSAGE:
			{
				//Reads the incomming chat message and broadcasts it to every other client
				RakNet::RakString rs;
				RakNet::Time ts;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(ts);
				bsIn.Read(rs);
				std::cout << ts << " ";
				printf("%s\n", rs.C_String());

				Message message = {
					ts,
					rs.C_String()
				};
				Messages.push_back(message);

				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_NEW_CHAT_MESSAGE);
				bsOut.Write(rs.C_String());
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);

				break;
			}
			case ID_NEW_USERNAME:
			{
				//When a new user connects to the server
				//Store the user and the address

				RakNet::RakString rs;
				RakNet::Time ts;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(ts);
				bsIn.Read(rs);
				User user = {
					rs.C_String(),
					packet->systemAddress
				};

				UserList.push_back(user);

				std::string join = user.username + "has joined the chat";

				Message message = {
					ts,
					join
				};
				Messages.push_back(message);

				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_NEW_CHAT_MESSAGE);
				bsOut.Write(join.c_str());
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);
				break;
			}
			case ID_GET_USERS:
			{
				//Gets all the active users and sends the list as a chat message to the client that requested it
				std::string msg = "";
				for (int i = 0; i < UserList.size(); i++)
				{
					msg += UserList[i].username + ",";
				}

				char userlist[] = "UserList: ";
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_NEW_CHAT_MESSAGE);
				bsOut.Write(strcat(userlist, msg.c_str()));
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
				break;
			}
			default:
			{
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
			}
		}


		// Write messages to log file
		file.open("serverlog.txt");

		for (int i = 0; i < Messages.size(); i++)
		{
			file << Messages[i].timestamp << ' ';
			file << Messages[i].message << '\n';
		}

		file.close();
	}


	RakNet::RakPeerInterface::DestroyInstance(peer);

	return 0;
}