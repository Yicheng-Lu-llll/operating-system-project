/* 
 * tab:4
 *
 * maze.h - maze generation and display header file
 *
 * "Copyright (c) 2004-2009 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO 
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE 
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE, 
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Steve Lumetta
 * Version:       2
 * Creation Date: Thu Sep  9 22:09:54 2004
 * Filename:      maze.h
 * History:
 *    SL    1    Thu Sep  9 22:09:54 2004
 *        First written.
 *    SL    2    Sat Sep 12 14:01:15 2009
 *        Integrated original release back into main code base.
 */

#ifndef MAZE_H
#define MAZE_H

#include "blocks.h"
#include "modex.h"

#define SHOW_MIN       6  /* hide the last six pixels of boundary */

/*
 * Define maze minimum and maximum dimensions.  The description of make_maze
 * in maze.c gives details on the layout of the maze.  Minimum values are
 * chosen to ensure that a maze fills the scrolling region of the screen.
 * Maximum values are somewhat arbitrary.
 */
#define MAZE_MIN_X_DIM ((SCROLL_X_DIM + (BLOCK_X_DIM - 1) + 2 * SHOW_MIN) / (2 * BLOCK_X_DIM))
#define MAZE_MAX_X_DIM 50
#define MAZE_MIN_Y_DIM ((SCROLL_Y_DIM + (BLOCK_Y_DIM - 1) + 2 * SHOW_MIN) / (2 * BLOCK_Y_DIM))
#define MAZE_MAX_Y_DIM 30

/* bit vector of properties for spaces in the maze */
typedef enum {
    MAZE_NONE           = 0,    /* empty                                    */
    MAZE_WALL           = 1,    /* wall                                     */
    MAZE_FRUIT_1        = 2,    /* fruit (3-bit field, with 0 for no fruit) */
    MAZE_FRUIT_2        = 4,
    MAZE_FRUIT_3        = 8,  
    LAST_MAZE_FRUIT_BIT = MAZE_FRUIT_3,
    MAZE_FRUIT          = (MAZE_FRUIT_1 | MAZE_FRUIT_2 | MAZE_FRUIT_3),
    MAZE_EXIT           = 16,   /* exit from maze                           */
    MAZE_REACH          = 128   /* seen already (not shrouded in mist)      */
} maze_bit_t;



/* create a maze and place some fruits inside it */
extern int make_maze(int x_dim, int y_dim, int start_fruits);

/* fill a buffer with the pixels for a horizontal line of the maze */
extern void fill_horiz_buffer(int x, int y, unsigned char buf[SCROLL_X_DIM]);

/* fill a buffer with the pixels for a vertical line of the maze */
extern void fill_vert_buffer(int x, int y, unsigned char buf[SCROLL_Y_DIM]);

/* mark a maze location as reached and draw it onto the screen if necessary */
extern void unveil_space(int x, int y);

/* consume fruit at a space, if any; returns the fruit number consumed */
extern int check_for_fruit(int x, int y);

/* check whether the player has reached the exit with no fruits left */
extern int check_for_win(int x, int y);

/* add a new fruit randomly in the maze */
extern int add_a_fruit();

/* get pointer to the player's block image; depends on direction of motion */
extern unsigned char* get_player_block(dir_t cur_dir);

/* get pointer to the player's mask image; depends on direction of motion */
extern unsigned char* get_player_mask(dir_t cur_dir);

/* determine which directions are open to movement from a given maze point */
extern void find_open_directions(int x, int y, int op[NUM_DIRS]);


/* 
 * find_block_outside
 *   DESCRIPTION: Find the appropriate image to be used for a given maze
 *                lattice point.
 *   INPUTS: (x,y) -- the maze lattice point
 *   OUTPUTS: none
 *   RETURN VALUE: a pointer to an image of a BLOCK_X_DIM x BLOCK_Y_DIM
 *                 block of data with one byte per pixel laid out as a
 *                 C array of dimension [BLOCK_Y_DIM][BLOCK_X_DIM]
 *   SIDE EFFECTS: none
 */
extern unsigned char* find_block_outside(int x, int y);
/*this function is used to get the number of fruit.*/
extern int get_num_fruit();



/* 
 * get_fruit_message_mask
 *   DESCRIPTION: Get a graphical mask for the fruit message.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: a pointer to an image of a fruit_text_buffer_size_y*fruit_text_buffer_size_x
 *                 block of data with one byte per pixel laid out as a
 *                 C array of dimension fruit_text_buffer_size_y*fruit_text_buffer_size_x;
 *                 the bytes in this block indicate whether or not the
 *                 corresponding byte in the fruit fonts image should be
 *                 drawn (1 is drawn/opaque, 0 is not drawn/fully 
 *                 transparent)
 *   SIDE EFFECTS: none
 */
extern unsigned char* get_fruit_message_mask();



/*
this function is used to get fruit_message_indicator.

*/
extern int get_fruit_message_indicator();

/*
this function is used to get set fruit_message_indicator.

*/
void set_fruit_message_indicator();


#endif /* MAZE_H */
