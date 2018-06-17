#include <stdio.h>
#include <stdbool.h>
#include "Keypad_task.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

#define ROW_PORT	GPIOA
#define COL_PORT	GPIOB
#define	TOTAL_ROWS	4
#define TOTAL_COLS	4

static const char keypad[TOTAL_ROWS][TOTAL_COLS] = {{'1', '2', '3', 'A'},
											        {'4', '5', '6', 'B'},
											        {'7', '8', '9', 'C'},
											        {'*', '0', '#', 'D'}};

static	GPIO_TypeDef* col_ports[TOTAL_COLS] = {KEYPAD_COL_1_GPIO_Port, KEYPAD_COL_2_GPIO_Port, KEYPAD_COL_3_GPIO_Port, KEYPAD_COL_4_GPIO_Port};
static	uint16_t col_pins[TOTAL_COLS] = {KEYPAD_COL_1_Pin, KEYPAD_COL_2_Pin, KEYPAD_COL_3_Pin, KEYPAD_COL_4_Pin};

static	GPIO_TypeDef* row_ports[TOTAL_ROWS] = {KEYPAD_ROW_1_GPIO_Port, KEYPAD_ROW_2_GPIO_Port, KEYPAD_ROW_3_GPIO_Port, KEYPAD_ROW_4_GPIO_Port};
static	uint16_t row_pins[TOTAL_ROWS] = {KEYPAD_ROW_1_Pin, KEYPAD_ROW_2_Pin, KEYPAD_ROW_3_Pin, KEYPAD_ROW_4_Pin};
static 	uint8_t	debounce_counter = 0;
static  char last_key = 'Z';
static char key_pressed = 'Z';

void keypad_task_init(void)
{
	HAL_GPIO_WritePin(ROW_PORT, (KEYPAD_ROW_1_Pin | KEYPAD_ROW_2_Pin | KEYPAD_ROW_3_Pin | KEYPAD_ROW_4_Pin), GPIO_PIN_RESET);
	HAL_GPIO_WritePin(COL_PORT, (KEYPAD_COL_1_Pin | KEYPAD_COL_2_Pin | KEYPAD_COL_3_Pin | KEYPAD_COL_4_Pin), GPIO_PIN_RESET);
}



/*ROWS ARE GPIOA, COLS ARE GPIOB*/
void keypad_task_check_keyboard(void)
{
	uint8_t	row = 0, col = 0;
	bool pressed = false;
	for(row = 0; row < TOTAL_ROWS; row++)
	{
	    /*We set row by row to HIGH*/
		HAL_GPIO_WritePin(row_ports[row], row_pins[row], GPIO_PIN_SET);
		for(col = 0; col < TOTAL_COLS; col++)
		{
		    /* We check col by col if there is contact with the row that we are checking (we have set before to HIGH)*/
			if(HAL_GPIO_ReadPin(col_ports[col], col_pins[col]) == GPIO_PIN_SET)
			{
			    /*It has been pressed a key*/
                pressed = true;
                /*Is the same key than the last time?*/
                if(keypad[row][col] == last_key)
                {
                    if(debounce_counter < 3)
                    {   /*debouncing*/
                        debounce_counter++;
                    }
                }
                else
                {   /*first stroke of key*/
                    debounce_counter = 1;
                    last_key = keypad[row][col];
                }

                if (debounce_counter == 2)
                {   /* debouncing complete; key has been definitely typed*/
                    //printf("TYPED KEY %c\n", keypad[row][col]);
                    key_pressed = keypad[row][col];
                }
				break;
			}
		}
		/*Before checking next row, we set to LOW the row that we have checked*/
		HAL_GPIO_WritePin(row_ports[row], row_pins[row], GPIO_PIN_RESET);
		if (pressed == true)
		{ /*If a key was pressed, then we do not continue checking keys*/
			break;
		}
	}

	if(pressed == false)
	{   /*no key was pressed this iteration, we clear debounce counter and last key pressed*/
		debounce_counter = 0;
		last_key = 'Z';
	}
}

char keypad_task_get_key(void)
{
    return key_pressed;
}
