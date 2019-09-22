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
#include <avr/interrupt.h>

#include <math.h>

#include <fan_controller/hw/fan.h>
#include <fan_controller/error.h>
#include <fan_controller/median.h>
#include <fan_controller/hw/led.h>
#include <fan_controller/hw/io.h>
#include <fan_controller/config.h>
#include <fan_controller/delay.h>
#include <fan_controller/hw/button.h>

/** Pocet vzoriek z ktorych sa bude pocitat median pri merani sirky pulzu z tachometra ventilatora. */
#define FAN_PULSE_SAMPLE_COUNT        7

/** Pocet milisekund kolko sa bude cakat na ustalenie otacok pri novom nastaveni pri kalibracii merania otacok. */
#define FAN_RPM_STABILIZATION_MS   10000U // = 10 s

/** Pocet milisekund do kedy sa musi zachytit pulz tachometra pri merani. Inak sa bude povazovat ventilator za zastaveny a bude ohlasena chyba. */
#define FAN_RPM_TIMER_MAX_VALUE    16000U // = 1,024 s

static void     fan_pwm_enable(void);
static void     fan_pwm_disable(void);
static void     fan_error(void);
static void     fan_rpm_set_raw(uint8_t rpm);
static void     fan_rpm_sense_calibrate(void);
static uint16_t fan_get_pulse_time(void);
static uint16_t fan_get_pulse_time_median(void);

uint16_t fan_min_rpm_pulse_time;
uint16_t fan_max_rpm_pulse_time;

/** Minimalna nastavitelna hodnota otacok pre PWM. */
#define FAN_PWM_MIN                 1U

/** Maximalna nastavitelna hodnota otacok pre PWM. */
#define FAN_PWM_MAX                 79U

/** Pocet urovni otacok pre PWM. */
#define FAN_PWM_LEVEL_COUNT         (FAN_PWM_MAX - FAN_PWM_MIN + 1U)

/** Reload hodnota casovaca pre generovanie PWM o frekvencii priblizne 25 kHz. */
#define FAN_PWM_TIMER_RELOAD_VALUE  176U


/**
 * @brief Nastavenie otacok ventilatora cez PWM.
 *
 * @param rpm Hodnota otacok z rozsahu 0 az FAN_PWM_LEVEL_COUNT (vratane).
 */
void
fan_rpm_set_raw(uint8_t rpm)
{
	if (rpm >= FAN_PWM_LEVEL_COUNT) {
		// Hodnota PWM je mimo pripustneho rozsahu
		error();
	}
	// Nastavenie output-compare pre generovanie PWM signalu
	OCR0 = FAN_PWM_TIMER_RELOAD_VALUE + FAN_PWM_MIN + rpm;
}

/**
 * @brief Nastavenie otacok ventilatora.
 *
 * @param rpm Hodnota otacok z rozsahu 0 az 255.
 */
void
fan_rpm_set(uint8_t rpm)
{
	// Hodnota sa prepocita z rozsahu [0, 255] na [0, FAN_PWM_LEVEL_COUNT]
	double ratio = ((double) (FAN_PWM_LEVEL_COUNT - 1)) / UINT8_MAX;

	uint8_t v = ratio * rpm;
	
	if (v >= FAN_PWM_LEVEL_COUNT) {
		// Pretecenie pri prevode
		v = FAN_PWM_LEVEL_COUNT - 1;
	}
	// Nastavenie otacok ventilatora
	fan_rpm_set_raw(v);
}

/**
 * @brief Obsluha prerusenia od casovaca pre generovanie PWM signalu pre ovladanie otacok.
 *
 * Naplni casovac hodnotou tak, aby sa generovala PWM o frekvencii priblizne 25 kHz.
 */
ISR(TIMER0_OVF_vect)
{
	TCNT0 = FAN_PWM_TIMER_RELOAD_VALUE;
}

/**
 * @brief Inicializacia ventilatora.
 *
 * Zapne sa PWM, precita sa konfiguracia pre vypocet realnych otacok z EEPROM.
 * Ak este nebola ziadna konfiguracia vytvorena, spusti sa kalibracia merania otacok.
 * Kalibraciu meranie je mozne vyziadat vzdy stlacenim tlacitka pri resete zariadenia.
 */
