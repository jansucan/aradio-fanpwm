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

#include <avr/io.h>

#include <fan_controller/hw/io.h>
#include <fan_controller/hw/led.h>
#include <fan_controller/utils.h>

static uint8_t led_green_bar_mask_from_value(uint8_t value);

/** Naposledy pouzita maska pre nastavenie stlpca LED. Je pouziva pre detekciu zmeny zeleneho stlpca LED pre automaticke vypinanie LED. */
static uint8_t led_green_bar_value_mask_last_set;

/**
 * @brief Inicializacia LED.
 *
 * Zelena LED nalavo bude svietit (s roznym jasom) vzdy, aby indikovala, ze je zariadenie zapnute.
 */
void
led_init(void)
{
	io_on_green_led_0();
}		

/**
 * @brief Zasvietenie cervenej LED.
 */
void
led_red_off(void)
{
	io_off_red_led();
}

/**
 * @brief Zhasnutie cervenej LED.
 */
void
led_red_on(void)
{
	io_on_red_led();
}        

/**
 * @brief Zhasnutie zelenej LED najviac nalavo.
 */
void
led_min_green_off(void)
{
	io_off_green_led_0();
}

/**
 * @brief Nastavenie svitu zelenej LED najviac nalavo.
 *
 * @param light Svit LED z rozsahu 0 az 255.
 */
void
led_min_green_set(uint8_t light)
{
	io_set_green_led_0(light);
}

/**
 * @brief Zasvietenie zelenej LED najviac nalavo.
 */
void
led_min_green_on(void)
{
	io_on_green_led_0();
}

/**
 * @brief Nastavenie zobrazenie jednotlivych LED zelenho stlpca (druhej zlava az poslednej zelenej LED) podla bitovej masky.
 *
 * 0. bit bitovej masky zodpoveda zelej LED druhej zlava a ostatne bity nasledujucim LED.
 *
 * @param mask Bitova maska rozsvietenia LED. Log. 0 je zhasnutie a log. 1 je zasvietenie prislusnej LED.
 */
void
led_green_bar_show_by_mask(uint8_t mask)
{
	void (*led_on[LED_GREEN_BAR_LED_COUNT])(void) = {
		io_on_green_led_1,
		io_on_green_led_2,
		io_on_green_led_3,
		io_on_green_led_4,
		io_on_green_led_5,
		io_on_green_led_6,
		io_on_green_led_7,
		io_on_green_led_8
	};
	
	void (*led_off[LED_GREEN_BAR_LED_COUNT])(void) = {
		io_off_green_led_1,
		io_off_green_led_2,
		io_off_green_led_3,
		io_off_green_led_4,
		io_off_green_led_5,
		io_off_green_led_6,
		io_off_green_led_7,
		io_off_green_led_8
	};
		
	for (uint8_t i = 0; i < LED_GREEN_BAR_LED_COUNT; ++i) {
		// Podla prislusneho bitu masky sa bud zavola funkcia pre zhasnutie alebo pre zasvietenie prislusnej LED
		if (UTILS_IS_BIT_SET(mask, i)) {
			(led_on[i])();
		} else {
			(led_off[i])();
		}
	}
}

/**
 * @brief Prevod hodnoty z rozsahu 0 az 255 na masku pre rozsvietenie zeleneho stlpca LED.
 *
 * Cim vyssia hodnota tym viac stlpec zelenych LED narastie.
 *
 * @param value Hodnota z rozsahu 0 az 255.
 * @return Maska pre rozsvietenie stlpca zelenych LED.
 */
uint8_t
led_green_bar_mask_from_value(uint8_t value)
{
	// Hranice oblasti na ktore je rozdeleny rozsah 0 az 255
	const uint8_t led_thresholds[] = {
		0+16, 32+16, 64+16, 96+16, 128+16, 160+16, 192+16, 224+16
	};
	// Pocet oblasti na ktore je rozdeleny rozsah 0 az 255
	// Kazdej oblasi zodpoveda jedna zelena LED od druhej zlava az po poslednu zelenu. Spolu 8 zelenych LED.
	const uint8_t threshold_count = sizeof(led_thresholds) / sizeof(led_thresholds[0]);

	uint8_t mask = 0x00;

	for (uint8_t i = 0; i < threshold_count; ++i) {
		if (value > led_thresholds[i]) {
			// Hodnota je vyssia ako prislusna hranica, prislusna LED sa nastavi v maske
			UTILS_SET_BIT(mask, i);
		} else {
			// Hodnota nie je vyssia ako hranicna hodnota, nema zmysel dalej porovnavat
			break;
		}
	}
	
	return mask;	
}

/**
 * @brief Zisti, ci by hodnota pre nastavenie LED zmenila pocet zasvietenych LED zeleneho stlpca.
 *
 * @param value Hodnota z rozsahu 0 az 255.
 * @retval true Hodnota by sposobila zmenu v rozsvieteni zelenych LED.
 * @retval false Hodnota by nesposobila zmenu v rozsvieteni zelenych LED.
 */
bool
led_green_bar_will_be_changed_by_value(uint8_t value)
{
	return (led_green_bar_value_mask_last_set != led_green_bar_mask_from_value(value));
}

/**
 * @brief Rozsvietenie stlpca zelenych LED podla velkosti hodnoty od 0 po 255.
 *
 * Cim vyssia hodnota tym viac stlpec zelenych LED narastie.
 *
 * @param value Hodnota z rozsahu 0 az 255.
 */
void
led_green_bar_show_by_value(uint8_t value)
{
	const uint8_t mask = led_green_bar_mask_from_value(value);
	led_green_bar_show_by_mask(mask);
	led_green_bar_value_mask_last_set = mask;
}
