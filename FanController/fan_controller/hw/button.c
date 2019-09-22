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

#include <fan_controller/hw/button.h>
#include <fan_controller/hw/io.h>
#include <fan_controller/delay.h>

/**
 * @brief Zisti, ci je tlacitko stlacene.
 *
 * @retval true Tlacitko je stlacene.
 * @retval false Tlacitko nie je stlacene.
 */
bool
button_is_pressed(void)
{
	bool a, b;
	
	a = io_is_button_pressed();
	// Minimalna hodnota pre cakanie funkciou delay_ms() je 16 ms.
	delay_ms(20);
	b = io_is_button_pressed();
	// Stavy oboch vzoriek sa musia rovnat 
	// a musia byt rovne stlacenemu tlacitku.
	// Ak by sa nerovnali, islo by o zakmit.
	return (!a == !b) && !a && !b;
}

/**
 * @brief Zisti, ci je tlacitko pustene (nestlacene).
 *
 * @retval true Tlacitko je pustene.
 * @retval false Tlacitko nie je pustene.
 */
bool
button_is_released(void)
{
	return !button_is_pressed();
}
