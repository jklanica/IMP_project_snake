/*
 * snake.c
 *
 *  Created on: 9. 11. 2024
 *      Author: Jan Klanica (xklani00)
 */

#include "snake.h"

void snake_game_init(SnakeGame* sgame)
{
    // Initialize head and tail positions
    sgame->head[0] = 8;
    sgame->head[1] = 3;
    sgame->tail[0] = 10;
    sgame->tail[1] = 3;

    // Set all cells on the board to ST_NONE (empty)
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 8; y++) {
            sgame->board[x][y] = ST_NONE;
        }
    }

    // Initialize snake's initial position
    sgame->board[8][3] = ST_LEFT;
    sgame->board[9][3] = ST_LEFT;
    sgame->board[10][3] = ST_LEFT;

    // Place the initial reward randomly on the board
    int reward_pos[2];
    _get_random_free_point(sgame, reward_pos);
    sgame->board[reward_pos[0]][reward_pos[1]] = ST_REWARD;
}

void snake_game_move(SnakeGame* sgame, enum Direction direction)
{
    // Retrieve the current direction of the snake’s head
    enum Direction current_direction = _conv_state2direction(sgame->board[sgame->head[0]][sgame->head[1]]);

    // Prevent moving in the opposite direction or no direction
    if (direction == NONE || direction == (current_direction + 2) % 4) {
        direction = current_direction;
    }
    enum State head_state = _conv_direction2state(direction);

    // Calculate the new head position based on the direction
    int diff[2];
    _get_difference(direction, diff);
    int new_head[2] = {(sgame->head[0] + diff[0] + 16) % 16, (sgame->head[1] + diff[1] + 8) % 8};

    // Check if snake ate a reward
    bool ate_reward = (sgame->board[new_head[0]][new_head[1]] == ST_REWARD);

    // If no reward was eaten, move the tail
	if (!ate_reward) {
		// Calculate new tail position
		_get_difference(_conv_state2direction(sgame->board[sgame->tail[0]][sgame->tail[1]]), diff);
		int new_tail[2] = {(sgame->tail[0] + diff[0] + 16) % 16, (sgame->tail[1] + diff[1] + 8) % 8};

		// Clear old tail position and update to new tail position
		sgame->board[sgame->tail[0]][sgame->tail[1]] = ST_NONE;
		sgame->tail[0] = new_tail[0];
		sgame->tail[1] = new_tail[1];
	} else {
		// Place a new reward randomly on the board
		int reward_pos[2];
		_get_random_free_point(sgame, reward_pos);

		if (reward_pos[0] == -1) {
			// No free space on board, the game is won
			snake_game_init(sgame);
			return;
		}

		sgame->board[reward_pos[0]][reward_pos[1]] = ST_REWARD;
	}

    // Check if snake collided with itself
    enum State new_head_state = sgame->board[new_head[0]][new_head[1]];
    bool collided_with_self = (new_head_state == ST_UP || new_head_state == ST_DOWN ||
                               new_head_state == ST_LEFT || new_head_state == ST_RIGHT);
    if (collided_with_self) {
        snake_game_init(sgame);
        return;
    }

    // Update previous head to point in the new direction
    sgame->board[sgame->head[0]][sgame->head[1]] = head_state;

    // Update head position
    sgame->head[0] = new_head[0];
    sgame->head[1] = new_head[1];

    // Mark new head on the board
    sgame->board[sgame->head[0]][sgame->head[1]] = head_state;
}

void snake_game_get_col_as_nulls_and_zeros(SnakeGame* sgame, int chosen_col, int col[8])
{
    for (int y = 0; y < 8; y++) {
        col[y] = (sgame->board[chosen_col][y] == ST_UP ||
                  sgame->board[chosen_col][y] == ST_DOWN ||
                  sgame->board[chosen_col][y] == ST_LEFT ||
                  sgame->board[chosen_col][y] == ST_RIGHT ||
                  sgame->board[chosen_col][y] == ST_REWARD);
    }
}

void _get_random_free_point(SnakeGame* sgame, int point[2])
{
    int cols[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    int rows[] = {0,1,2,3,4,5,6,7};
    _shuffle(cols, NELEMS(cols));
    _shuffle(rows, NELEMS(rows));

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            if (sgame->board[cols[i]][rows[j]] == ST_NONE) {
                point[0] = cols[i];
                point[1] = rows[j];
                return;
            }
        }
    }

    // No free space available
    point[0] = -1;
}

void _shuffle(int *array, size_t n)
{
    for (size_t i = 0; i < n - 1; i++) {
        size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
        int temp = array[j];
        array[j] = array[i];
        array[i] = temp;
    }
}

enum Direction _conv_state2direction(enum State state)
{
    switch (state) {
        case ST_UP:    return UP;
        case ST_DOWN:  return DOWN;
        case ST_LEFT:  return LEFT;
        case ST_RIGHT: return RIGHT;
        default:       return UP; // Default to UP in case of an unexpected state
    }
}

enum State _conv_direction2state(enum Direction direction)
{
    switch (direction) {
        case UP:    return ST_UP;
        case DOWN:  return ST_DOWN;
        case LEFT:  return ST_LEFT;
        case RIGHT: return ST_RIGHT;
        default:    return ST_NONE;
    }
}

void _get_difference(enum Direction direction, int diff[2])
{
    switch (direction) {
        case UP:
            diff[0] = 0;
            diff[1] = -1;
            break;
        case DOWN:
            diff[0] = 0;
            diff[1] = 1;
            break;
        case LEFT:
            diff[0] = -1;
            diff[1] = 0;
            break;
        default:
            diff[0] = 1;
            diff[1] = 0;
            break;
    }
}
