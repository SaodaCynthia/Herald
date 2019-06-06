#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_sdm.h"
#include "nrf_delay.h"
#include "ble.h"
#include "ble_advdata.h"
#include "ble_db_discovery.h"
#include "softdevice_handler.h"
#include "ble_debug_assert_handler.h"
#include "app_util_platform.h"
#include "ble_conn_params.h"
#include "ble_hci.h"
//#include "boards.h"
#include "nrf_gpio.h"
#include "ble_hrs_c.h"
#include "ble_bas_c.h"
#include "ble_dis.h"
#include "app_timer.h"
#include "simple_ble.h"
#include "simple_adv.h"
#include "eddystone.h"

#include "nrf_drv_config.h"

//#include "fm25l04b.h"

#include "nrf_drv_spi.h"




#define BLE_ADVERTISING_ENABLED 1
#define DEVICE_NAME "herald"
#define PHYSWEB_URL "j2x.us/herald"

#define UVA_COMPANY_IDENTIFIER 0x02E0

#define UVA_MANDATA_SERVICE_HERALD 0x21
#define FM25L04B_nCS 30

#define MAGIC_ID 0x63F2BA02
#define FRAM_ADDR_MAGIC 4
#define FRAM_ADDR_COUNTER FRAM_ADDR_MAGIC + 4
#define FRAM_ADDR_SEQ_NO FRAM_ADDR_COUNTER + 4

#define EDDYSTONE_ADV_DURATION APP_TIMER_TICKS(500,APP_TIMER_PRESCALER)
#define MANUF_ADV_DURATION APP_TIMER_TICKS(500,APP_TIMER_PRESCALER)

APP_TIMER_DEF(start_eddystone_adv_timer);
APP_TIMER_DEF(start_manufdata_adv_timer);

void start_eddystone_adv(void);
void start_manufdata_adv(void);
void timers_init(void);
int main(void);

typedef struct {
    uint8_t service;
    uint8_t version;
    uint8_t room_no_4;
    uint8_t room_no_3;
    uint8_t room_no_2;
    uint8_t room_no_1;
    uint32_t counter;
    uint32_t seq_no;
    
} __attribute__((packed)) herald_mandata_t;

static herald_mandata_t herald_mandata = {
    UVA_MANDATA_SERVICE_HERALD,
    1, 
    0,
    0,
    0,
    0,
    0,
    0
};

static ble_advdata_manuf_data_t mandata = {
    UVA_COMPANY_IDENTIFIER,
    {

       14,
        (uint8_t*) &herald_mandata
    }
};


// static nrf_drive_spi_t _spi=NRF_DRV_SPI_INSTANCE(0);

// fm25l04b_t fm25l04b= {
// 	.spi = &_spi,
// 	.sck_pin = SPIO_CONFIG_SCK_PIN,
// 	.mosi_pin = SPIO_CONFIG_MOSI_PIN,
// 	.miso_pin= SPIO_CONFIG_MISO_PIN,
// 	.ss_pin = FM25L04B_nCS
// };


// typedef struct {
// 	uint32_t magic;
// 	uint32_t counter;
// 	uint32_t seq_no;
// }__attribute__((packed)) herald_fram_t;

// static herald_fram_t herald_fram;


// void gpio_init(){
// 	nrf_gpio_cfg_input(4, NRF_GPIO_PIN_PULLUP); //pins as input
// 	nrf_gpio_cfg_input(5, NRF_GPIO_PIN_PULLUP);
// 	nrf_gpio_range_cfg_input(7,20, NRF_GPIO_PIN_PULLUP);
// 	nrf_gpio_cfg_sense_input(4, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(5, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(7, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(8, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(10, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(11, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(12, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(13, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(14, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(15, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(16, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(17, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(18, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(19, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(20, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// 	nrf_gpio_cfg_sense_input(9, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
// }


// uint32_t gpio_get_Sw1Data(){

// 	uint32_t SW1_1=(nrf_gpio_pin_read(SW1_PIN1))^1;
// 	uint32_t SW1_2=(nrf_gpio_pin_read(SW1_PIN2))^1;
// 	uint32_t SW1_4=(nrf_gpio_pin_read(SW1_PIN4))^1;
// 	uint32_t SW1_8=(nrf_gpio_pin_read(SW1_PIN8))^1;
// 	uint32_t SW1_bin = SW1_8 << 3 | SW1_4 << 2 |SW1_2 << 1 | SW1_1;
// 	return SW1_bin;
// }

// uint32_t gpio_get_Sw2Data(){
	
// 	uint32_t SW2_1=(nrf_gpio_pin_read(SW2_PIN1))^1;
// 	uint32_t SW2_2=(nrf_gpio_pin_read(SW2_PIN2))^1;
// 	uint32_t SW2_4=(nrf_gpio_pin_read(SW2_PIN4))^1;
// 	uint32_t SW2_8=(nrf_gpio_pin_read(SW2_PIN8))^1;
// 	uint32_t SW2_bin = SW2_8 << 3 | SW2_4 << 2 |SW2_2 << 1 | SW2_1;
// 	return SW2_bin;
// }

