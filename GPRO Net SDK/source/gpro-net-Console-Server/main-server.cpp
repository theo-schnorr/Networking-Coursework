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

		Author: Theo Schnorrenberg
*/

#include "gpro-net/gpro-net.h"


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"  

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

//Struct to hold the the current users
struct User {
	std::string username;
	RakNet::SystemAddress address;
	bool ready;
};

struct UserHand
{
	std::string hand;
	RakNet::SystemAddress address;
	int userSpot;
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
	int currentPlayer = 0;

	std::vector<UserHand> PlayerHands;
	bool startGame = false;

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

				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_NEW_CHAT_MESSAGE);
				bsOut.Write(rs.C_String());
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);

				break;
			}
			case ID_NEW_USERNAME:
			{
				// Store new username info when person joins
				RakNet::RakString rs;
				RakNet::Time ts;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(ts);
				bsIn.Read(rs);

				User user = {
					rs.C_String(),
					packet->systemAddress,
					false
				};

				UserList.push_back(user);

				std::string join = user.username + " has joined the chat";

				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_NEW_CHAT_MESSAGE);
				bsOut.Write(join.c_str());
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);

				// Ask if player wants to play blackjack
				std::string message = "Start a game of Blackjack?";

				RakNet::BitStream newOut;
				newOut.Write((RakNet::MessageID)ID_START_GAME);
				newOut.Write(message.c_str());
				peer->Send(&newOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);

				break;
			}
			case ID_START_GAME:
			{
				// Get in players ready and once all are ready, begin the game
				bool allIn = true;

				for (int i = 0; i < UserList.size(); i++)
				{
					if (packet->systemAddress == UserList[i].address)
					{
						UserList.at(i).ready = true;
					}
					// If any user is not ready, then not all are in
					if (!UserList.at(i).ready)
					{
						allIn = false;
					}
				}
				
				// If everyone is in, begin the game by shuffling the cards and start dealing 2 cards per
				if (allIn)
				{
					Shuffle();
					currentPlayer = 0;
					PlayerHands.clear();

					for (int i = 0; i < 2; i++)
					{
						for (int j = 0; j < UserList.size(); j++)
						{
							int card = Deal();

							RakNet::BitStream bsOut;
							bsOut.Write((RakNet::MessageID)ID_DEAL_MESSAGE);
							bsOut.Write(card);
							peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UserList.at(i).address, false);
						}

						Hit(Deal());
					}

					for (int i = 0; i < UserList.size(); i++)
					{
						RakNet::BitStream bsOut;
						bsOut.Write((RakNet::MessageID)ID_REQUEST_HAND);
						peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UserList.at(i).address, false);
					}
				}

				break;
			}
			case ID_END_TURN:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);

				for (int i = 0; i < PlayerHands.size(); i++)
				{
					if (packet->systemAddress == PlayerHands[i].address)
					{
						PlayerHands.at(i).hand = UserList.at(PlayerHands.at(i).userSpot).username + "'s Hand: ";
						PlayerHands.at(i).hand += rs.C_String();
					}
				}

				// Send out to players the current state of the game
				string handMes = "";

				for (int i = 0; i < PlayerHands.size(); i++)
				{
					handMes += PlayerHands.at(i).hand + "\n";
				}

				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_SHOW_HANDS);
				bsOut.Write(handMes.c_str());
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);


				// Move onto the next player if there is one
				if (currentPlayer < UserList.size())
				{
					string turnMessage = UserList.at(currentPlayer).username + ", it's your turn. Hit or stand?";

					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_START_TURN);
					bsOut.Write(turnMessage.c_str());
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UserList.at(currentPlayer).address, false);

					currentPlayer++;
				}
				// Otherwise, it's the dealer's turn
				else
				{
					int dealerHand = DealerTurn();

					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_DEALER_HAND);
					bsOut.Write(dealerHand);
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);

					for (int i = 0; i < UserList.size(); i++)
					{
						UserList.at(i).ready = false;
					}
				}

				break;
			}
			case ID_HIT_MESSAGE:
			{
				int card = Deal();

				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_HIT_MESSAGE);
				bsOut.Write(card);
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);

				break;
			}
			case ID_REQUEST_HAND:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				
				string username = "";
				int place = 0;

				for (int i = 0; i < UserList.size(); i++)
				{
					if (packet->systemAddress == UserList[i].address)
					{
						username = UserList.at(i).username + "'s Hand: ";
						place = i;
					}
				}

				username += rs.C_String();

				UserHand mHand = {
					username,
					packet->systemAddress,
					place
				};

				PlayerHands.push_back(mHand);

				if (PlayerHands.size() == UserList.size())
				{
					string deal = "Dealer's Hand: " + ShowHand(true);

					UserHand dealer = {
						deal,
						NULL
					};

					PlayerHands.insert(PlayerHands.begin(), dealer);

					startGame = true;
				}

				break;
			}
			default:
			{
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
			}
		}

		// Once the game has truly begin, send out the hands
		if (startGame)
		{
			startGame = false;

			// Send out to players the current state of the game
			string handMes = "";

			for (int i = 0; i < PlayerHands.size(); i++)
			{
				handMes += PlayerHands.at(i).hand + "\n";
			}

			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_SHOW_HANDS);
			bsOut.Write(handMes.c_str());
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);

			// Start turn w/ player 1
			string turnMessage = UserList.at(currentPlayer).username + ", it's your turn. Hit or stand?";

			RakNet::BitStream newOut;
			newOut.Write((RakNet::MessageID)ID_START_TURN);
			newOut.Write(turnMessage.c_str());
			peer->Send(&newOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UserList.at(currentPlayer).address, false);

			currentPlayer++;
		}
	}


	RakNet::RakPeerInterface::DestroyInstance(peer);

	return 0;
}