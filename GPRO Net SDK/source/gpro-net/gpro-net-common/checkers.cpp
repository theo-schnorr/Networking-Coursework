/*
	Author: Theo Schnorrenberg

	checkers.cpp
	Gamestate information and rules in checkers
*/

#include "gpro-net/gpro-net-common/checkers.h"

// Update the board with any changes
bool UpdateBoard()
{
	// Clear vectors to have current board info
	Player1Pieces.clear();
	Player2Pieces.clear();

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (spaces[i][j] == 1)
			{
				Piece piece = {
					i,
					j,
					false
				};

				Player1Pieces.push_back(piece);
			}
			else if (spaces[i][j] == 3)
			{
				Piece piece = {
					i,
					j,
					true
				};

				Player1Pieces.push_back(piece);
			}
			else if (spaces[i][j] == 2)
			{
				Piece piece = {
					i,
					j,
					false
				};

				Player2Pieces.push_back(piece);
			}
			else if (spaces[i][j] == 4)
			{
				Piece piece = {
					i,
					j,
					true
				};

				Player2Pieces.push_back(piece);
			}
		}
	}
}

// Draw the current version of the board
void DrawBoard()
{

}

// Reset the board to starting positions
void ResetBoard()
{
	// Clear out the vectors and set to player 1 turn
	Player1Pieces.clear();
	Player2Pieces.clear();

	player1Turn = true;

	// Reset the board
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (i < 2)
			{
				// Set player one pieces at top as normal pieces
				spaces[i][j] = 1;

				Piece piece = {
					i,
					j,
					false
				};

				Player1Pieces.push_back(piece);
			}
			else if (i > 5)
			{
				// Set player two pieces at bottom as normal pieces
				spaces[i][j] = 2;

				Piece piece = {
					i,
					j,
					false
				};

				Player2Pieces.push_back(piece);
			}
			else
			{
				spaces[i][j] = 0;
			}
		}
	}
}

// Check if a player has won the game
bool CheckWin()
{
	if (Player1Pieces.size() == 0)
	{
		winner = 2;
		return true;
	}
	else if (Player2Pieces.size() == 0)
	{
		winner = 1;
		return true;
	}

	return false;
}

// State in which a player wins the game
void PlayerWin(int player)
{

}

