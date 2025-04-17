#ifndef CHESS_GAME_H
#define CHESS_GAME_H

#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <cmath>
#include <cctype>
#include <limits>

enum Piece { KING, QUEEN, BISHOP, KNIGHT, ROOK, PAWN, EMPTY };
enum Color { WHITE, BLACK, NONE };

class Square {
    Piece piece;
    Color color;
    int x, y;
    bool hasMoved;

public:
    Square() : piece(EMPTY), color(NONE), x(0), y(0), hasMoved(false) {}

    void setSpace(Square* other) {
        piece = other->piece;
        color = other->color;
        hasMoved = other->hasMoved;
    }

    void setEmpty() {
        piece = EMPTY;
        color = NONE;
        hasMoved = false;
    }

    void setPieceAndColor(Piece p, Color c, bool moved = false) {
        piece = p;
        color = c;
        hasMoved = moved;
    }

    Piece getPiece() const { return piece; }
    Color getColor() const { return color; }
    int getX() const { return x; }
    int getY() const { return y; }
    bool getHasMoved() const { return hasMoved; }

    void setX(int xCoord) { x = xCoord; }
    void setY(int yCoord) { y = yCoord; }
    void setHasMoved(bool moved) { hasMoved = moved; }
};

class Board {
    Square board[8][8];
    Color turn = WHITE;
    std::pair<int, int> enPassantTarget = {-1, -1};

    Color opposite(Color c) const {
        return (c == WHITE) ? BLACK : WHITE;
    }

    bool isPathClear(int fromX, int fromY, int toX, int toY) const {
        int dx = (toX > fromX) ? 1 : (toX < fromX) ? -1 : 0;
        int dy = (toY > fromY) ? 1 : (toY < fromY) ? -1 : 0;

        int x = fromX + dx;
        int y = fromY + dy;

        while (x != toX || y != toY) {
            if (x < 0 || x > 7 || y < 0 || y > 7) return false;
            if (board[x][y].getPiece() != EMPTY) {
                return false;
            }
            x += dx;
            y += dy;
        }
        return true;
    }

