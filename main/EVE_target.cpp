
/*********************
 *      INCLUDES
 *********************/
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "EVE_target.h"


/**********************
 *  STATIC PROTOTYPES
 **********************/
static void IRAM_ATTR spi_ready (spi_transaction_t *trans);

/**********************
 *  STATIC VARIABLES
 **********************/
static spi_device_handle_t spi;
static volatile uint8_t spi_pending_trans  = 0;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void spi_add_device_config(spi_host_device_t host, spi_device_interface_config_t *devcfg)
{
	devcfg->post_cb=spi_ready;
	esp_err_t ret=spi_bus_add_device(host, devcfg, &spi);
	assert(ret==ESP_OK);
}

void spi_add_device(spi_host_device_t host)
{
	spi_device_interface_config_t devcfg = {};
	devcfg.mode=0;							// SPI mode 0
	devcfg.clock_speed_hz=30*1000*1000;		// Clock out at 30 MHz
	devcfg.spics_io_num=EVE_CS;				// CS pin
	devcfg.queue_size=1;
	devcfg.pre_cb=NULL;
	devcfg.post_cb=NULL;

	spi_add_device_config(host, &devcfg);
}

void spi_init(void)
{
	esp_err_t ret;

	spi_bus_config_t buscfg = {};
	buscfg.mosi_io_num=EVE_SPI_MOSI;
	buscfg.miso_io_num=EVE_SPI_MISO;
	buscfg.sclk_io_num=EVE_SPI_CLK;
	buscfg.quadwp_io_num=-1;
	buscfg.quadhd_io_num=-1;
	buscfg.max_transfer_sz = 0;	// DMA default

	// Initialize the SPI bus
	ret=spi_bus_initialize(EVE_SPI_HOST, &buscfg, 1);
	assert(ret==ESP_OK);

	// Attach the LCD to the SPI bus
	spi_add_device(EVE_SPI_HOST);
}


void spi_transaction(const uint8_t* data, uint16_t length, spi_send_flag_t flags, spi_read_data* out, uint64_t addr)
{
	if (length == 0) return;           //no need to send anything

	// wait for previous pending transaction results
	spi_wait_for_pending_transactions();

	spi_transaction_ext_t t = {};

	t.base.length = length * 8; // transaction length is in bits

	if (length <= 4 && data != NULL) 
	{
		t.base.flags = SPI_TRANS_USE_TXDATA;
		memcpy(t.base.tx_data, data, length);
	} 
	else 
	{
		t.base.tx_buffer = data;
	}

	if(flags & SPI_RECEIVE) 
	{
		assert(out !=NULL && (flags & (SPI_SEND_POLLING | SPI_SEND_SYNCHRONOUS)));	// sanity check
		t.base.rx_buffer = out;
		t.base.rxlength = 0;	// default to same as tx length
	}

	if(flags & SPI_ADDRESS_24) 
	{
		t.address_bits = 24;
		t.base.addr = addr;
		t.base.flags |= SPI_TRANS_VARIABLE_ADDR;
	}

	t.base.user = (void*)flags;		// save flags for pre/post transaction processing

	// poll/complete/queue transaction
	if(flags & SPI_SEND_POLLING) 
	{
		spi_device_polling_transmit(spi, (spi_transaction_t*)&t);
	} 
	else if (flags & SPI_SEND_SYNCHRONOUS) 
	{
		spi_device_transmit(spi, (spi_transaction_t*)&t);
	} 
	else 
	{
		static spi_transaction_ext_t queuedt;		// must hang around until results retrieved via spi_device_get_trans_result()
		memcpy(&queuedt, &t, sizeof(t));
		spi_pending_trans++;
		if(spi_device_queue_trans(spi, (spi_transaction_t*)&queuedt, portMAX_DELAY) != ESP_OK) 
		{
			spi_pending_trans--;	// clear wait state
		}
	}
}


void spi_wait_for_pending_transactions(void)
{
	spi_transaction_t* presult;
	while(spi_pending_trans)
	{
		if(spi_device_get_trans_result(spi, &presult, portMAX_DELAY) == ESP_OK) 
		{
			spi_pending_trans--;
		}
	}
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void IRAM_ATTR spi_ready(spi_transaction_t *trans)
{
}


void disp_spi_acquire()
{
	esp_err_t ret = spi_device_acquire_bus(spi, portMAX_DELAY);
	assert(ret == ESP_OK);
}


void disp_spi_release()
{
	spi_device_release_bus(spi);
}

