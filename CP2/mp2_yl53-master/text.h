/*
 * tab:4
 *
 * text.h - font data and text to mode X conversion utility header file
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
 * Creation Date: Thu Sep  9 22:08:16 2004
 * Filename:      text.h
 * History:
 *    SL    1    Thu Sep  9 22:08:16 2004
 *        First written.
 *    SL    2    Sat Sep 12 13:40:11 2009
 *        Integrated original release back into main code base.
 */

#ifndef TEXT_H
#define TEXT_H

/* The default VGA text mode font is 8x16 pixels. */
#define FONT_WIDTH   8
#define FONT_HEIGHT  16

#define text_buffer_size 18*320
#define text_buffer_size_for_one_plane 18*320/4
#define text_buffer_size_x 320
#define text_buffer_size_y 18




#define fruit_text_buffer_size_y 24
#define fruit_text_buffer_size_x 108

#define fruit_maze_block_number_y 2
#define fruit_maze_block_number_x 9



#define big_gap_size_x 320/10
#define small_gap_size_x 320/20
#define color_offset 40
#define front_color_offset 100
#define x_offset 0

#define bar_background_color_index 40
#define bar_font_color_index 41
/* Standard VGA text font. */
extern unsigned char font_data[256][16];
extern unsigned char cycle_palette[5][3];
//cycle_palette[current index] is the color that is current showing.
extern int changing_index;


/* 
 * text_buffer_creater
 *   DESCRIPTION: Given a string, it produces a buffer that holds a graphical image of the ASCII characters in the string. this buffer is called build buffer and have same structure as build buffer.
 * 
 *   INPUTS: string. For example: Level*1-6*Fruits-00:00  * means small_gap_size_x   - means big_gap_size_x
 *   RETURN VALUE: none
 *   SIDE EFFECTS: None
 */
void text_buffer_creater(char* string,unsigned char* text_buffer);




/* 
 * change_to_normal_buffer
 *   DESCRIPTION: Given a text_buffer, it change the structure of text_buffer. meaning change to: plane3->plane2->plane1->plane0
 * 
 *   INPUTS: text_buffer and will turn the text_buffer to a buffer that have same structure with build buffer

 *   SIDE EFFECTS: None
 */
void change_to_normal_buffer(unsigned char* text_buffer);




/* 
 * fruit_text_buffer_creater
 *   DESCRIPTION: Given a string, it produces a buffer that holds a graphical image of the ASCII characters in the string. this buffer is called build buffer and have same structure as build buffer.
 * 
 *   INPUTS: string. For example: Level*1-6*Fruits-00:00  * means small_gap_size_x   - means big_gap_size_x
 *   RETURN VALUE: none
 *   SIDE EFFECTS: None
 */
void fruit_text_buffer_creater(char* string,unsigned char* text_buffer);



/* 
 * fruit_change_to_normal_buffer
 *   DESCRIPTION: Given a text_buffer, it change the structure of text_buffer. meaning change to: plane3->plane2->plane1->plane0
 * 
 *   INPUTS: text_buffer and will turn the text_buffer to a buffer that have same structure with build buffer

 *   SIDE EFFECTS: None
 */
void fruit_change_to_normal_buffer(unsigned char* text_buffer);


#endif /* TEXT_H */