    bool isSquareUnderAttack(int x, int y, Color attackerColor) {
        Color targetColor = opposite(attackerColor);
        Color originalTurn = turn;
        turn = attackerColor;

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (board[i][j].getColor() == attackerColor) {
                    if (isValidMoveInternal(i, j, x, y, false)) {
                        turn = originalTurn;
                        return true;
                    }
                }
            }
        }
        turn = originalTurn;
        return false;
    }

    bool isValidMoveInternal(int fromX, int fromY, int toX, int toY, bool checkCheckConstraints) {
        if (fromX < 0 || fromX > 7 || fromY < 0 || fromY > 7 ||
            toX < 0 || toX > 7 || toY < 0 || toY > 7) {
            return false;
        }

        Square* from = &board[fromX][fromY];
        Square* to = &board[toX][toY];

        if (from->getPiece() == EMPTY || (checkCheckConstraints && from->getColor() != turn) ) {
             if (!checkCheckConstraints && from->getPiece() != EMPTY) {
             } else {
                return false;
             }
        }

        if (to->getColor() == from->getColor() && to->getPiece() != EMPTY) {
           return false;
        }

        int dx_abs = abs(toX - fromX);
        int dy_abs = abs(toY - fromY);
        bool basicMoveValid = false;

        switch(from->getPiece()) {
            case PAWN:
                basicMoveValid = isValidPawnMove(fromX, fromY, toX, toY);
                break;
            case KNIGHT:
                basicMoveValid = (dx_abs == 2 && dy_abs == 1) || (dx_abs == 1 && dy_abs == 2);
                break;
            case BISHOP:
                if (dx_abs == dy_abs) {
                    basicMoveValid = isPathClear(fromX, fromY, toX, toY);
                }
                break;
            case ROOK:
                if (dx_abs == 0 || dy_abs == 0) {
                    basicMoveValid = isPathClear(fromX, fromY, toX, toY);
                }
                break;
            case QUEEN:
                if (dx_abs == dy_abs || dx_abs == 0 || dy_abs == 0) {
                    basicMoveValid = isPathClear(fromX, fromY, toX, toY);
                }
                break;
            case KING:
                if (dx_abs <= 1 && dy_abs <= 1) {
                    basicMoveValid = true;
                }
                else if (dx_abs == 0 && dy_abs == 2 && !from->getHasMoved()) {
                    basicMoveValid = isValidCastling(fromX, fromY, toX, toY, checkCheckConstraints);
                }
                break;
            case EMPTY:
            default:
                basicMoveValid = false;
                break;
        }

        if (!basicMoveValid) {
            return false;
        }

        if (checkCheckConstraints) {
            Square tempFrom = *from;
            Square tempTo = *to;
            Piece capturedPiece = to->getPiece();
            Color capturedColor = to->getColor();
            bool capturedHasMoved = to->getHasMoved();
            std::pair<int, int> tempEnPassantTarget = enPassantTarget;
            Square enPassantCapturedSquare;
            bool didEnPassantSim = false;
            int enPassantCaptureSimX = -1, enPassantCaptureSimY = -1;

             if (from->getPiece() == PAWN && to->getPiece() == EMPTY && fromY != toY &&
                enPassantTarget.first == toX && enPassantTarget.second == toY)
             {
                 int direction = (from->getColor() == WHITE) ? -1 : 1;
                 enPassantCaptureSimX = toX - direction;
                 enPassantCaptureSimY = toY;

                 if (enPassantCaptureSimX >= 0 && enPassantCaptureSimX < 8 &&
                     board[enPassantCaptureSimX][enPassantCaptureSimY].getPiece() == PAWN &&
                     board[enPassantCaptureSimX][enPassantCaptureSimY].getColor() == opposite(turn))
                 {
                     enPassantCapturedSquare = board[enPassantCaptureSimX][enPassantCaptureSimY];
                     board[enPassantCaptureSimX][enPassantCaptureSimY].setEmpty();
                     didEnPassantSim = true;
                 } else {
                 }
            }

            to->setSpace(from);
            to->setHasMoved(true);
            from->setEmpty();

            bool didCastleSim = false;
            int castleRookSimFromY = -1, castleRookSimToY = -1;
            Square tempRookFrom, tempRookTo;
            if (tempFrom.getPiece() == KING && dy_abs == 2) {
                castleRookSimFromY = (toY > fromY) ? 7 : 0;
                castleRookSimToY = (toY > fromY) ? 5 : 3;
                if(board[fromX][castleRookSimFromY].getPiece() == ROOK){
                    tempRookFrom = board[fromX][castleRookSimFromY];
                    tempRookTo = board[fromX][castleRookSimToY];
                    board[fromX][castleRookSimToY].setSpace(&board[fromX][castleRookSimFromY]);
                    board[fromX][castleRookSimToY].setHasMoved(true);
                    board[fromX][castleRookSimFromY].setEmpty();
                    didCastleSim = true;
                } else {
                }
            }

            bool leavesKingInCheck = isInCheck(turn);

            from->setPieceAndColor(tempFrom.getPiece(), tempFrom.getColor(), tempFrom.getHasMoved());
            to->setPieceAndColor(capturedPiece, capturedColor, capturedHasMoved);
            enPassantTarget = tempEnPassantTarget;

            if (didEnPassantSim) {
                 board[enPassantCaptureSimX][enPassantCaptureSimY] = enPassantCapturedSquare;
            }
            if (didCastleSim) {
                board[fromX][castleRookSimFromY] = tempRookFrom;
                board[fromX][castleRookSimToY] = tempRookTo;
            }

            if (leavesKingInCheck) {
                return false;
            }
        }

        return true;
    }

    bool isValidPawnMove(int fromX, int fromY, int toX, int toY) {
        Square* from = &board[fromX][fromY];
        Square* to = &board[toX][toY];
        Color pawnColor = from->getColor();
        int direction = (pawnColor == WHITE) ? -1 : 1;

        if (fromY == toY && toX == fromX + direction && to->getPiece() == EMPTY) {
            return true;
        }

        bool startRank = (pawnColor == WHITE && fromX == 6) || (pawnColor == BLACK && fromX == 1);
        if (startRank && fromY == toY && toX == fromX + 2 * direction &&
            to->getPiece() == EMPTY && board[fromX + direction][fromY].getPiece() == EMPTY) {
            return true;
        }

        if (abs(fromY - toY) == 1 && toX == fromX + direction &&
            to->getPiece() != EMPTY && to->getColor() != pawnColor) {
            return true;
        }

        if (abs(fromY - toY) == 1 && toX == fromX + direction &&
            to->getPiece() == EMPTY &&
            enPassantTarget.first == toX && enPassantTarget.second == toY)
        {
            int capturedPawnX = toX - direction;
            int capturedPawnY = toY;
            if(capturedPawnX >= 0 && capturedPawnX < 8 &&
               board[capturedPawnX][capturedPawnY].getPiece() == PAWN &&
               board[capturedPawnX][capturedPawnY].getColor() == opposite(pawnColor))
            {
               return true;
            }
        }

        return false;
    }

    bool isValidCastling(int fromX, int fromY, int toX, int toY, bool checkCheckConstraints) {
        if (abs(fromY - toY) != 2 || fromX != toX) return false;
        if (board[fromX][fromY].getHasMoved()) return false;

        Color kingColor = board[fromX][fromY].getColor();

        if (checkCheckConstraints && isInCheck(kingColor)) {
             return false;
        }

        int rookY = (toY > fromY) ? 7 : 0;
        if (rookY < 0 || rookY > 7) return false;
        Square* rook = &board[fromX][rookY];
        if (rook->getPiece() != ROOK || rook->getColor() != kingColor || rook->getHasMoved()) {
            return false;
        }

        int step = (rookY > fromY) ? 1 : -1;
        for (int y = fromY + step; y != rookY; y += step) {
            if (board[fromX][y].getPiece() != EMPTY) {
                 return false;
            }
        }

        if(checkCheckConstraints) {
            int kingStep = (toY > fromY) ? 1 : -1;
            if (isSquareUnderAttack(fromX, fromY + kingStep, opposite(kingColor))) {
                 return false;
            }
             if (isSquareUnderAttack(toX, toY, opposite(kingColor))) {
                  return false;
             }
        }

        return true;
    }

    void promotePawn(int x, int y) {
        Piece newPiece = QUEEN;
        Color pawnColor = board[x][y].getColor();
        std::cout << "Pawn promotion! Choose piece (Q=Queen[default], R=Rook, B=Bishop, N=Knight): ";
        char choice;

        std::string line;
        if (std::getline(std::cin, line) && !line.empty()) {
             choice = line[0];
        } else {
             choice = 'Q';
             if (std::cin.fail()) {
                  std::cin.clear();
             }
        }

        switch (toupper(choice)) {
            case 'R': newPiece = ROOK; break;
            case 'B': newPiece = BISHOP; break;
            case 'N': newPiece = KNIGHT; break;
            case 'Q':
            default:
                if (toupper(choice) != 'Q') {
                    std::cout << "Invalid choice '" << choice << "', promoting to Queen." << std::endl;
                }
                newPiece = QUEEN;
        }
        board[x][y].setPieceAndColor(newPiece, pawnColor, true);
        char symbol;
        switch (newPiece) {
             case QUEEN: symbol = 'Q'; break;
             case ROOK: symbol = 'R'; break;
             case BISHOP: symbol = 'B'; break;
             case KNIGHT: symbol = 'N'; break;
             default: symbol = '?';
        }
        std::cout << (pawnColor == WHITE ? "White" : "Black") << " promoted pawn to " << symbol << std::endl;
    }

    bool isInCheck(Color kingColor) {
        int kingX = -1, kingY = -1;
        for (int x = 0; x < 8; ++x) {
            for (int y = 0; y < 8; ++y) {
                if (board[x][y].getPiece() == KING && board[x][y].getColor() == kingColor) {
                    kingX = x;
                    kingY = y;
                    break;
                }
            }
            if (kingX != -1) break;
        }

        if (kingX == -1) {
             std::cerr << "Error: King of color " << (kingColor == WHITE ? "WHITE" : "BLACK") << " not found!" << std::endl;
             return false;
        }

        return isSquareUnderAttack(kingX, kingY, opposite(kingColor));
    }

    bool isCheckmate(Color kingColor) {
        if (!isInCheck(kingColor)) {
            return false;
        }

        Color originalTurn = turn;
        turn = kingColor;

        for (int fromX = 0; fromX < 8; ++fromX) {
            for (int fromY = 0; fromY < 8; ++fromY) {
                if (board[fromX][fromY].getColor() == kingColor) {
                    for (int toX = 0; toX < 8; ++toX) {
                        for (int toY = 0; toY < 8; ++toY) {
                            if (isValidMoveInternal(fromX, fromY, toX, toY, true)) {
                                turn = originalTurn;
                                return false;
                            }
                        }
                    }
                }
            }
        }

        turn = originalTurn;
        return true;
    }

    bool isStalemate(Color playerColor) {
        if (isInCheck(playerColor)) {
            return false;
        }

        Color originalTurn = turn;
        turn = playerColor;

        for (int fromX = 0; fromX < 8; ++fromX) {
            for (int fromY = 0; fromY < 8; ++fromY) {
                if (board[fromX][fromY].getColor() == playerColor) {
                    for (int toX = 0; toX < 8; ++toX) {
                        for (int toY = 0; toY < 8; ++toY) {
                            if (isValidMoveInternal(fromX, fromY, toX, toY, true)) {
                                turn = originalTurn;
                                return false;
                            }
                        }
                    }
                }
            }
        }

        turn = originalTurn;
        return true;
    }

