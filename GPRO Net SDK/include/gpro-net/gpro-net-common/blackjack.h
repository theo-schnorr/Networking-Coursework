/*
	Author: Theo Schnorrenberg

	blackjack.h
	Header for gamestate information and rules in blackjack
*/


#pragma once

#include <vector>
#include <string>

using namespace std;

/* VARIABLES */

// Deck variables
int deck[52] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 11,
				2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 11,
				2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 11,
				2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 11 };
vector<int> Deck;

// Info on hand
vector<int> Hand;
int softTotal = 0;
int hardTotal = 0;
bool bust = false;


/* FUNCTIONS */

// Shuffle the deck (dealer only)
void Shuffle();

// Deal a card (dealer only)
int Deal();

// Get a card (player only)
void Hit(int card);

// Check hand to see if bust
void CheckHand();

// Return whether or not the player has bust
bool GetBust();

// Checks if you won and shows you the result
void CheckWin(int dealerTotal);

// Goes through the logic of the dealer's turn and returns dealer result (dealer only)
int DealerTurn();

// Sends out the player's hand (if dealer, only show first card)
string ShowHand(bool dealer);