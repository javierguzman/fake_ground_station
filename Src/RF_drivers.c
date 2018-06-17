/*
 * RF_drivers.c
 *
 *  Created on: 17 de dic. de 2016
 *      Author: dejav
 */
#include "main.h"
#include "stm32f4xx_hal_spi.h"
#include "RF_drivers.h"
#include "LOG_task.h"

#define RF_DRIVERS_MOSI_PIN    RF_RECEIVE_SPI_MOSI_Pin
#define RF_DRIVERS_MISO_PIN    RF_RECEIVE_SPI_MISO_Pin
#define RF_DRIVERS_SCK_PIN     RF_RECEIVE_SPI_SCK_Pin
#define RF_DRIVERS_NSS_PIN     RF_RECEIVE_SPI_NSS_Pin
#define RF_DRIVERS_RESET_PIN   RF_RECEIVE_SPI_RESET_Pin

#define RF_DRIVERS_SPI_GPIO_PORT GPIOB
#define RF_DRIVERS_SPI_INTERFACE SPI3

inline static void rf_drivers_activate(void);

SPI_InitTypeDef   rf_drivers_spi_init;
SPI_HandleTypeDef rf_drivers_spi_handler;
GPIO_InitTypeDef  rf_drivers_spi_gpio_handler;
GPIO_InitTypeDef  rf_drivers_nss_gpio;

static bool rf_drivers_initialised = false;

static char rf_drivers_string_log[80];

static uint8_t RFM69HFreqTbl[6] = {0x87, 0xEc, 0x88, 0x80, 0x89,0x00  //434MHz
};

static uint8_t RFM69HConfigTbl[40] = {
 0x82, 0x00,        //RegDataModul      FSK Packet
 0x85, 0x02,        //RegFdevMsb        241*61Hz = 35KHz
 0x86, 0x41,        //RegFdevLsb
 0x83, 0x34,        //RegBitrateMsb     32MHz/0x3410 = 2.4kpbs
 0x84, 0x10,        //RegBitrateLsb
 0x93, 0x0F,        //RegOcp            Disable OCP
 0x98, 0x88,        //RegLNA            200R
 0x99, 0x52,        //RegRxBw           RxBW  83KHz
 0xAC, 0x00,        //RegPreambleMsb
 0xAD, 0x05,        //RegPreambleLsb    5Byte Preamble
 0xAE, 0x90,        //enable Sync.Word  2+1=3bytes
 0xAF, 0xAA,        //0xAA              SyncWord = aa2dd4
 0xB0, 0x2D,        //0x2D
 0xB1, 0xD4,        //0xD4

 0xB7, 0x00,        //RegPacketConfig1  Disable CRC£¬NRZ encode
 0xB8, 0x15,        //RegPayloadLength  21bytes for length & Fixed length
 0xBC, 0x95,        //RegFiFoThresh

 0xC8, 0x1B,        //RegTestLna        Normal sensitivity
 //0x582D,        //RegTestLna        increase sensitivity with LNA (Note: consumption also increase!)
 0xEF, 0x30,        //RegTestDAGC       Improved DAGC
 //0x6F00,        //RegTestDAGC       Normal DAGC
 0x81, 0x04        //Standby
};

static const uint8_t RFM69HRxTable[12] = {
 0x91, 0x9F,        //RegPaLevel Fifo In for Rx
 0xA5, 0x44,        //DIO Mapping for Rx
 0x93,0x1A,        //RegOcp            enable OCP
 0xDA,0x55,        //Normal and TRx
 0xDC,0x70,        //Normal and TRx
 0x81,0x10,        //Entry to Rx
};

static const uint8_t RFM69HTxTable[12] = {
 0xA5, 0x04,        //
 0x91, 0x7F,        //RegPaLevel 20dBm
 0x93, 0x0F,        //RegOcp            Disable OCP
 0xDA, 0x5D,        //High power mode
 0xDC, 0x7C,        //High power mode
 0x81, 0x0C        //Entry to Tx
};

