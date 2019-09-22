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

/* Verzia 2: Aktualna verzia programu.
 *   - Opravene nastavovanie otacok ventilatora.
 *     Povolene nested prerusenia. V obsluhe prerusenia od AD prevodnika
 *     povolene prerusenia globalne, aby sa obsluha prerusenia AD prevodnika
 *     mohla prerusit obsluhou casovaca pre generovanie PWM pre ovladanie
 *     otacok ventilatora.
 *
 * Verzia 1:
 *   - Uvodne vydanie programu.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

#include <fan_controller/delay.h>
#include <fan_controller/hw/led.h>
#include <fan_controller/hw/ad_converter.h>
#include <fan_controller/hw/fan.h>
#include <fan_controller/hw/io.h>
#include <fan_controller/hw/button.h>
#include <fan_controller/config.h>
#include <fan_controller/error.h>

/** Maska pre zasvietenie zelenych LED, pri mode konstantnych otacok (Override mode). */
#define RPM_OVERRIDE_LED_MASK 0xAA

/** Otacky pri mode konstantnych otacok (Override mode). */
#define RPM_OVERRIDE_VALUE UINT8_MAX

/** Pocet milisekund po ktore sa bude indikovat predchadzajuci nastaveny typ zobrazenia pri indikacii prepnutia zobrazenia. */
#define	SHOW_SELECTED_DISP_TYPE_PREV_DELAY 400U // ms
		
/** Pocet milisekund po ktore sa bude indikovat aktualne nastaveny typ zobrazenia pri indikacii prepnutia zobrazenia. */
#define SHOW_SELECTED_DISP_TYPE_CURRENT_DELAY 1200U // ms

static void display_set(bool display_type_changed);
static void display_real(bool display_type_changed, uint8_t rpm);
static void display_with_off(bool display_type_changed, uint8_t rpm);
static void rpm_override(bool display_type_changed);
static uint8_t show_get_display_type_mask(int display_type);
static void show_selected_display_type(int prev_display_type, int display_type);

/** Stavy stavoveho automatu pre obsluhu tlacitka. */
enum button_states {
	BUTTON_WAITING_FOR_RELEASE, /**< Cakanie na pustenie tlacitka. */
	BUTTON_WAITING_FOR_PRESS, /**< Cakanie na stlacenie tlacitka. */
	BUTTON_PRESSED, /**< Tlacitko stlacene. */
	BUTTON_SHORT_PRESS, /**< Detekovane kratke stlacenie tlacitka. */
	BUTTON_LONG_PRESS /**< Detekovane dhle stlacenie tlacitka. */
};

/**
 * @brief Hlavna funkcia programu s hlavnou sluckou.
 */
