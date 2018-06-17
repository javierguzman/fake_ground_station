/*
 * LOG_task.h
 *
 *  Created on: 4 de ene. de 2017
 *      Author: dejav
 */

#ifndef LOG_TASK_H_
#define LOG_TASK_H_

typedef enum {GREEN_ALARM = 0, ORANGE_ALARM = 1, RED_ALARM = 1} error_level;

void log_error_handler(char * message, error_level level);

#endif /* LOG_TASK_H_ */
