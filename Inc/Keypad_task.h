/*
 * Keypad_task.h
 *
 *  Created on: 14 de abr. de 2016
 *      Author: dejav
 */

#ifndef INCLUDE_KEYPAD_TASK_H_
#define INCLUDE_KEYPAD_TASK_H_

void keypad_task_init(void);
void keypad_task_check_keyboard(void);
char keypad_task_get_key(void);

#endif /* INCLUDE_KEYPAD_TASK_H_ */
