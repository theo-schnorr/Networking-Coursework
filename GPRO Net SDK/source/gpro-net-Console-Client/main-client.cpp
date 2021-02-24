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

	Author: Theo Schnorrenberg
*/

#include "gpro-net/gpro-net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/GetTime.h"

#include "gpro-net/gpro-net-common/blackjack.h"

enum GameMessages
{
	ID_NEW_CHAT_MESSAGE = ID_USER_PACKET_ENUM + 1,
	ID_NEW_USERNAME,
	ID_NEW_CHAT_MESSAGE_WITH_TIME,
	ID_DEAL_MESSAGE,
	ID_HIT_MESSAGE,
	ID_START_TURN,
	ID_END_TURN,
	ID_START_GAME,
	ID_DEALER_HAND,
	ID_REQUEST_HAND,
	ID_SHOW_HANDS
};

int main(void)
{
	// Theo, remember to update this with the vdi
	const char SERVER_IP[] = "172.16.2.60";
	const short SERVER_PORT = 7777;

	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;
	RakNet::SocketDescriptor sd;
	RakNet::SystemAddress server;


	peer->Startup(1, &sd, 1);
	peer->SetMaximumIncomingConnections(0);
	peer->Connect(SERVER_IP, SERVER_PORT, 0, 0);

	printf("Starting the client. \n");

	// Accept a message of 128 characters max and username of 32 characters max
	char username[32];
	char message[128];

	//Asks the user to enter a username, this will help identify the user when chatting
	printf("Please enter a username: ");
	gets_s(username);

	// NETWORK LOOP
	while (1)
	{
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			RakNet::MessageID msg = packet->data[0];
			server = packet->systemAddress;

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

				RakNet::Time timestamp = RakNet::GetTime();
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_NEW_USERNAME);
				bsOut.Write((RakNet::Time)timestamp);
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
			case ID_DEAL_MESSAGE:
			{
				// Get dealt card and add it to hand
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);

				int card;
				sscanf(rs.C_String(), "%d", &card);
				Hit(card);

				break;
			}
			case ID_START_TURN:
			{
				// Message sent telling it's your turn and asking to hit or stand
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());

				std::string ans;
				std::cin >> ans;

				// Hit requests a card to be dealt
				if (ans == "hit")
				{
					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_HIT_MESSAGE);
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, server, false);
				}
				// Stand goes to next turn
				else
				{
					string message = ShowHand(false);

					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_END_TURN);
					bsOut.Write(message.c_str());
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, server, false);
				}

				break;
			}
			case ID_HIT_MESSAGE:
			{
				// Get dealt card and add it to hand
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);

				int card;
				sscanf(rs.C_String(), "%d", &card);
				Hit(card);

				// Update hand and show if bust
				cout << ShowHand(false) << endl;
				cout << "Soft total: " << softTotal << endl;
				cout << "Hard total: " << hardTotal << endl;

				// If not bust, ask if go again
				if (!GetBust)
				{
					cout << "Hit or stand?" << endl;
					std::string ans;
					std::cin >> ans;

					// Hit requests a card to be dealt
					if (ans == "hit")
					{
						RakNet::BitStream bsOut;
						bsOut.Write((RakNet::MessageID)ID_HIT_MESSAGE);
						peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, server, false);
					}
					// Stand goes to next turn
					else
					{
						string message = ShowHand(false);

						RakNet::BitStream bsOut;
						bsOut.Write((RakNet::MessageID)ID_END_TURN);
						bsOut.Write(message.c_str());
						peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, server, false);
					}
				}
				// Else, continue to next turn
				else
				{
					cout << "Bust!" << endl;

					string message = ShowHand(false);

					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_END_TURN);
					bsOut.Write(message.c_str());
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, server, false);
				}

				break;
			}
			case ID_START_GAME:
			{
				// Get request to start game of blackjack
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());

				string ans;
				cin >> ans;

				system("cls");

				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_START_GAME);
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, server, false);

				break;
			}
			case ID_DEALER_HAND:
			{
				// Get dealer total and compare
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);

				int card;
				sscanf(rs.C_String(), "%d", &card);
				CheckWin(card);

				break;
			}
			case ID_REQUEST_HAND:
			{
				string message = ShowHand(false);

				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_REQUEST_HAND);
				bsOut.Write(message.c_str());
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, server, false);
			}
			case ID_SHOW_HANDS:
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
	}

	RakNet::RakPeerInterface::DestroyInstance(peer);
	printf("\n\n");
	system("pause");

}
