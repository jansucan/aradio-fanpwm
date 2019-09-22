/* Autor: Ján Suèan <jan@jansucan.sk>
 * 
 * Zdrojove kody, ich casti a subory z nich vzniknute priamo alebo nepriamo
 * (objektove subory, Intel Hex, ...) prosim nepouzivajte komercne, ani ako
 * sucast komercnych diel. Vsetky ostatne pripady pouzitia su dovolene.
 *
 * Please don't use the source codes, their parts and files created from them
 * directly or indirectly, (object files, Intel Hex files, ...) for commercial
 * purposes, not even as a part of commercial products. All other use cases
 * are allowed.
 */

#include <fan_controller/delay.h>
#include <fan_controller/error.h>
#include <fan_controller/hw/led.h>

/** Pocet milisekund periody blikania LED pri chybe. Polovicu periody su LED zasvietene a polovicu zhasnute. */
#define ERROR_BLINK_PERIOD_MS  1000U // ms

/**
 * @brief Funkcia pre indikaciu chyby blikanim LED.
 *
 * Blikaju vsetky zelene a cervena LED naraz. Tato funkcia
 * sa vola pri detekcii zastavenia ventilatora ale aj pri
 * lubovolnej dalsej chybe (napr. chyba argumentov nejakej
 * funkcie). Spracovanie programu sa z tejto funkcie uz
 * nedostane.
 */
void
error(void)
{
	while (1)
	{
		// Zasvietit LED
		led_min_green_on();
		led_green_bar_show_by_mask(0xff);
		led_red_on();
		// Pockat
		delay_ms(ERROR_BLINK_PERIOD_MS / 2);
		// Zhasnut LED 
		led_min_green_off();
		led_green_bar_show_by_mask(0x00);
		led_red_off();
		// Pockat
		delay_ms(ERROR_BLINK_PERIOD_MS / 2);
	}
}