void RF_init (bool receiver)
{
    uint8_t i;
    uint8_t read_byte = 0x00;
    /* TODO: Why this has to be const? */
    const uint8_t rf_state_reg = 0x27;

    rf_drivers_spi_init.Mode               = SPI_MODE_MASTER;
    rf_drivers_spi_init.Direction          = SPI_DIRECTION_2LINES;
    rf_drivers_spi_init.DataSize           = SPI_DATASIZE_8BIT;
    rf_drivers_spi_init.CLKPolarity        = SPI_POLARITY_LOW;
    rf_drivers_spi_init.CLKPhase           = SPI_PHASE_1EDGE;
    rf_drivers_spi_init.NSS                = SPI_NSS_HARD_OUTPUT;
    rf_drivers_spi_init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_256;
    rf_drivers_spi_init.FirstBit           = SPI_FIRSTBIT_MSB;
    rf_drivers_spi_init.TIMode             = SPI_TIMODE_DISABLE;
    rf_drivers_spi_init.CRCCalculation     = SPI_CRCCALCULATION_DISABLE;
    rf_drivers_spi_handler.Init            = rf_drivers_spi_init;
    rf_drivers_spi_handler.Instance        = RF_DRIVERS_SPI_INTERFACE;

    if (HAL_SPI_Init(&rf_drivers_spi_handler) != HAL_OK)
    {
        log_error_handler("Ground Station: SPI init error\n", RED_ALARM);
    }

    /*NSS TO LOW, SO WE SELECT THE RF*/
    for (i = 0u; i < 6u;  i += 2u)
    {
        HAL_GPIO_WritePin(RF_DRIVERS_SPI_GPIO_PORT, RF_DRIVERS_NSS_PIN, GPIO_PIN_RESET);
        HAL_SPI_Transmit(&rf_drivers_spi_handler, (uint8_t * )&RFM69HFreqTbl[i], 2, 20);
        HAL_Delay(10);
        HAL_GPIO_WritePin(RF_DRIVERS_SPI_GPIO_PORT, RF_DRIVERS_NSS_PIN, GPIO_PIN_SET);
        HAL_Delay(5);
    }
    /*base parameters*/
    for(i = 0u; i< 40u; i+=2u)
    {
        HAL_GPIO_WritePin(RF_DRIVERS_SPI_GPIO_PORT, RF_DRIVERS_NSS_PIN, GPIO_PIN_RESET);
        HAL_SPI_Transmit(&rf_drivers_spi_handler, (uint8_t *)&RFM69HConfigTbl[i], 2, 20);
        HAL_Delay(10);
        HAL_GPIO_WritePin(RF_DRIVERS_SPI_GPIO_PORT, RF_DRIVERS_NSS_PIN, GPIO_PIN_SET);
        HAL_Delay(5);
    }
    if(receiver == true)
    {
        for(i = 0u; i < 12u; i+=2u)
        {
            HAL_GPIO_WritePin(RF_DRIVERS_SPI_GPIO_PORT, RF_DRIVERS_NSS_PIN, GPIO_PIN_RESET);
            HAL_SPI_Transmit(&rf_drivers_spi_handler, (uint8_t *)&RFM69HRxTable[i], 2, 20);
            HAL_Delay(10);
            HAL_GPIO_WritePin(RF_DRIVERS_SPI_GPIO_PORT, RF_DRIVERS_NSS_PIN, GPIO_PIN_SET);
            HAL_Delay(5);
        }
    }
    else
    {
        for(i = 0u; i < 12u; i+=2u)
        {
            HAL_GPIO_WritePin(RF_DRIVERS_SPI_GPIO_PORT, RF_DRIVERS_NSS_PIN, GPIO_PIN_RESET);
            HAL_SPI_Transmit(&rf_drivers_spi_handler, (uint8_t *)&RFM69HTxTable[i], 2, 20);
            HAL_Delay(10);
            HAL_GPIO_WritePin(RF_DRIVERS_SPI_GPIO_PORT, RF_DRIVERS_NSS_PIN, GPIO_PIN_SET);
            HAL_Delay(5);
        }
    }
    /*We send last data to configure the module and we wait if the module has been configured successfully*/
    HAL_GPIO_WritePin(RF_DRIVERS_SPI_GPIO_PORT, RF_DRIVERS_NSS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&rf_drivers_spi_handler, (uint8_t *)&rf_state_reg, 1, 20);
    HAL_Delay(5);
    HAL_SPI_Receive(&rf_drivers_spi_handler, &read_byte, 1, 50);
    if(receiver == true)
    {
        if ((read_byte & 0x40) == 0x40 )
        {
            error_handler("Ground Station: Entry RX done\n", GREEN_ALARM);
        }
        else
        {
            sprintf ( rf_drivers_string_log, "Ground Station: RX NO entry; Byte read: %d\n", read_byte);
            log_error_handler(rf_drivers_string_log, ORANGE_ALARM);
        }
    }
    else
    {
        if ((read_byte & 0xA0) == 0xA0 )
        {
            error_handler("Ground Station: Entry TX done\n", GREEN_ALARM);
        }
        else
        {
            sprintf ( rf_drivers_string_log, " Ground Station: RX NO entry; Byte read: %d\n", read_byte);
            log_error_handler(rf_drivers_string_log, ORANGE_ALARM);
        }
    }

    HAL_Delay(10);
    HAL_GPIO_WritePin (RF_DRIVERS_SPI_GPIO_PORT, RF_DRIVERS_NSS_PIN, GPIO_PIN_SET);
    HAL_Delay(5);

    if ( (receiver == true) && ((read_byte & 0x40) == 0x40) )
    {
        rf_drivers_activate();
    }
    else if ( (receiver == false) && ((read_byte & 0xA0) == 0xA0) )
    {
        rf_drivers_activate();
    }
    else
    {
        /* There was an error*/
    }
}

inline static void rf_drivers_activate(void)
{
    rf_drivers_initialised = true;
}

inline bool rf_drivers_is_initialised(void)
{
    return rf_drivers_initialised;
}

void SPI3_IRQHandler (void)
{
    HAL_SPI_IRQHandler(&rf_drivers_spi_handler);
}