void
fan_init(void)
{
	fan_pwm_enable();

	if (!config_is_fan_max_rpm_pulse_time_init() || !config_is_fan_min_rpm_pulse_time_init() || button_is_pressed()) {
		// Kalibruje sa ked je hodnota neinicializovana alebo ked je kalibracia vyziadana uzivatelom
		fan_rpm_sense_calibrate();
	}
	// Nacitaju sa casy pulzu na minimalnych a maximalnych otackach ventilatora
	fan_min_rpm_pulse_time = config_fan_min_rpm_pulse_time_load();
	fan_max_rpm_pulse_time = config_fan_max_rpm_pulse_time_load();
}

/**
 * @brief Chyba ventilatora.
 *
 * Odpoji sa PWM vstup ventilatora a vyvola sa chyba.
 */
void
fan_error(void)
{
	fan_pwm_disable();
	io_disable_fan_pwm();
	error();	
}

/**
 * @brief Kalibracia merania otacok ventilatora.
 *
 * Zmeria sa cas pulzu signalu tachometra pri minimalnych a pri maximalnych otackach.
 * Postup merania sa indikuje stlpcom zelenych LED.
 */
void
fan_rpm_sense_calibrate(void)
{
	// Cervenou LED indikovat kalibracny mod
	led_red_on();
	led_green_bar_show_by_mask(0);
	
	// Cas, ktoreho uplunutie bude indikovat jedna zelena LED
	const uint16_t time_unit = (2 * FAN_RPM_STABILIZATION_MS) / LED_GREEN_BAR_LED_COUNT;
	// Pocet rozsvietenych LED pre cakanie na ustalenie minimalnych otacok
	const uint8_t leds_for_one_wait = LED_GREEN_BAR_LED_COUNT / 2;
	// Maska pre postupne rozsvecovanie zelenych LED pre indikaciu priebehu
	uint8_t mask = 0x00;
	
	// Ziska cas cas pulzu na minimalnych otackach
	fan_rpm_set(0);
	for (uint8_t i = 0; i < leds_for_one_wait; ++i) {
		// Indikuje sa cas cakania
		delay_ms(time_unit);	
		mask = ((mask << 1) | 0x01);
		led_green_bar_show_by_mask(mask);
	}
	fan_min_rpm_pulse_time = fan_get_pulse_time_median();
	config_fan_min_rpm_pulse_time_save(fan_min_rpm_pulse_time);
	
	// Ziska sa cas pulzu na maximalnych otackach
	fan_rpm_set(UINT8_MAX);
	for (uint8_t i = 0; i < leds_for_one_wait; ++i) {
		// Indikuje sa cas cakania
		delay_ms(time_unit);
		mask = ((mask << 1) | 0x01);
		led_green_bar_show_by_mask(mask);
	}
	fan_max_rpm_pulse_time = fan_get_pulse_time_median();
	// Namerane vysledky sa zapisu do EEPROM, aby sa nemusela kalibracia spustat pri dalsom zapnuti zariadenia
	config_fan_max_rpm_pulse_time_save(fan_max_rpm_pulse_time);
	
	// Nastavi sa kludova hodnota otacok = najnizsie otacky
	fan_rpm_set(0);
	
	// Indikacia ukoncenia kalibracneho rezimu
	led_red_off();
	led_green_bar_show_by_mask(0);
}

/**
 * @brief Ziskanie relativnej realnej hodnoty otacok.
 *
 * Zmeria sa sirka pulzu z tachometra a z rozsahu [sirka_pulzu_pri_max_otackach, sirka_pulzu_pri_min_otackach]
 * sa prepocita do rozsahu 0 az 255.
 *
 * @return Relativna hodnota otacok z rozsahu 0 az 255.
 */
uint8_t
fan_rpm_get(void)
{	
	const float pulse_time_range = (float) (fan_min_rpm_pulse_time - fan_max_rpm_pulse_time);
	
	uint16_t pt = fan_get_pulse_time();
	// Korekcia nameraneho casu pulzu do platneho rozsahu (rozsahu zmeranom pri kalibracii)
	if (pt < fan_max_rpm_pulse_time) {
		pt = fan_max_rpm_pulse_time;
	} else if (pt > fan_min_rpm_pulse_time) {
		pt = fan_min_rpm_pulse_time;
	}
	
	const float pt_relative = pt - fan_max_rpm_pulse_time;
	// Pomer nameranej hodnoty sirky pulzu k celemu rozsahu sirky pulzu z tachometra
	float ratio = pt_relative / pulse_time_range;
	// Osetrenie pretecenia a podtecenie pri aritm. operaciach
	if (ratio < 0.0) {
		ratio = 0.0;
	} else if (ratio > 1.0) {
		ratio = 1.0;
	}
	
	// Aplikuje sa korekcia zobrazenia, aby merane otacky mali viac linearny priebeh
	ratio = (ratio * 9.0) + 1.0;
	ratio = log10(ratio);
	ratio = (ratio * 9.0) + 1.0;
	ratio = log10(ratio);
	// Osetrenie pretecenia a podtecenie pri aritm. operaciach
	if (ratio < 0.0) {
		ratio = 0.0;
	} else if (ratio > 1.0) {
		ratio = 1.0;
	}

	return (UINT8_MAX - ((uint8_t) (((float) UINT8_MAX) * ratio)));
}

