/*
 * LCD_Task.c
 *
 *  Created on: 11 de dic. de 2016
 *      Author: dejav
 */

#include <LCD_task.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdbool.h"

/* RW ALWAYS LOW -> We write always*/
/* RS LOW: COMMAND; RS HIGH: DATA */
/*LCD_RS_Pin
* |LCD_RW_Pin
* |LCD_E_Pin
* |LCD_D0_Pin
  |LCD_D1_Pin|LCD_D2_Pin|LCD_D3_Pin|LCD_D4_Pin
  |LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin;
*/
/*MACROS*/
#define MAX_ROWS 4
#define MAX_COLS 20
/*internal prototypes*/
static void lcd_set_byte(uint8_t byte);
static void lcd_set_cursor(uint8_t row, uint8_t col);
static void lcd_delay(uint32_t delay);

/*constants*/
static const uint8_t row_bases[MAX_ROWS] = {0x80, 0xC0, 0x94, 0xD4};
static const uint8_t max_characters = (uint8_t)(MAX_ROWS * MAX_COLS);

/*Variables*/

typedef struct position_type
{
    uint8_t row;
    uint8_t col;
}position;

static  position position_cursor;
static  bool initialised = false;

/*Function bodies*/
void lcd_task_init(void)
{
    /*Delay 40ms as input voltage is 3V*/
    HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    /*init supposedly*/
    lcd_delay(40);

    /*Function set with 8-bit interface*/
    lcd_set_byte(0x30);
    lcd_delay(5);

    /*Function set with 8-bit interface*/
    lcd_set_byte(0x30);
    lcd_delay(1);

    /*Function set with 8-bit interface*/
    lcd_set_byte(0x30);

    /*Function set with 8-bit interface;
     * 2-lines set
     * 5x10 dots as font
     * */
    lcd_set_byte(0x3C);

    /*Display on:
     * Display on
     * Cursor off
     * Cursor blink off
     */
    lcd_set_byte(0x0C);

    lcd_set_byte(0x01);//Clear display
    /*Entry mode:
     * Increment cursor position
     * Display shift desactivated
     */
    lcd_set_byte(0x06);
    position_cursor.row = 0;
    position_cursor.col = 0;
    /*Initialisation finished*/
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
    lcd_delay(1);
    lcd_task_write_whole("LCD INIT");
    //initialised = 1u;
}

static void lcd_set_byte(uint8_t byte)
{
    HAL_GPIO_WritePin(LCD_D0_GPIO_Port, LCD_D0_Pin, (byte & 0x01));
    HAL_GPIO_WritePin(LCD_D1_GPIO_Port, LCD_D1_Pin, ((byte & 0x02)>>1));
    HAL_GPIO_WritePin(LCD_D2_GPIO_Port, LCD_D2_Pin, ((byte & 0x04)>>2));
    HAL_GPIO_WritePin(LCD_D3_GPIO_Port, LCD_D3_Pin, ((byte & 0x08)>>3));
    ///////////////////////////////////////////////////////////
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, ((byte & 0x10)>>4));
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, ((byte & 0x20)>>5));
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, ((byte & 0x40)>>6));
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, ((byte & 0x80)>>7));
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET);
    lcd_delay(1);
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
}

void lcd_task_clear_screen(void)
{
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    lcd_delay(1);
    lcd_set_byte(0x01);//Clear display
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
    position_cursor.row = 0;
    position_cursor.col = 0;
    lcd_delay(1);
}

void lcd_task_write_whole(char * str)
{
    char * aux = str;
    uint8_t iterations = 0;
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
    lcd_delay(1);
    lcd_task_clear_screen();
    while( ((*aux) != '\0') && (iterations < max_characters))
    {
        if( (*aux) == '\n')
        {
            if (position_cursor.row < (MAX_ROWS -1))
            {
                lcd_set_cursor(position_cursor.row+1, 0);
                position_cursor.col = 0;
            }
            else
            {
                /* More rows than allowed*/
                break;
            }
        }
        else
        {
            if(position_cursor.col < (MAX_COLS - 1) )
            {
                lcd_set_byte( (*aux) );
                lcd_set_cursor(position_cursor.row, position_cursor.col+1);
            }
            else if(position_cursor.col == (MAX_COLS - 1) )
            {
                lcd_set_byte( (*aux) );
                //break;
            }
            else
            {
                /*More cols than allowed; No break, maybe there is \n later*/
               // break;
            }
        }
        aux++;
        iterations++;
    }

    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    lcd_delay(1);
}

void lcd_task_write_line(uint8_t row, uint8_t col, char * str)
{
    char * aux = str;
    if( (row < MAX_ROWS) && (col < MAX_COLS))
    {
        lcd_set_cursor(row, col);
        HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
        lcd_delay(1);
        while( ( (*aux) != '\0') && (position_cursor.col < MAX_COLS))
        {
                if( (*aux) == '\n')
                {
                    if (position_cursor.row < (MAX_ROWS -1))
                    {
                        lcd_set_cursor(position_cursor.row+1, 0);
                        position_cursor.col = 0;
                        break;
                    }
                    else
                    {
                        /* More rows than allowed*/
                        break;
                    }
                }
                else
                {
                    if(position_cursor.col < (MAX_COLS - 1) )
                    {
                        lcd_set_byte( (*aux) );
                        lcd_set_cursor(position_cursor.row, (position_cursor.col+1));
                    }
                    else if(position_cursor.col == (MAX_COLS - 1) )
                    {
                        lcd_set_byte( (*aux) );
                        break;
                    }
                    else
                    {
                        /*More cols than allowed;*/
                        break;
                    }
                }
                aux++;
            }
    }
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    lcd_delay(1);
}
static void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t cursor_byte = 0x00;
    if((row < MAX_ROWS) && (col < MAX_COLS))
    {
        cursor_byte = row_bases[row] + col;
        HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
        lcd_delay(1);
        lcd_set_byte(cursor_byte);
        HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
        lcd_delay(1);
        position_cursor.row = row;
        position_cursor.col = col;
    }
    else
    {
        /*we do nothing (we do not move cursor)*/
    }
}

static void lcd_delay(uint32_t delay)
{
    if (initialised == true)
    {
        vTaskDelay(delay);
    }
    else
    {
        HAL_Delay(delay);
    }
}

void lcd_set_init(void)
{
    initialised = true;
}