int
main(void)
{
	// Globalne povolit prerusenia
	sei();
	
	io_init();
	delay_init();
	led_init();
	fan_init();
	// Ovladanie potenciometrom musi fungovat az po pripadnej kalibracii,
	// aby neboli narusene otacky ventilatora
	adc_init();
	
	// Ziska sa ulozeny typ zobrazenia, ak existuje, inak sa vyberie predvoleny
	int prev_display_type = DISPLAY_TYPE_NONE;
	int display_type = config_disp_type_load();
	// Zobrazi sa prepnutie na zvoleny typ zobrazenia
	show_selected_display_type(prev_display_type, display_type);
	
	// Zo zaciatku sa bude cakat na pustenie tlacitka
	int button_state = BUTTON_WAITING_FOR_RELEASE;
	// Pre ulozenie deadline pre meranie dlzky stlacenia tlacitka
	uint32_t button_long_press_time = 0;
	// Pre ulozenie zvoleneho casu ktory bol predbehnuty nastavenim konstantnej hodnoty otacok dlhym stlacenim tlacitka
	int display_type_overriden = DISPLAY_TYPE_NONE;
	
	// Hlavna slucka
	while (1) {
		// Ziskat otacky pre detekciu zastavenia ventilatora (chyby) a pre pripadne zobrazenie otacok na LED
		const uint8_t rpm = fan_rpm_get();

		// Zistit, ci doslo k zmene typu zobrazenia
		const bool display_type_changed = (display_type != prev_display_type);

		// Nastavit a zobrazit prislusne otacky
		switch(display_type) {
			case DISPLAY_TYPE_SET:
				// Zobrazenie hodnoty nastavenej na potenciometri
				display_set(display_type_changed);
				break;
				
			case DISPLAY_TYPE_REAL:
				// Zobrazenie realnych otacok podla RPM signalu z ventilatora
				display_real(display_type_changed, rpm);
				break;
				
			case DISPLAY_TYPE_SET_WITH_OFF:
				// Zobrazenie hodnoty nastavenej na potenciometri, s automatickym zhasnutim LED
				display_with_off(display_type_changed, adc_get_converted_value());
				break;
				
			case DISPLAY_TYPE_REAL_WITH_OFF:
				// Zobrazenie realnych otacok podla RPM signalu z ventilatora, s automatickym zhasnutim LED
				display_with_off(display_type_changed, rpm);
				break;
				
			case DISPLAY_TYPE_OVERRIDE:
				rpm_override(display_type_changed);	
				break;
				
			default:
				// Neznama hodnota typu zobrazenia
				error();
				break;
		}
		
		// Predchadzajuci typ zobrazenie pre pouzitie v dalsej iteracii hlavnej slucky
		prev_display_type = display_type;
				
		// Obsluzi sa tlacitko
		switch(button_state) {
			case BUTTON_WAITING_FOR_RELEASE:
				if (button_is_released()) {
					// Tlaticko bolo pustene
					button_state = BUTTON_WAITING_FOR_PRESS;
				}
				break;
				
			case BUTTON_WAITING_FOR_PRESS:
				if (button_is_pressed()) {
					// Tlacitko bolo stlacene
					button_state = BUTTON_PRESSED;
					// A bude sa odmeriavat cas pre rozlisenie dlheho stlacenia
					button_long_press_time = delay_get_deadline_ms(1500);
				}
				break;
				
			case BUTTON_PRESSED:
				if (delay_has_deadline_expired(button_long_press_time)) {
					// Bol prekroceny deadline pre dlhe stlacenie
					button_state = BUTTON_LONG_PRESS;
				} else if (button_is_released()) {
					// Tlacitko bolo pustene pred uplynutim deadlinu pre dlhe stlacenie, je to kratke stlacenie
					button_state = BUTTON_SHORT_PRESS;
				}
				break;	
				
			case BUTTON_SHORT_PRESS:
				if (display_type == DISPLAY_TYPE_OVERRIDE) {
					// Kratke stlacenie tlacitka, prepne sa naspat na normalny mod aktivny pred nastavenim konstantnej hodnoty otacok (Override mod)
					// Vynuti sa inicializacia noveho zobrazenia tym, ze predchadzjuci typ bude odlisny od vsetkych moznych typov zobrazenia
					prev_display_type = DISPLAY_TYPE_NONE;
					// Obnovi sa normalny mod zobrazenia aktivny pred Override modom
					display_type = display_type_overriden;
					// Indikuje sa novo zvoleny mod
					show_selected_display_type(prev_display_type, display_type);
				} else {
					// Kratke stlacenie, prepne sa na dalsi typ
					prev_display_type = display_type;
					if (++display_type >= DISPLAY_TYPE_COUNT) {
						display_type = DISPLAY_TYPE_FIRST;
					}
					// Ulozit nastaveny typ zobrazenia
					config_disp_type_save(display_type);
					// Indikacia zvoleneho typu zobrazenia
					show_selected_display_type(prev_display_type, display_type);
				}
				// Bude sa cakat na dalsie stlacenie
				button_state = BUTTON_WAITING_FOR_PRESS;
				break;
				
			case BUTTON_LONG_PRESS:
				// Dlhe stlacenie, override otacok
				// Zalohuje sa aktualne zvoleny typ zobrazenia pre obnovu pri ukonceni nastavenia konstantnej hodnoty otacok
				display_type_overriden = display_type;
				display_type = DISPLAY_TYPE_OVERRIDE;
				// Pre dalsiu reakciu na tlacitko sa musi najprv pustit
				button_state = BUTTON_WAITING_FOR_RELEASE;
				break;
				
			default:
				// Neznamy stav tlacitka
				error();
				break;
		}
		
	}
}

/**
 * @brief Zobrazenie otacok nastavenych na potenciometri.
 *
 * @param display_type_changed Ci bolo prepnute na novy typ zobrazenia (na tento typ).
 */
void
display_set(bool display_type_changed)
{
	if (display_type_changed) {
		// Zmeneny typ zobrazenia, inicializuje sa nove zobrazenie
		adc_override_value_delete();
		led_min_green_on();
	}
	led_green_bar_show_by_value(adc_get_converted_value());	
}

/** Stavy statoveho automatu pre automaticke vypinanie LED. */
enum off_states {
	OFF_SHOW, /**< Zapnute zobrazenie. */
	OFF_SLEEP, /**< Vypnutie zobrazenie, stlmenie jasu LED. */
	OFF_WAITING_FOR_CHANGE_OR_DEADLINE, /**< Cakanie na vyprsanie casu pre vypnutie alebo na vyznamnu zmenu otacok. */
	OFF_WAITING_FOR_CHANGE /**< Cakanie na vyznamnu zmenu otacok. */
};

/**
 * @brief Zobrazenie otacok nastavenych na potenciometri, alebo realnych otacok, s automatickym vypinanim LED.
 *
 * @param display_type_changed Ci bolo prepnute na novy typ zobrazenia (na tento typ).
 * @param rpm Otacky nastavene na potenciometri, alebo realne otacky ventilatora.
 */
