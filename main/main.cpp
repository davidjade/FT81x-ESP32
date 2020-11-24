#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "soc/rtc.h"
#include "driver/gpio.h"

#include "EVE_commands.h"
#include "tft.h"


void loop()
{
	uint32_t current_millis;
	static uint32_t previous_millis = 0;
	static uint8_t display_delay = 0;

	uint32_t micros_start, micros_end;

	current_millis = esp_timer_get_time() / 1000ULL;
	
	if((current_millis - previous_millis) > 4) /* execute the code only every 5 milli-seconds */
	{
		previous_millis = current_millis;
		
		micros_start = esp_timer_get_time();
		TFT_touch();
		micros_end = esp_timer_get_time();
		num_profile_b = (micros_end - micros_start); /* calculate the micro-seconds passed during the call to TFT_touch */

		display_delay++;
		if(display_delay > 3) /* refresh the display every 20ms */
		{
			display_delay = 0;
			micros_start = esp_timer_get_time();
			TFT_display();
			micros_end = esp_timer_get_time();
			num_profile_a = (micros_end - micros_start); /* calculate the micro-seconds passed during the call to TFT_display */
		}
	}
}

extern "C" void app_main()
{
	gpio_pad_select_gpio(EVE_PDN);
	gpio_set_direction(EVE_PDN, GPIO_MODE_OUTPUT);

	spi_init();

	TFT_init();

	while(true)
	{
		loop();
	}
}