// uint32_t gpio_get_Sw3Data(){
	
// 	uint32_t SW3_1=(nrf_gpio_pin_read(SW3_PIN1))^1;
// 	uint32_t SW3_2=(nrf_gpio_pin_read(SW3_PIN2))^1;
// 	uint32_t SW3_4=(nrf_gpio_pin_read(SW3_PIN4))^1;
// 	uint32_t SW3_8=(nrf_gpio_pin_read(SW3_PIN8))^1;
	
// 	uint32_t SW3_bin = SW3_8 << 3 | SW3_4 << 2 |SW3_2 << 1 | SW3_1;
// 	return SW3_bin;

// }
	

// uint32_t gpio_get_Sw4Data(){	
// 	uint32_t SW4_1=(nrf_gpio_pin_read(SW4_PIN1))^1;
// 	uint32_t SW4_2=(nrf_gpio_pin_read(SW4_PIN2))^1;
// 	uint32_t SW4_4=(nrf_gpio_pin_read(SW4_PIN4))^1;
// 	uint32_t SW4_8=(nrf_gpio_pin_read(SW4_PIN8))^1;
// 	uint32_t SW4_bin = SW4_8 << 3 | SW4_4 << 2 |SW4_2 << 1 | SW4_1;
// 	return SW4_bin;
	
// }
	
static simple_ble_config_t ble_config = {
	.platform_id = 0x00,              // used as 4th octect in device BLE address
	.device_id = DEVICE_ID_DEFAULT,
	.adv_name = DEVICE_NAME,       // used in advertisements if there is room
	.adv_interval = MSEC_TO_UNITS(500, UNIT_0_625_MS),
	.min_conn_interval = MSEC_TO_UNITS(50, UNIT_1_25_MS),
	.max_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS),
};

void timers_init(void){
	uint32_t err_code;

	err_code=app_timer_create(&start_eddystone_adv_timer, APP_TIMER_MODE_SINGLE_SHOT, (app_timer_timeout_handler_t)start_eddystone_adv);
	APP_ERROR_CHECK(err_code);

	err_code=app_timer_create(&start_manufdata_adv_timer, APP_TIMER_MODE_SINGLE_SHOT, (app_timer_timeout_handler_t)start_manufdata_adv);
	APP_ERROR_CHECK(err_code);
}

void start_eddystone_adv(void){
	uint32_t err_code;
	eddystone_with_manuf_adv(PHYSWEB_URL, &mandata);

	err_code=app_timer_start(start_eddystone_adv_timer,EDDYSTONE_ADV_DURATION, NULL);

	APP_ERROR_CHECK(err_code);
}

void start_manufdata_adv(void){
	uint32_t err_code;

	uint32_t sw1= 3 ;
	
	uint32_t sw2= 2;
	
	uint32_t sw3= 1;
	
	uint32_t sw4= 0;
	

    herald_mandata.room_no_1=sw1+0x30;
	herald_mandata.room_no_2=sw2+0x30;
	herald_mandata.room_no_3=sw3+0x30;
	herald_mandata.room_no_4=sw4+0x30;
	herald_mandata.counter++;
    herald_mandata.seq_no++;

    simple_adv_manuf_data(&mandata);

    err_code=app_timer_start(start_manufdata_adv_timer, MANUF_ADV_DURATION, NULL);

	APP_ERROR_CHECK(err_code);



}




int main(void) {
        
	//gpio_init();
	// uint32_t sw1= 3 ;
	
	// uint32_t sw2= 2;
	
	// uint32_t sw3= 1;
	
	// uint32_t sw4= 0;
	

 //    herald_mandata.room_no_1=sw1+0x30;
	// herald_mandata.room_no_2=sw2+0x30;
	// herald_mandata.room_no_3=sw3+0x30;
	// herald_mandata.room_no_4=sw4+0x30;
	simple_ble_init(&ble_config);
	timers_init();
	start_manufdata_adv();
	//start_eddystone_adv();
    //simple_adv_only_name();
	
	
// 	if (herald_fram.magic != MAGIC_ID) {
// 		herald_fram.magic= MAGIC_ID;
// 		fm25l04b_write(&fm25l04b, FRAM_ADDR_MAGIC, (uint8_t*) &herald_fram, 4);
// 		herald_fram.counter =0;
// 		herald_fram.seq_no = 0;
		
// } 
// 	fm25l04b_read(&fm25l04b, FRAM_ADDR_MAGIC, (uint8_t*) &herald_fram, sizeof		(herald_fram_t));
// 	herald_fram.counter++;
// 	herald_fram.seq_no++;
// 	fm25l04b_write(&fm25l04b, FRAM_ADDR_COUNTER, ((uint8_t*) &herald_fram)+4, 8);
 //    herald_mandata.counter++;
 //    herald_mandata.seq_no++;

	// herald_mandata.counter=herald_mandata.counter;
	// herald_mandata.seq_no= herald_mandata.seq_no;


	// simple_adv_manuf_data(&mandata);
	
	// eddystone_with_manuf_adv (PHYSWEB_URL, &mandata);

	


	// Enter main loop
	while (1) {
		power_manage();
		
		
	}
}

