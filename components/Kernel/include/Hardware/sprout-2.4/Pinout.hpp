#ifndef KUDZUKERNEL_HARDWARE_PINOUT_HPP
#define KUDZUKERNEL_HARDWARE_PINOUT_HPP

/**
 * The built-in led
 */
#define   PIN_BUILTIN_LED   GPIO_NUM_0

/**
 * Enablers for various peripherals
 */
#define   PIN_EN_VBUS       GPIO_NUM_2
#define   PIN_EN_RFM        GPIO_NUM_5
#define   PIN_EN_MEM        GPIO_NUM_15
#define   PIN_EN_IMU        GPIO_NUM_25
#define   PIN_EN_SARA       GPIO_NUM_33
/**
 * UARTs
 */
#define   PIN_UART0_TX      GPIO_NUM_1
#define   PIN_UART0_RX      GPIO_NUM_3
#define   PIN_UART1_TX      GPIO_NUM_17
#define   PIN_UART1_RX      GPIO_NUM_16

/**
 * Internal and external SPI Buses
 */
#define   PIN_I_MISO        GPIO_NUM_12
#define   PIN_I_MOSI        GPIO_NUM_13
#define   PIN_I_SCK         GPIO_NUM_14
#define   PIN_E_SCK         GPIO_NUM_18
#define   PIN_E_MISO        GPIO_NUM_19
#define   PIN_E_MOSI        GPIO_NUM_23

/**
 * Internal and external I2C Buses
 */
#define   PIN_I_SDA         GPIO_NUM_21
#define   PIN_I_SCL         GPIO_NUM_22
#define   PIN_E_SDA         GPIO_NUM_27
#define   PIN_E_SCL         GPIO_NUM_32

/**
 * Interrupt pins
 */
#define   PIN_INT_EXTERNAL  GPIO_NUM_34
#define   PIN_INT_RFM       GPIO_NUM_35

/**
 * Power management
 */
#define   PIN_POWER_GOOD    GPIO_NUM_39
#define   PIN_EN_HIGH_POWER GPIO_NUM_4

/**
 * Free pins (USER2 is only input)
 */
#define   PIN_USER1         GPIO_NUM_26
#define   PIN_USER2         GPIO_NUM_36

#endif
