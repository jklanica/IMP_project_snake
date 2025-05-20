/*
 * snake.h
 *
 *  Created on: 9. 11. 2024
 *      Author: Jan Klanica (xklani00)
 */

#ifndef INCLUDES_SNAKE_H_
#define INCLUDES_SNAKE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Macro to determine the number of elements in an array */
#define NELEMS(x)  (sizeof(x) / sizeof(x[0]))

/**
 * Enum representing the possible states of each cell on the game board.
 *
 * - ST_UP, ST_LEFT, ST_DOWN, ST_RIGHT:
 *     The cell is part of the snake's body, pointing toward the next cell
 *     closer to the head in the respective direction.
 *
 * - ST_NONE:
 *     The cell is empty (no snake segment or reward).
 *
 * - ST_REWARD:
 *     The cell contains a reward item that the snake can consume.
 */
enum State { ST_UP, ST_LEFT, ST_DOWN, ST_RIGHT, ST_NONE, ST_REWARD };

/**
 * Enum representing possible directions for the snake's movement.
 *
 * - UP, LEFT, DOWN, RIGHT:
 *     Movement directions on the board.
 *
 * - NONE:
 *     No movement specified (used as a placeholder).
 */
enum Direction { UP, LEFT, DOWN, RIGHT, NONE };

/**
 * Struct representing the snake game state.
 *
 * - board:
 *     2D array representing the game board, where each cell stores a State.
 *
 * - head:
 *     Array of two integers representing the x and y coordinates of the snake's head.
 *
 * - tail:
 *     Array of two integers representing the x and y coordinates of the snake's tail.
 */
typedef struct {
    enum State board[16][8];
    int head[2];
    int tail[2];
} SnakeGame;

/**
 * Initializes the snake game environment.
 *
 * @param sgame Pointer to a SnakeGame structure that will be initialized.
 *
 * This function sets up the game board with the snake positioned at its starting
 * location and a reward placed randomly on the board. It also initializes the
 * snake’s head and tail positions.
 */
void snake_game_init(SnakeGame* sgame);

/**
 * Executes a single move in the snake game environment.
 *
 * @param sgame Pointer to the SnakeGame structure representing the current game state.
 * @param direction The direction in which the snake should move.
 *
 * This function moves the snake one step in the specified direction, updating the
 * position of its head and adjusting the board accordingly. If the snake consumes
 * a reward, its length is extended. If the snake collides with itself or the board’s
 * boundaries, the game may end (specifics depend on game rules defined elsewhere).
 */
void snake_game_move(SnakeGame* sgame, enum Direction direction);

/**
 * Retrieves a specified column from the board as an array of ones and zeros.
 *
 * @param sgame Pointer to the SnakeGame structure representing the current game state.
 * @param chosen_col The column index (0 to 7) to retrieve.
 * @param col Array of integers where the column data will be stored (8 elements).
 *
 * The output array contains ones where the snake body or a reward is present in
 * the chosen column, and zeros elsewhere.
 */
void snake_game_get_col_as_nulls_and_zeros(SnakeGame* sgame, int chosen_col, int col[8]);

/**
 * Converts a cell state to the corresponding movement direction.
 *
 * @param state The state to convert.
 * @return The direction that corresponds to the state.
 *
 * This helper function is useful for interpreting the snake’s body orientation
 * based on the direction markers stored in the board cells.
 */
enum Direction _conv_state2direction(enum State state);

/**
 * Converts a movement direction to the corresponding cell state.
 *
 * @param direction The direction to convert.
 * @return The state that corresponds to the direction.
 *
 * This helper function is used for updating the board based on the snake's
 * movement direction.
 */
enum State _conv_direction2state(enum Direction direction);

/**
 * Computes the x and y position changes for a given movement direction.
 *
 * @param direction The movement direction.
 * @param diff Array of two integers to store the x and y differences.
 *
 * The resulting `diff` array holds the change in x and y coordinates needed to move
 * one cell in the specified direction. This is useful for calculating the next position
 * of the snake’s head.
 */
void _get_difference(enum Direction direction, int diff[2]);

/**
 * @brief Finds a random free cell on the board.
 *
 * Stores the coordinates of a free cell in `point`. If no free cell is available,
 * sets `point[0]` to -1.
 *
 * @param sgame Pointer to the SnakeGame structure.
 * @param point Array to store the free cell's coordinates or -1 if none found.
 */
void _get_random_free_point(SnakeGame* sgame, int point[2]);

/**
 * Randomly shuffles the elements of an integer array.
 *
 * @param array Pointer to the array to shuffle.
 * @param n The number of elements in the array.
 *
 * This utility function is useful for randomizing elements, such as for placing rewards
 * or selecting initial snake positions.
 */
void _shuffle(int *array, size_t n);

#endif /* INCLUDES_SNAKE_H_ */
