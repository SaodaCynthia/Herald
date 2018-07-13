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
#include "app_util.h"
#include "app_error.h"
#include "ble_advdata_parser.h"
#include "ble_conn_params.h"
#include "ble_hci.h"
#include "boards.h"
#include "nrf_gpio.h"
#include "app_trace.h"
#include "ble_hrs_c.h"
#include "ble_bas_c.h"
#include "ble_dis.h"
#include "app_util.h"
#include "app_timer.h"
#include "simple_ble.h"
#include "simple_adv.h"
#include "eddystone.h"
#include "fm25l04b.h"

#define DEVICE_NAME "herald"
#define PHYSWEB_URL "j2x.us/herald"

#define UVA_COMPANY_IDENTIFIER 0x02E0

#define UVA_MANDATA_SERVICE_HERALD 0x21

typedef struct {
    uint8_t service;
    uint8_t version;
    //uint8_t herald_version;
    uint8_t room_no_4;
    uint8_t room_no_3;
    uint8_t room_no_2;
    uint8_t room_no_1;
    uint32_t counter;
    uint32_t seq_no;
    //char room[10];
    
} __attribute__((packed)) herald_mandata_t;

static herald_mandata_t herald_mandata = {
    UVA_MANDATA_SERVICE_HERALD,
    1, 
    //1,
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
       10,
        (uint8_t*) &herald_mandata
    }
};

static nrf_drive_spi_t _spi=NRF_DRV_SPI_INSTANCE(0);

fm25l04b_t fm25l04b= {
	.spi = &_spi,
	.sck_pin = SPIO_CONFIG_SCK_PIN,
	.mosi_pin = SPIO_CONFIG_MOSI_PIN,
	.miso_pin= SPIO_CONFIG_MISO_PIN,
	.ss_pin = FM25L04B_nCS
};

#define MAGIC_ID 0x63F2BA02
#define FRAM_ADDR_COUNTER FRAM_ADDR_MAGIC + 4;
#define FRAM_ADDR_SEQ_NO FRAM_ADDR_COUNTER + 4;

typedef struct {
	uint32_t magic;
	uint32_t counter;
	uint32_t seq_no;
}__attribute__((packed)) herald_fram_t;

static herald_fram_t herald_fram;


void gpio_init(){
	nrf_gpio_cfg_input(4, NRF_GPIO_PIN_PULLUP); //pins as input
	nrf_gpio_cfg_input(5, NRF_GPIO_PIN_PULLUP);
	nrf_gpio_range_cfg_input(7,20, NRF_GPIO_PIN_PULLUP);
	//nrf_gpio_cfg (4,NRF_GPIO_PIN_DIR_INPUT,NRF_GPIO_PIN_INPUT_CONNECT,NRF_GPIO_PIN_NOPULL,NRF_GPIO_PIN_S0S1,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(4, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(5, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(7, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(8, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(10, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(11, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(12, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(13, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(14, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(15, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(16, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(17, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(18, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(19, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(20, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(9, NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_SENSE_HIGH);
}


uint32_t gpio_get_Sw1Data(){

	//uint32_t room_num = 0;
	//uint32_t dec_base = 1;
	uint32_t SW1_1=(nrf_gpio_pin_read(SW1_PIN1))^1;
	uint32_t SW1_2=(nrf_gpio_pin_read(SW1_PIN2))^1;
	uint32_t SW1_4=(nrf_gpio_pin_read(SW1_PIN4))^1;
	uint32_t SW1_8=(nrf_gpio_pin_read(SW1_PIN8))^1;
	uint32_t SW1_bin = SW1_8 << 3 | SW1_4 << 2 |SW1_2 << 1 | SW1_1;
	return SW1_bin;
}

uint32_t gpio_get_Sw2Data(){
	
	uint32_t SW2_1=(nrf_gpio_pin_read(SW2_PIN1))^1;
	uint32_t SW2_2=(nrf_gpio_pin_read(SW2_PIN2))^1;
	uint32_t SW2_4=(nrf_gpio_pin_read(SW2_PIN4))^1;
	uint32_t SW2_8=(nrf_gpio_pin_read(SW2_PIN8))^1;
	uint32_t SW2_bin = SW2_8 << 3 | SW2_4 << 2 |SW2_2 << 1 | SW2_1;
	return SW2_bin;
}

uint32_t gpio_get_Sw3Data(){
	
	uint32_t SW3_1=(nrf_gpio_pin_read(SW3_PIN1))^1;
	uint32_t SW3_2=(nrf_gpio_pin_read(SW3_PIN2))^1;
	uint32_t SW3_4=(nrf_gpio_pin_read(SW3_PIN4))^1;
	uint32_t SW3_8=(nrf_gpio_pin_read(SW3_PIN8))^1;
	
	uint32_t SW3_bin = SW3_8 << 3 | SW3_4 << 2 |SW3_2 << 1 | SW3_1;
	return SW3_bin;

}
	

