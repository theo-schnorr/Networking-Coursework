/*
	Author: Theo Schnorrenberg

	blackjack.cpp
	Functions for the blackjack game
*/

#include <iostream>
#include <algorithm>

#include "gpro-net/gpro-net-common/blackjack.h"

// Shuffle the deck (dealer only)
void Shuffle()
{
	// Need to clear current shuffled deck to reshuffle
	Deck.clear();
	Hand.clear();

	for (int i = 0; i < 52; i++)
	{
		Deck.push_back(deck[i]);
	}

	random_shuffle(Deck.begin(), Deck.end());
}

// Deal a card (dealer only)
int Deal()
{
	// This will deal and remove of the first card
	int deal = 0;

	deal = Deck.front();
	Deck.erase(Deck.begin());

	return deal;
}

// Get a card (player only)
void Hit(int card)
{
	Hand.push_back(card);
	
	// Add 1 and 11 to proper totals if ace
	if (card == 11)
	{
		if ((softTotal + 11) > 21)
		{
			softTotal += 1;
		}
		else
		{
			softTotal += 11;
		}

		hardTotal += 1;
	}
	// Otherwise, add card num to both totals
	else
	{
		softTotal += card;
		hardTotal += card;
	}

	CheckHand();
}

// Check hand to see if bust
void CheckHand()
{
	if (hardTotal > 21)
	{
		bust = true;
	}
}

// Return whether or not the player has bust
bool GetBust()
{
	return bust;
}

// Checks if you won and shows you the result
void CheckWin(int dealerTotal)
{
	if (!bust)
	{
		if (hardTotal > dealerTotal || (softTotal > dealerTotal && softTotal <= 21))
		{
			cout << "You beat the dealer!" << endl;
		}
		else
		{
			cout << "You lost to the dealer." << endl;
		}
	}
	else
	{
		cout << "You busted out." << endl;
	}

	Hand.clear();
}

// Goes through the logic of the dealer's turn and returns dealer result (dealer only)
int DealerTurn()
{
	// Final total for dealer to send
	int total = softTotal;
	
	while (total <= 16)
	{
		Hand.push_back(Deal());

		softTotal += Hand.back();
		hardTotal += Hand.back();

		if (softTotal > 21)
		{
			total = hardTotal;
		}
		else
		{
			total = softTotal;
		}
	}

	return total;
}

// Sends out the player's hand (if dealer, only show first card)
string ShowHand(bool dealer)
{
	string myHand = "";

	myHand += Hand.front();

	if (!dealer)
	{
		for (int i = 1; i < Hand.size(); i++)
		{
			myHand += " " + Hand.at(i);
		}
	}

	return myHand;
}