'''
Filename: Board.py
Description: Methods that pertain to board functions.
It's better to have these as static methods so they can be used
without instantiating an unnecessary object.

@author: et
'''

import re
import Piece



##
# Returns whether this piece is frozen.
# @param board - the current board state
# @param piece - the piece
# @param row - this pieces' row
# @param col - this piece's column
# @return True if the piece is frozen, False otherwise
def isFrozen(board, piece, row, col):
    occ_adj_pos = getAdjacentPositions(board, row, col, True)
    
    for pos in occ_adj_pos:
        adj_row = pos[0]
        adj_col = pos[1]
        adj_piece = board[adj_row][adj_col]
        
        if adj_piece == " ":
            continue
        elif adj_piece.isupper() and piece.isupper():
            continue
        elif adj_piece.islower() and piece.islower():
            continue
        elif Piece.isStronger(adj_piece, piece):
            return True
    
    return False


##
# Returns all the adjacent positions (north, south, east, west),
# that are on the board, to this piece.
# @param board - the current board state.
# @param row - the piece's row
# @param col - the pieces's column
# @param occupied - True to return only occupied space, False to return empty spaces.
# @return pieces - the positions adjacent to this piece
def getAdjacentPositions(board, row, col, occupied):
    
    positions = []
    
    if occupied:
        expr = "[^ xX]"
    else:
        expr = "( |x|X)"
        
    
    # North
    if row - 1 >= 0 and re.match(expr, board[row-1][col], re.IGNORECASE):
        positions.append([row - 1, col])
    
    # South
    if row + 1 <= 7 and re.match(expr, board[row+1][col], re.IGNORECASE):
        positions.append([row + 1, col])
    
    # West
    if col - 1 >= 0 and re.match(expr, board[row][col-1], re.IGNORECASE):
        positions.append([row, col - 1])
    
    # East
    if col + 1 <= 7 and re.match(expr, board[row][col+1], re.IGNORECASE):
        positions.append([row, col + 1])
    
    return positions



##
# Simply returns whether or not a piece has a friendly
# piece next to it in an adjacent square. Used to check
# trap squares.
# @param row - the row
# @param col - the column
# @return True if it's safe, False otherwise.
def isSafe(board, row, col):

    piece = board[row][col]
    if piece == "x" or piece == "X":
        return True


    adj_occ_pos = getAdjacentPositions(board, row, col, True)
    for pos in adj_occ_pos:
        adj_row = pos[0]
        adj_col = pos[1]
        adj_piece = board[adj_row][adj_col]
        if Piece.isFriends(adj_piece, piece):
            return True
   
    return False
    
