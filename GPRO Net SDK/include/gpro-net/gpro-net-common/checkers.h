/*
	Author: Theo Schnorrenberg

	checkers.h
	Header for gamestate information and rules in checkers
*/

#pragma once
#include <vector>

/*
	Checkers board:
		[8]
			 _______________________________________
		 0	| 0  |    | 1  |    | 2  |    | 3  |    |	[4]
			|____|____|____|____|____|____|____|____|
		 1	|    | 0  |    | 1  |    | 2  |    | 3  |
			|____|____|____|____|____|____|____|____|
		 2	| 0  |    | 1  |    | 2  |    | 3  |    |
			|____|____|____|____|____|____|____|____|
		 3	|    | 0  |    | 1  |    | 2  |    | 3  |
			|____|____|____|____|____|____|____|____|
		 4	| 0  |    | 1  |    | 2  |    | 3  |    |
			|____|____|____|____|____|____|____|____|
		 5	|    | 0  |    | 1  |    | 2  |    | 3  |
			|____|____|____|____|____|____|____|____|
		 6	| 0  |    | 1  |    | 2  |    | 3  |    |
			|____|____|____|____|____|____|____|____|
		 7	|    | 0  |    | 1  |    | 2  |    | 3  |
			|____|____|____|____|____|____|____|____|


	Piece setup:
		
			 _______________________________________
		 	| 0  |    | 1  |    | 2  |    | 3  |    |	P1 pieces
			|____|____|____|____|____|____|____|____|
		 	|    | 4  |    | 5  |    | 6  |    | 7  |
			|____|____|____|____|____|____|____|____|
		 	|    |    |    |    |    |    |    |    |
			|____|____|____|____|____|____|____|____|
		 	|    |    |    |    |    |    |    |    |
			|____|____|____|____|____|____|____|____|
		 	|    |    |    |    |    |    |    |    |
			|____|____|____|____|____|____|____|____|
		 	|    |    |    |    |    |    |    |    |
			|____|____|____|____|____|____|____|____|
		 	| 0  |    | 1  |    | 2  |    | 3  |    |  P2 Pieces
			|____|____|____|____|____|____|____|____|
		 	|    | 4  |    | 5  |    | 6  |    | 7  |
			|____|____|____|____|____|____|____|____|

*/

// Keep track of piece info (pos and rank)
struct Piece
{
	int row;
	int col;
	bool king;
};

enum Direction
{
	Up_Left,
	Up_Right,
	Down_Left,
	Down_Right
};

// Keeps track of spaces for drawing purposes: 0 = empty, 1 = player 1, 2 = player 2, 3 = p1 king, 4 = p2 king
int spaces[8][4];

// true = p1 turn, false = p2 turn
bool player1Turn = true;
int winner = 0;

// Vectors keeping track of state of pieces
std::vector<Piece> Player1Pieces;
std::vector<Piece> Player2Pieces;

// Direction for set capture
Direction capDir = Up_Left;

// Update the board with any changes
bool UpdateBoard();

// Draw the current version of the board
void DrawBoard();

// Reset the board to starting positions
void ResetBoard();

// Check if a player has won the game
bool CheckWin();

// State in which a player wins the game
void PlayerWin(int player);

// Check whether or not there is a capture the player MUST make
int CheckForCaptures();

// Check whether or not there is a capture after an intial capture
bool CheckForExtraCapture(Piece mPiece, int player);

// Go through the movement to do a capture
bool Capture(int piece, int player);

// Make chosen piece move
bool Jump(int piece, int player, bool right, bool forward = true);

// Set which player's turn it is
void SetTurn();