// Check whether or not there is a capture the player MUST make
int CheckForCaptures()
{
	if (player1Turn)
	{
		for (int i = 0; i < Player1Pieces.size(); i++)
		{
			// Can only capture if enough space forward or king (who can move any direction)
			if (Player1Pieces.at(i).row < 6 || Player1Pieces.at(i).king)
			{
				// 0 = even row
				int rowEven = Player1Pieces.at(i).row % 2;
				int left = 0;
				int right = 0;
				int row = Player1Pieces.at(i).row;
				int col = Player1Pieces.at(i).col;

				// In even rows, diagonal left is -1 column
				if (rowEven == 0)
				{
					left = 1;
				}
				// In odd rows, diagonal right is +1 column
				else
				{
					right = 1;
				}

				// Check for left movement if not in leftmost 2 columns (need a space behind enemy to capture)
				if (col > 0)
				{
					// Check row forward
					if (row < 6)
					{
						// If there's someone to capture with space, then return that this unit can capture
						if (((spaces[row+1][col-left] == 2) || (spaces[row + 1][col - left] == 4)) && (spaces[row+2][col-1] == 0))
						{
							capDir = Down_Left;
							return i;
						}
					}
					// King can check behind
					if (Player1Pieces.at(i).king && Player1Pieces.at(i).row > 1)
					{
						// If there's someone to capture with space, then return that this unit can capture
						if (((spaces[row-1][col-left] == 2) || (spaces[row - 1][col - left] == 4)) && (spaces[row-2][col-1] == 0))
						{
							capDir = Up_Left;
							return i;
						}
					}
				}

				// Check for right movement if not in rightmost 2 columns (need a space behind enemy to capture)
				if (col < 3)
				{
					// Check row forward
					if (row < 6)
					{
						// If there's someone to capture with space, then return that this unit can capture
						if (((spaces[row+1][col+right] == 2) || (spaces[row + 1][col + right] == 4)) && (spaces[row+2][col+1] == 0))
						{
							capDir = Down_Right;
							return i;
						}
					}
					// King can check behind
					if (Player1Pieces.at(i).king && Player1Pieces.at(i).row > 1)
					{
						// If there's someone to capture with space, then return that this unit can capture
						if (((spaces[row-1][col+right] == 2) || (spaces[row - 1][col + right] == 4)) && (spaces[row-2][col+1] == 0))
						{
							capDir = Up_Right;
							return i;
						}
					}
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < Player2Pieces.size(); i++)
		{
			// Can only capture if enough space forward or king (who can move any direction)
			if (Player2Pieces.at(i).row > 1 || Player2Pieces.at(i).king)
			{
				// 0 = even row
				int rowEven = Player2Pieces.at(i).row % 2;
				int left = 0;
				int right = 0;
				int row = Player2Pieces.at(i).row;
				int col = Player2Pieces.at(i).col;

				// In even rows, diagonal left is -1 column
				if (rowEven == 0)
				{
					left = 1;
				}
				// In odd rows, diagonal right is +1 column
				else
				{
					right = 1;
				}

				// Check for left movement if not in leftmost 2 columns (need a space behind enemy to capture)
				if (col > 0)
				{
					// Check row forward
					if (row > 1)
					{
						// If there's someone to capture with space, then return that this unit can capture
						if (((spaces[row - 1][col - left] == 1) || (spaces[row - 1][col - left] == 3)) && (spaces[row - 2][col - 1] == 0))
						{
							capDir = Up_Left;
							return i;
						}
					}
					// King can check behind
					if (Player2Pieces.at(i).king && Player2Pieces.at(i).row < 6)
					{
						// If there's someone to capture with space, then return that this unit can capture
						if (((spaces[row + 1][col - left] == 1) || (spaces[row + 1][col - left] == 3)) && (spaces[row + 2][col - 1] == 0))
						{
							capDir = Down_Left;
							return i;
						}
					}
				}

				// Check for right movement if not in rightmost 2 columns (need a space behind enemy to capture)
				if (col < 3)
				{
					// Check row forward
					if (row > 1)
					{
						// If there's someone to capture with space, then return that this unit can capture
						if (((spaces[row - 1][col + right] == 1) || (spaces[row - 1][col + right] == 3)) && (spaces[row - 2][col + 1] == 0))
						{
							capDir = Up_Right;
							return i;
						}
					}
					// King can check behind
					if (Player2Pieces.at(i).king && Player2Pieces.at(i).row < 6)
					{
						// If there's someone to capture with space, then return that this unit can capture
						if (((spaces[row + 1][col + right] == 1) || (spaces[row + 1][col + right] == 3)) && (spaces[row + 2][col + 1] == 0))
						{
							capDir = Down_Right;
							return i;
						}
					}
				}
			}
		}
	}

	return -1;
}

// Check whether or not there is a capture after an intial capture
bool CheckForExtraCapture(Piece mPiece, int player)
{
	bool found = false;
	bool goForward = false;
	bool goRight = false;

	int otherPlayer = 2;
	int forward = 1;

	if (player == 2)
	{
		otherPlayer = 1;
		forward = -1;
	}

	int forwardMove = mPiece.row + (forward * 2);
	int backMove = mPiece.row - (forward * 2);

	if (mPiece.king || (forwardMove >= 0 && forwardMove <= 7))
	{
		// 0 = even row
		int rowEven = mPiece.row % 2;
		int left = 0;
		int right = 0;

		// In even rows, diagonal left is -1 column
		if (rowEven == 0)
		{
			left = 1;
		}
		// In odd rows, diagonal right is +1 column
		else
		{
			right = 1;
		}

		// Check for left movement if not in leftmost 2 columns (need a space behind enemy to capture)
		if (mPiece.col > 0)
		{
			// Check row forward
			if (forwardMove >= 0 && forwardMove <= 7)
			{
				// If there's someone to capture with space, then return that this unit can capture
				if (((spaces[mPiece.row + forward][mPiece.col - left] == otherPlayer) || (spaces[mPiece.row + forward][mPiece.col - left] == (otherPlayer+2))) && (spaces[forwardMove][mPiece.col - 1] == 0))
				{
					goForward = true;
					goRight = false;
					found = true;
				}
			}
			// King can check behind
			if (mPiece.king && backMove >= 0 && backMove <= 7)
			{
				// If there's someone to capture with space, then return that this unit can capture
				if (((spaces[mPiece.row - forward][mPiece.col - left] == otherPlayer) || (spaces[mPiece.row - forward][mPiece.col - left] == (otherPlayer+2))) && (spaces[backMove][mPiece.col - 1] == 0))
				{
					goForward = false;
					goRight = false;
					found = true;
				}
			}
		}

		// Check for right movement if not in rightmost 2 columns (need a space behind enemy to capture)
		if (mPiece.col < 3)
		{
			// Check row forward
			if (forwardMove >= 0 && forwardMove <= 7)
			{
				// If there's someone to capture with space, then return that this unit can capture
				if (((spaces[mPiece.row + forward][mPiece.col + right] == otherPlayer) || (spaces[mPiece.row + forward][mPiece.col + right] == (otherPlayer+2))) && (spaces[forwardMove][mPiece.col + 1] == 0))
				{
					goForward = true;
					goRight = true;
					found = true;
				}
			}
			// King can check behind
			if (mPiece.king && backMove >= 0 && backMove <= 7)
			{
				// If there's someone to capture with space, then return that this unit can capture
				if (((spaces[mPiece.row - forward][mPiece.col + right] == otherPlayer) || (spaces[mPiece.row - forward][mPiece.col + right] == (otherPlayer+2))) && (spaces[backMove][mPiece.col + 1] == 0))
				{
					goForward = false;
					goRight = true;
					found = true;
				}
			}
		}
	}

	if (found)
	{
		if ((goForward && forward == -1) || (!goForward && forward == 1))
		{
			if (goRight)
			{
				capDir = Up_Right;
			}
			else
			{
				capDir = Up_Left;
			}
		}
		else
		{
			if (goRight)
			{
				capDir = Down_Right;
			}
			else
			{
				capDir = Down_Left;
			}
		}
	}

	return found;
}

// Go through the movement to do a capture
bool Capture(int piece, int player)
{
	Piece mPiece;

	if (player == 1)
	{
		mPiece = Player1Pieces.at(piece);
	}
	else
	{
		mPiece = Player2Pieces.at(piece);
	}

	// Row info setup to delete piece jumping over
	int rowEven = mPiece.row % 2;
	int left = 0;
	int right = 0;

	// In even rows, diagonal left is -1 column
	if (rowEven == 0)
	{
		left = 1;
	}
	// In odd rows, diagonal right is +1 column
	else
	{
		right = 1;
	}

	// Update spaces on the board
	switch (capDir)
	{
	case Up_Left:
		spaces[mPiece.row][mPiece.col] = 0;
		spaces[mPiece.row - 1][mPiece.col - left] = 0;
		mPiece.row -= 2;
		mPiece.col -= 1;

		break;

	case Up_Right:
		spaces[mPiece.row][mPiece.col] = 0;
		spaces[mPiece.row - 1][mPiece.col + right] = 0;
		mPiece.row -= 2;
		mPiece.col += 1;

		break;

	case Down_Left:
		spaces[mPiece.row][mPiece.col] = 0;
		spaces[mPiece.row + 1][mPiece.col - left] = 0;
		mPiece.row += 2;
		mPiece.col -= 1;

		break;

	case Down_Right:
		spaces[mPiece.row][mPiece.col] = 0;
		spaces[mPiece.row + 1][mPiece.col + right] = 0;
		mPiece.row += 2;
		mPiece.col += 1;

		break;
	}

	if (mPiece.king)
	{
		spaces[mPiece.row][mPiece.col] = player + 2;
	}
	else
	{
		spaces[mPiece.row][mPiece.col] = player;
	}

	return CheckForExtraCapture(mPiece, player);
}

// Make chosen piece move
bool Jump(int piece, int player, bool right, bool forward = true)
{
	Piece mPiece;
	int forward = 1;

	if (player == 1)
	{
		mPiece = Player1Pieces.at(piece);
	}
	else
	{
		mPiece = Player2Pieces.at(piece);
		forward = -1;
	}

	// Row info setup to delete piece jumping over
	int rowEven = mPiece.row % 2;
	int left = 0;
	int right = 0;

	// In even rows, diagonal left is -1 column
	if (rowEven == 0)
	{
		left = 1;
	}
	// In odd rows, diagonal right is +1 column
	else
	{
		right = 1;
	}

	// Set previous space to false
	spaces[mPiece.row][mPiece.col] = 0;

	// Set column of new space
	if (right)
	{
		mPiece.col += right;
	}
	else
	{
		mPiece.col -= left;
	}

	// Set row of new space
	if (forward)
	{
		mPiece.row += forward;
	}
	else
	{
		mPiece.row -= forward;
	}

	// Set new placement
	if (mPiece.king)
	{
		spaces[mPiece.row][mPiece.col] = player + 2;
	}
	else
	{
		spaces[mPiece.row][mPiece.col] = player;
	}
}

// Set turn to other player
void SetTurn()
{
	player1Turn = !player1Turn;
}