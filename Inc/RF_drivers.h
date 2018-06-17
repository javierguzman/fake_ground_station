/*
 * RF_drivers.h
 *
 *  Created on: 17 de dic. de 2016
 *      Author: dejav
 */

#ifndef RF_DRIVERS_H_
#define RF_DRIVERS_H_
#include <stdbool.h>

void RF_init (bool receiver);
inline bool rf_drivers_is_initialised(void);

void SPI3_IRQHandler (void);

#endif /* RF_DRIVERS_H_ */
