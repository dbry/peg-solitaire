////////////////////////////////////////////////////////////////////////////
//                             *** PEG ***                                //
//                  Peg-Solitaire Random-Moves Solver                     //
//                   Copyright (c) 2020 David Bryant                      //
//                         All Rights Reserved                            //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

// peg.c

// This program attempts to solve the English Peg Solitaire game by simply
// playing out complete games, choosing randomly from the legal moves, until
// the puzzle is solved. This is a little closer to how a person might
// actually approach this.

//  English Peg-Solitaire board, notation, and initial position of pegs:
// 
//    1 2 3 4 5 6 7
//  1     ● ● ●
//  2     ● ● ●
//  3 ● ● ● ● ● ● ●
//  4 ● ● ● ○ ● ● ●
//  5 ● ● ● ● ● ● ●
//  6     ● ● ●
//  7     ● ● ●
// 

// The program's default behavior is to run until it finds a solution, at
// which time it will display the solution and also provide histogram results
// on all the games run. It is possible to specify a game count as an argument
// to the program and it will terminate after that many games whether or not
// solutions were found (although each solution found will be displayed). If
// only a single game is requested then the final board position and the game
// sequence will be displayed for that one game.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

typedef char board_t [9] [9];

typedef struct {
    int x, y, dx, dy;
} move_t;

#define HOLE        0x1
#define PEG         0x2
#define EMPTY_HOLE  HOLE
#define FILLED_HOLE (HOLE|PEG)

#ifdef _WIN32
static const char hole [] = ".";
static const char peg [] = "o";
#else
static const char hole [] = { 0xE2, 0x97, 0x8b, 0x0 };
static const char peg [] = { 0xE2, 0x97, 0x8f, 0x0 };
#endif

// print the board position to the specified stream

static void fprintf_board (FILE *out, board_t board)
{
    int x, y;

    fprintf (out, "   1 2 3 4 5 6 7");

    for (y = 1; y <= 7; ++y) {
        fprintf (out, "\n %d", y);
        for (x = 1; x <= 7; ++x)
            fprintf (out, " %s", (board [x] [y] & HOLE) ? (board [x] [y] & PEG ? peg : hole) : " ");
    }

    fprintf (out, "\n");
}

// print the specified moves to the specified stream

static void fprintf_game (FILE *out, move_t *moves, int num_moves)
{
    int i;

    for (i = 0; i < num_moves; ++i)
        printf ("%2d: (%d,%d) %s\n", i + 1, moves [i].x, moves [i].y,
            moves [i].dx ? (moves [i].dx < 0 ? "left" : "right") : (moves [i].dy < 0 ? "up" : "down"));
}

// main program

