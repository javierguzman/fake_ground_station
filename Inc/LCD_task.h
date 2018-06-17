/*
 * LCD_Task.h
 *
 *  Created on: 11 de dic. de 2016
 *      Author: dejav
 */

#ifndef LCD_TASK_H_
#define LCD_TASK_H_

#include <stdio.h>
/*External prototypes*/
void lcd_task_init(void);
void lcd_task_clear_screen(void);
void lcd_task_write_whole(char * str);
void lcd_task_write_line(uint8_t row, uint8_t col, char * str);
void lcd_set_init(void);
#endif /* LCD_TASK_H_ */