/**
 * @brief Zmeranie sirky pulzu z tachometra ventilatora.
 *
 * Pri meranie sa detekuje zastavenie ventilatora a ak nastalo, vyvola sa chyba.
 *
 * @return Sirka pulzu z tachometra ventilatora.
 */
uint16_t
fan_get_pulse_time(void)
{
	// Zacina sa s meranim sirky pulzu, casovac je zastaveny
	TCNT1 = 0x0000;
	
	// Pocka sa na nabeznu hranu
	// Pocka sa na log. 0 RPM signalu
	while (io_is_fan_tachometer_set()) {
		// Mozne chyby pristup po 8-bitovych polovickach sa ignoruju
		if (TCNT1 > FAN_RPM_TIMER_MAX_VALUE) {
			// Detekovane zastavenie ventilatora
			fan_error();
		}
	}
	
	// Pocka sa na log. 1 RPM signalu
	while (!io_is_fan_tachometer_set()) {
		if (TCNT1 > FAN_RPM_TIMER_MAX_VALUE) {
			// Detekovane zastavenie ventilatora
			fan_error();
		}
	}
	
	// Zapnutie casovaca 1, preddelicka 1024
	TCCR1B |= ((1 << CS12) | (1 << CS10));
	// Pocka sa na log. 0 RPM signalu
	while (io_is_fan_tachometer_set()) {
		if (TCNT1 > FAN_RPM_TIMER_MAX_VALUE) {
			// Detekovane zastavenie ventilatora
			fan_error();
		}
	}
	// Konci s s meranim sirky pulzu, vypnutie casovaca 1
	TCCR1B &= ~((1 << CS12) | (1 << CS10));
	
	// Ziskanie nameranej hodnoty
	return TCNT1;
}

/**
 * @brief Zmeranie sirky pulzu z tachometra ventilatora vypocitaneho ako median z viacerych nameranych hodnot.
 *
 * Pri meranie sa detekuje zastavenie ventilatora a ak nastalo, vyvola sa chyba.
 *
 * @return Sirka pulzu z tachometra ventilatora vypocitana ako median z viacerych nameranych hodnot.
 */
uint16_t
fan_get_pulse_time_median(void)
{
	median_buffer_t m;
	
	median_init_buffer(&m, FAN_PULSE_SAMPLE_COUNT);
	for (uint8_t i = 0; i < FAN_PULSE_SAMPLE_COUNT; i++) {
		median_save_sample(&m, fan_get_pulse_time());
	}
	return median_get(&m);
}

/**
 * @brief Aktivovanie generovania PWM signalu pre ovladanie otacok ventilatora.
 *
 * Nastavi sa Casovac 0 a jemu prislusny Output Compare register.
 */
void
fan_pwm_enable(void)
{
	// 25 KHz perioda
	TCNT0 = FAN_PWM_TIMER_RELOAD_VALUE;
	// Povoli sa prerusenie od casovaca pre generovanie PWM
	TIMSK |= 1 << TOIE0;
	// Nastavi sa najnizsia kludova hodnota otacok
	fan_rpm_set(0);
	// Fast PWM mode
	TCCR0 |= (1 << WGM01) | (1 << WGM00);
	// Non-inverted PWM
	TCCR0 |= (1 << COM01);
	// Prescale by 8 = Zapnutie casovaca
	TCCR0 |= (1 << CS01);
}

/**
 * @brief Deaktivovanie generovania PWM signalu pre ovladanie otacok ventilatora.
 *
 * Zastavi sa Casovac 0 a odpoji sa vystup z Output Compare od GPIO pinu.
 */
void
fan_pwm_disable(void)
{
	// Vypnutie casovaca
	TCCR0 &= ~((1 << CS02) | (1 << CS01) | (1 << CS00));
	// Odpojenie OC0
	TCCR0 &= ~((1 << COM01) | (1 << COM00));
}