void
display_with_off(bool display_type_changed, uint8_t rpm)
{
	// Pre ulozenie deadlinu pre vypnutie/stlmenie zobrazenia
	static uint32_t off_time;
	// Po prepnuti na typ zobrazenia s automatickym vypinanim bude zobrazenie zapnute
	static int off_state = OFF_SHOW;
		
	if (display_type_changed) {
		// Zmeneny typ zobrazenia, inicializuje sa nove zobrazenie
		adc_override_value_delete();
		off_state = OFF_SHOW;
	}
	
	while (1) {
		switch (off_state) {
			case OFF_SHOW:
				led_min_green_on();
				led_green_bar_show_by_value(rpm);
				off_time = delay_get_deadline_ms(4000);
				off_state = OFF_WAITING_FOR_CHANGE_OR_DEADLINE;
				break;
			
			case OFF_SLEEP:
				led_min_green_set(90);
				led_green_bar_show_by_mask(0);
				off_state = OFF_WAITING_FOR_CHANGE;
				break;
			
			case OFF_WAITING_FOR_CHANGE_OR_DEADLINE:
				if (delay_has_deadline_expired(off_time)) {
					off_state = OFF_SLEEP;
				} else if (led_green_bar_will_be_changed_by_value(rpm)) {
					off_state = OFF_SHOW;
				}
				break;
			
			case OFF_WAITING_FOR_CHANGE:
				if (led_green_bar_will_be_changed_by_value(rpm)) {
					off_state = OFF_SHOW;
				}
				break;
			
			default:
				// Neznamy stav
				error();
				break;
		}
		
		// Koncove stavy, aby sa spracovavanie programu vratilo do hlavnej slucky, su stavy cakania
		if ((off_state == OFF_WAITING_FOR_CHANGE_OR_DEADLINE) || (off_state == OFF_WAITING_FOR_CHANGE)) {
			break;
		}
	}
}

/**
 * @brief Zobrazenie realnych otacok.
 *
 * @param display_type_changed Ci bolo prepnute na novy typ zobrazenia (na tento typ).
 * @param rpm Realne otacky ventilatora.
 */
void
display_real(bool display_type_changed, uint8_t rpm)
{
	if (display_type_changed) {
		// Zmeneny typ zobrazenia, inicializuje sa nove zobrazenie
		adc_override_value_delete();
		led_min_green_on();
	}
	led_green_bar_show_by_value(rpm);
}

/**
 * @brief Zobrazenie a nastavenie konstantnej hodnoty otacok ventilatora (Override mod).
 *
 * @param display_type_changed Ci bolo prepnute na novy typ zobrazenia (na tento typ).
 */
void
rpm_override(bool display_type_changed)
{
	if (display_type_changed) {
		// Zmeneny typ zobrazenia, inicializuje sa nove zobrazenie
		adc_override_value_set(RPM_OVERRIDE_VALUE);
		led_min_green_on();
		led_green_bar_show_by_mask(RPM_OVERRIDE_LED_MASK);
	}	
}

/**
 * @brief Ziskanie masky pre zasvietenie LEDm pre indikaciu typu/modu zobrazenia.
 *
 * Tato funkcia z cisla tyu zobrazenia vyrobi masku urcujucu, ako sa bude mod zobrazovat
 * na zelenych LED pouzivatelovi.
 *
 * @param display_type Typ zobrazenia.
 * @return Maska pre rozsvietenie LED pre indikaciu typu zobrazenia @p display_type.
 */
uint8_t
show_get_display_type_mask(int display_type)
{
	if (display_type >= DISPLAY_TYPE_COUNT) {
		// Neznamy typ zobrazenia
		error();
	}
	
	// Cislo modu sa prevedie na masku pre jednu rozsvietenu LED sprava
	// Mod cislo 0 rozsvieti zelenu LED najviac napravo
	// Mod cislo 1 rozsvieti druhu LED sprava, atd.
	uint8_t mask = 0x80;
	while (display_type-- > 0) {
		mask >>= 1;
	}
	
	return mask;
}

/**
 * @brief Zobrazenie prepnutia typu zobrazenia otacok.
 *
 * Najprv na zobrazi aktualne nastaveny typ a potom sa zobrazi novo nastaveny typ.
 *
 * @param prev_display_type Predchadzajuci typ zobrazenia.
 * @param display_type Aktualny typ zobrazenia.
 */
void
show_selected_display_type(int prev_display_type, int display_type)
{
	led_min_green_off();
	led_red_on();
	
	// Zobrazit predchadzajuce zvolene zobrazenie, ak nejake bolo
	if (prev_display_type < DISPLAY_TYPE_COUNT) {
		led_green_bar_show_by_mask(show_get_display_type_mask(prev_display_type));
		delay_ms(SHOW_SELECTED_DISP_TYPE_PREV_DELAY);
	}
	// Zobrazit novo zvolene zobrazenie
	led_green_bar_show_by_mask(show_get_display_type_mask(display_type));
	delay_ms(SHOW_SELECTED_DISP_TYPE_CURRENT_DELAY);
	
	led_green_bar_show_by_mask(0x00);
	led_red_off();
}