int main (int argc, char **argv)
{
    unsigned int games_requested = 0, game_count = 0, histogram [32];
    unsigned long long kernel;
    int i;

    if (argc > 1)
        games_requested = atol (argv [1]);

    if (!games_requested) {
        printf ("\nSearching for the first puzzle solution. When found, the final board position\n");
        printf ("will be displayed along with the move list and a histogram of all the games run.\n\n");
        printf ("You can specify the number of games to run as a single argument to the program\n");
        printf ("and in that case all solutions will be displayed. If only 1 game is requested\n");
        printf ("then the final position of that game with the moves will be displayed.\n");
    }

    kernel = time (NULL);

    for (i = 0; i < 100; ++i)
        kernel = ((kernel << 4) - kernel) ^ 1;

    memset (histogram, 0, sizeof (histogram));

    // loop until either we reached the requested number of games (if specified) or until we solve the puzzle

    while (games_requested ? game_count < games_requested : !histogram [1]) {
        move_t game [32], *gp = game;
        board_t board;
        int x, y;

        // initialize the board

        memset (board, 0, sizeof (board));

        for (x = 3; x <= 5; ++x)
            for (y = 1; y <= 7; ++y) {
                board [x] [y] = FILLED_HOLE;
                board [y] [x] = FILLED_HOLE;
            }

        board [4] [4] = EMPTY_HOLE;

        // if this is the first time through, print the board

        if (!game_count) {
            printf ("\ninitial position with board notation:\n\n");
            fprintf_board (stdout, board);
            printf ("\n");
        }

        // play game making random moves

        while (1) {
            move_t moves [32], *mp = moves;
            int num_pegs = 0, num_moves;

            // count & store the available moves for the current board position

            for (x = 1; x <= 7; ++x)
                for (y = 1; y <= 7; ++y)
                    if (board [x] [y] == FILLED_HOLE) {
                        num_pegs++;

                        if (board [x+1] [y] == FILLED_HOLE && board [x+2] [y] == EMPTY_HOLE) {
                            mp->x = x;
                            mp->y = y;
                            mp->dx = 1;
                            mp++->dy = 0;
                        }

                        if (board [x-1] [y] == FILLED_HOLE && board [x-2] [y] == EMPTY_HOLE) {
                            mp->x = x;
                            mp->y = y;
                            mp->dx = -1;
                            mp++->dy = 0;
                        }

                        if (board [x] [y+1] == FILLED_HOLE && board [x] [y+2] == EMPTY_HOLE) {
                            mp->x = x;
                            mp->y = y;
                            mp->dx = 0;
                            mp++->dy = 1;
                        }

                        if (board [x] [y-1] == FILLED_HOLE && board [x] [y-2] == EMPTY_HOLE) {
                            mp->x = x;
                            mp->y = y;
                            mp->dx = 0;
                            mp++->dy = -1;
                        }
                    }

            num_moves = (int)(mp - moves);

            // if there are no moves, we're done (that's how the game works)

            if (!num_moves) {
                histogram [num_pegs]++;

                if (games_requested == 1 || num_pegs == 1) {
                    int game_moves = (int)(gp - game);

                    printf ("\n---------------------------------\n");
                    printf ("%d peg%s left after %d moves:\n\n", num_pegs, num_pegs != 1 ? "s" : "", game_moves);
                    fprintf_board (stdout, board);
                    printf ("\n");
                    fprintf_game (stdout, game, game_moves);
                    printf ("---------------------------------\n\n");
                }

                break;
            }

            // point to the moves and if there's more than one available move, choose randomly

            mp = moves;

            if (num_moves > 1) {
                do {
                    kernel = ((kernel << 4) - kernel) ^ 1;
                    kernel = ((kernel << 4) - kernel) ^ 1;
                    kernel = ((kernel << 4) - kernel) ^ 1;
                } while ((int)(kernel >> 56) >= 256 / num_moves * num_moves);

                mp += (int)(kernel >> 56) % num_moves;
            }

            // execute & store the chosen move

            board [mp->x] [mp->y] = board [mp->x + mp->dx] [mp->y + mp->dy] = EMPTY_HOLE;
            board [mp->x + mp->dx + mp->dx] [mp->y + mp->dy + mp->dy] = FILLED_HOLE;
            *gp++ = *mp;
        }

        if (!(++game_count % 1000000))
            printf ("completed %dM games with %d solution%s\n", game_count / 1000000, histogram [1], histogram [1] != 1 ? "s" : "");
    }

    if (game_count > 1) {
        printf ("\nfinal results of %d games:\n\n", game_count);

        for (i = 1; i < 32; ++i)
            if (histogram [i]) {
                if (game_count / histogram [i] > 100)
                    printf ("%2d peg%s %10d time%s (1:%d)\n", i, i != 1 ? "s," : ", ",
                        histogram [i], histogram [i] != 1 ? "s" : " ", (game_count + histogram [i] / 2) / histogram [i]);
                else
                    printf ("%2d peg%s %10d time%s (%.1f%%)\n", i, i != 1 ? "s," : ", ",
                        histogram [i], histogram [i] != 1 ? "s" : " ", histogram [i] * 100.0 / game_count);
            }

        printf ("\n");
    }

    return 0;
}