public:
    Board() {
        setBoard();
    }

    void setBoard() {
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                board[i][j].setX(i);
                board[i][j].setY(j);
                board[i][j].setEmpty();
            }
        }

        board[7][0].setPieceAndColor(ROOK, WHITE);
        board[7][1].setPieceAndColor(KNIGHT, WHITE);
        board[7][2].setPieceAndColor(BISHOP, WHITE);
        board[7][3].setPieceAndColor(QUEEN, WHITE);
        board[7][4].setPieceAndColor(KING, WHITE);
        board[7][5].setPieceAndColor(BISHOP, WHITE);
        board[7][6].setPieceAndColor(KNIGHT, WHITE);
        board[7][7].setPieceAndColor(ROOK, WHITE);
        for (int i = 0; i < 8; ++i) {
            board[6][i].setPieceAndColor(PAWN, WHITE);
        }

        board[0][0].setPieceAndColor(ROOK, BLACK);
        board[0][1].setPieceAndColor(KNIGHT, BLACK);
        board[0][2].setPieceAndColor(BISHOP, BLACK);
        board[0][3].setPieceAndColor(QUEEN, BLACK);
        board[0][4].setPieceAndColor(KING, BLACK);
        board[0][5].setPieceAndColor(BISHOP, BLACK);
        board[0][6].setPieceAndColor(KNIGHT, BLACK);
        board[0][7].setPieceAndColor(ROOK, BLACK);
        for (int i = 0; i < 8; ++i) {
            board[1][i].setPieceAndColor(PAWN, BLACK);
        }

        turn = WHITE;
        enPassantTarget = {-1, -1};
    }

    void printBoard() const {
        std::cout << "\n    a  b  c  d  e  f  g  h \n";
        std::cout << "  +------------------------+\n";
        for (int i = 0; i < 8; i++) {
            std::cout << 8-i << " |";
            for (int j = 0; j < 8; j++) {
                Piece p = board[i][j].getPiece();
                Color c = board[i][j].getColor();
                char symbol;

                switch (p) {
                    case KING:   symbol = 'K'; break;
                    case QUEEN:  symbol = 'Q'; break;
                    case BISHOP: symbol = 'B'; break;
                    case KNIGHT: symbol = 'N'; break;
                    case ROOK:   symbol = 'R'; break;
                    case PAWN:   symbol = 'P'; break;
                    case EMPTY:  symbol = '.'; break;
                    default:     symbol = '?'; break;
                }

                if (c == WHITE) {
                    symbol = tolower(symbol);
                } else if (c == BLACK) {
                } else {
                }
                std::cout << ' ' << symbol << ' ';
            }
            std::cout << "| " << 8-i << "\n";
        }
        std::cout << "  +------------------------+\n";
        std::cout << "    a  b  c  d  e  f  g  h \n\n";
        std::cout << (turn == WHITE ? "White" : "Black") << " to move." << std::endl;
    }

    bool makeMove(int fromX, int fromY, int toX, int toY) {

        if (!isValidMoveInternal(fromX, fromY, toX, toY, true)) {
            std::cout << "--- ILLEGAL MOVE --- Please try again." << std::endl;
            return false;
        }

        Square* from = &board[fromX][fromY];
        Square* to = &board[toX][toY];
        Piece movingPiece = from->getPiece();
        Color movingColor = from->getColor();

        std::pair<int, int> previousEnPassantTarget = enPassantTarget;
        enPassantTarget = {-1, -1};

        if (movingPiece == PAWN && to->getPiece() == EMPTY && fromY != toY) {
            if (toX == previousEnPassantTarget.first && toY == previousEnPassantTarget.second) {
                int capturedPawnX = fromX;
                int capturedPawnY = toY;
                if (board[capturedPawnX][capturedPawnY].getPiece() == PAWN && board[capturedPawnX][capturedPawnY].getColor() == opposite(movingColor)) {
                    board[capturedPawnX][capturedPawnY].setEmpty();
                } else {
                    std::cerr << "Error: En passant state inconsistency during move execution." << std::endl;
                }
            } else {
                 std::cerr << "Error: Mismatch between pawn move and en passant target during move execution." << std::endl;
            }
        }

        if (movingPiece == KING && abs(fromY - toY) == 2) {
            int rookStartY = (toY > fromY) ? 7 : 0;
            int rookEndY = (toY > fromY) ? 5 : 3;

            Square* rookSquare = &board[fromX][rookStartY];
             if (rookSquare->getPiece() == ROOK && rookSquare->getColor() == movingColor) {
                 board[fromX][rookEndY].setSpace(rookSquare);
                 board[fromX][rookEndY].setHasMoved(true);
                 rookSquare->setEmpty();
             } else {
                 std::cerr << "Error: Castling state inconsistency during move execution." << std::endl;
             }
        }

        to->setSpace(from);
        to->setHasMoved(true);
        from->setEmpty();

        if (movingPiece == PAWN && abs(fromX - toX) == 2) {
            int enPassantRow = (fromX + toX) / 2;
            enPassantTarget = {enPassantRow, toY};
        }

        if (movingPiece == PAWN && (toX == 0 || toX == 7)) {
            promotePawn(toX, toY);
        }

        turn = opposite(turn);

        if (isInCheck(turn)) {
            if (isCheckmate(turn)) {
                printBoard();
                std::cout << "\nCHECKMATE! " << (turn == WHITE ? "Black" : "White") << " wins!" << std::endl;
                return false;
            } else {
                std::cout << "\nCHECK!" << std::endl;
            }
        } else if (isStalemate(turn)) {
            printBoard();
            std::cout << "\nSTALEMATE! The game is a draw." << std::endl;
            return false;
        }

        return true;
    }

    void playGame() {
        setBoard();
        bool gameContinues = true;

        while (gameContinues) {
            printBoard();

            std::string moveInput;
            std::cout << "Enter move (e.g., e2e4, or type 'quit'): ";

             if (!std::getline(std::cin, moveInput)) {
                 if (std::cin.eof()) {
                      std::cout << "\nInput stream ended. Game aborted." << std::endl;
                 } else {
                      std::cout << "\nInput error. Game aborted." << std::endl;
                      std::cin.clear();
                 }
                 break;
             }

            if (moveInput == "quit") {
                 std::cout << "Game aborted by user." << std::endl;
                 break;
            }

             if (moveInput.length() != 4 ||
                 !isalpha(moveInput[0]) || !isdigit(moveInput[1]) ||
                 !isalpha(moveInput[2]) || !isdigit(moveInput[3]))
             {
                 std::cout << "Invalid move format. Use algebraic notation like 'e2e4'." << std::endl;
                 continue;
             }

            int fromY = tolower(moveInput[0]) - 'a';
            int fromX = 8 - (moveInput[1] - '0');
            int toY = tolower(moveInput[2]) - 'a';
            int toX = 8 - (moveInput[3] - '0');

             if (fromX < 0 || fromX > 7 || fromY < 0 || fromY > 7 ||
                 toX < 0 || toX > 7 || toY < 0 || toY > 7)
             {
                 std::cout << "Invalid coordinates. Use ranks 1-8 and files a-h." << std::endl;
                 continue;
             }

            if (!makeMove(fromX, fromY, toX, toY)) {
                Color potentiallyEndedPlayer = turn;
                if (isCheckmate(potentiallyEndedPlayer) || isStalemate(potentiallyEndedPlayer)) {
                     gameContinues = false;
                }
            }

        }

         std::cout << "\nGame Over." << std::endl;
    }

};

#endif