uint32_t gpio_get_Sw4Data(){	
	uint32_t SW4_1=(nrf_gpio_pin_read(SW4_PIN1))^1;
	uint32_t SW4_2=(nrf_gpio_pin_read(SW4_PIN2))^1;
	uint32_t SW4_4=(nrf_gpio_pin_read(SW4_PIN4))^1;
	uint32_t SW4_8=(nrf_gpio_pin_read(SW4_PIN8))^1;
	uint32_t SW4_bin = SW4_8 << 3 | SW4_4 << 2 |SW4_2 << 1 | SW4_1;
	return SW4_bin;
	
}
	
	
	
	
	/*uint32_t SW1_dec = bin2dec(SW1_bin);
	uint32_t SW2_dec = bin2dec(SW2_bin);
	uint32_t SW3_dec = bin2dec(SW3_bin);
	uint32_t SW4_dec = bin2dec(SW4_bin);*/
	

	

	/*for (int i = 0; i < NUM_OF_DIGITS; i++){
		room_num = room_num + arr[i] * dec_base;
			dec_base = dec_base * 10;
	}*/

	/*uint32_t bin2dec(uint32_t n){
          
	uint32_t num = n;
	uint32_t dec = 0;
	uint32_t bin_base = 1;
	uint32_t temp = num;

	while (temp){
		temp = temp % 10;
		temp=temp/10;
		dec = dec + bin_base * temp;
		bin_base = bin_base * 2;
	}

	return dec;
}*/
// ble 
static simple_ble_config_t ble_config = {
	.platform_id = 0x00,              // used as 4th octect in device BLE address
	.device_id = DEVICE_ID_DEFAULT,
	.adv_name = DEVICE_NAME,       // used in advertisements if there is room
	.adv_interval = MSEC_TO_UNITS(500, UNIT_0_625_MS),
	.min_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS),
	.max_conn_interval = MSEC_TO_UNITS(1500, UNIT_1_25_MS),
};

int main(void) {
        
	gpio_init();
	uint32_t sw1= gpio_get_Sw1Data();
	//nrf_delay_ms(100);
	uint32_t sw2= gpio_get_Sw2Data();
	//nrf_delay_ms(100);
	uint32_t sw3= gpio_get_Sw3Data();
	//nrf_delay_ms(100);
	uint32_t sw4= gpio_get_Sw4Data();
	//nrf_delay_ms(100);
	nrf_gpio_cfg_output(29);
	nrf_gpio_pin_set(29);
	simple_ble_init(&ble_config);
	
	fm24l04b_read(&fm25l04b, FRAM_ADDR_MAGIC, (uint8_t*) &herald_fram, sizeof		(herald_fram_t));
	if (herald_fram.magic != MAGIC_ID) {
		herald_fram.magic= MAGIC_ID;
		fm25l04b_write(&fm25l04b, FRAM_ADDR_MAGIC, (uint8_t*) &herald_fram, sizeof(herald_fram_t));
		herald_fram.counter =0;
		herald_fram.seq_no = 0;
		
}
	herald_fram.counter++;
	herald_fram.sequence++;
	fm25l04b_write(&fm25l04b, FRAM_ADDR_COUNTER, ((uint8_t*) &herald_fram)+4, 8);
	herald_mandata.counter=herald_fram.counter;
	herald_mandata.seq_no= herald_fram.seq_no;

	
	
	
	//ble_advdata_manuf_data_t manuf_data;
	//uint8_array_t manuf_data_array;
	//int sw2_1=sw2<<4|sw1;
	//int sw4_3=sw4<<4|sw3;
        herald_mandata.room_no_1=sw1+0x30;
	herald_mandata.room_no_2=sw2+0x30;
	herald_mandata.room_no_3=sw3+0x30;
	herald_mandata.room_no_4=sw4+0x30;
        //uint8_t manuf_srv_data[]={UVA_MANDATA_SERVICE_HERALD,sw4_3,sw2_1};
	
	//manuf_data.company_identifier= UVA_COMPANY_IDENTIFIER;
	//manuf_data_array.p_data = manuf_srv_data;
	//manuf_data_array.size= sizeof(manuf_srv_data);
	//manuf_data.data=manuf_data_array;

	// Advertise this
	 simple_adv_only_name();
	 simple_adv_manuf_data(&mandata);
	
	 eddystone_with_manuf_adv (PHYSWEB_URL, &mandata);

	// Enter main loop
	while (1) {
		//nrf_gpio_pin_clear(29);
		//nrf_delay_ms(1000);
		//nrf_gpio_pin_set(29);
		//nrf_delay_ms(1000);
		power_manage();
		
		
	}
}

