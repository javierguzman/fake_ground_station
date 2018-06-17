/*
 * Ground_control_task.c
 *
 *  Created on: 14 de dic. de 2016
 *      Author: dejav
 */

#include <string.h>
#include "Keypad_task.h"
#include "LCD_task.h"
#include "stdbool.h"



static  uint16_t GCT_altitude = 0u;
static  uint16_t GCT_speed = 0u;
static  char     last_state = 'Z';

typedef enum {ALTITUDE, SPEED, DEFAULT} state_showed_type;

static void ground_control_task_process_action(char key);
static void ground_control_task_process_print(bool repeated, state_showed_type state);

/*Function to initialise ground control variables*/
void ground_control_task_init (void)
{
    static char init_message[20] = "  GROUND STATION \n";//   INITIALISED";
    lcd_task_clear_screen();
    lcd_task_write_line(0, 0, init_message);
}

/*Function to control*/
void ground_control_task_main(void)
{
    static char last_key_pushed = 'Z';
    //Actualizar datos from RF module!!
    // TODO: Update data from RF Receive Module

    //Actualizar last key pushed
    last_state = last_key_pushed;
    last_key_pushed = keypad_task_get_key();
    //Action due to key
    ground_control_task_process_action(last_key_pushed);
}

static void ground_control_task_process_action(char key)
{
    static state_showed_type state_showed = DEFAULT;
    bool repeated = (last_state == key) ? (repeated = true) : (repeated= false);

    switch(key)
    {
        case '*':
                    /*ignition; Obtained not from RF*/
                    break;
        case '#':
                    /*parachute; Obtained not from RF*/
                    break;
        case '1':   /*altitude*/
                    state_showed = ALTITUDE;
                    break;
        case '2':   /*speed*/
                    state_showed = SPEED;
                    break;
        case '3':   /*GNSS*/
                    break;
        default:    state_showed = DEFAULT;
                    break;
    }
    ground_control_task_process_print(repeated, state_showed);
}

static void ground_control_task_process_print (bool repeated, state_showed_type state)
{
    static char string_message1[20];
    static char string_valueunit[20] = "";
    uint8_t row = 1, col = 0;
    switch(state)
    {
            case ALTITUDE:  strcpy (string_message1,"Altitude: ");
                            col = 9;
                            sprintf ( string_valueunit , "%d m", GCT_altitude);
                            break;

            case SPEED:     strcpy (string_message1,"Speed: ");
                            col = 6;
                            sprintf ( string_valueunit , "%d km/h", GCT_speed);
                            break;

            default:        strcpy (string_message1,"  GROUND STATION \n");
                            break;
    }

    if (!repeated)
    {
        lcd_task_clear_screen();
        lcd_task_write_line(0, 0, string_message1);
    }

    if(state != DEFAULT)
    {
        lcd_task_write_line(row, col, string_valueunit);
    }
}
