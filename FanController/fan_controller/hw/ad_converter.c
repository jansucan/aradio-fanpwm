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
#include <stdbool.h>

#include <fan_controller/hw/ad_converter.h>
#include <fan_controller/median.h>
#include <fan_controller/hw/fan.h>

/** Pocet naposledy nameranych vzoriek z ktorych sa bude brat median. */
#define ADC_SAMPLE_COUNT 5U
	  
/** Buffer pre median nameranych vzoriek. */
static median_buffer_t adc_median_buffer;

/** Naposledy ziskany median z nameranych vzoriek. */
static uint8_t adc_converted_value;

/** Ci sa ma nastavit konstantna hodnota pre otacky, namiesto nastavenia podla hodnoty z potenciometra. */
static bool adc_is_value_overriden = false;

/** Konstanta hodnota na ktoru sa nastavia otacky ventilatora pri aktivovani pouzitia pevnej hodnoty pre otacky. */
static uint8_t adc_override_value;

/**
 * @brief Obsluha prerusenia od AD prevodnika pri dokonceni prevodu.
 *
 * Namerana hodnota sa ulozi medzi vzorky pre median, ziska sa median
 * a bud sa otacky nastavia podla ziskaneho medianu, alebo ked je
 * aktivovane pouzitie konstantnej hodnoty, bude sa napatie z potenciometra
 * ignorovat a hodnota sa bude nastavovat na zvolenu konstantu.
 */
ISR(ADC_vect)
{
	// Povolenie preruseni s vyssou prioritou (pretecenie PWM casovaca pre ovladanie otacok ventilatora)
	sei();
	
	median_save_sample(&adc_median_buffer, ADCH);
	adc_converted_value = median_get(&adc_median_buffer);
	
	if (adc_is_value_overriden) {
		fan_rpm_set(adc_override_value);
	} else {
		fan_rpm_set(adc_converted_value);
	}
}

/**
 * @brief Inicializacia AD prevodnika.
 *
 * Spusti sa AD prevodnik, bude stale konvertovat napatie
 * z potenciometra (free-running mod) a povoli sa prerusenie
 * pri dokonceni prevodu.
 */
void
adc_init(void)
{
	// Kanal ADC0
	// Napatova referencia na pine AREF MCU
	// Zarovnanie vysledku dolava
    ADMUX = (1 << ADLAR);
    
    // ADC enable
    // Najnizsia ADC frekvencia, preddelicka 128
	// Free-runnung mod predvolene v SFIOR
	// Povolenie prerusenia
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADIE) | (1 << ADATE);
	
	median_init_buffer(&adc_median_buffer, ADC_SAMPLE_COUNT);
	for (uint8_t i = 0; i < ADC_SAMPLE_COUNT; ++i) {
		median_save_sample(&adc_median_buffer, 0);
	}
	
	// Spustenie A/D prevodu
	ADCSRA |= 1 << ADSC;
}

/**
 * @brief Ziskanie hodnoty z AD prevodnika spocitanej ako median niekolkych predchadzajucich merani.
 *
 * @param Hodnota namerana AD prevodnikom.
 */
uint8_t
adc_get_converted_value(void)
{
	uint8_t v;

	// Zakaze sa prerusenie aby k premennej nepristupovala zaroven aj obsluha prerusenia
	ADCSRA &= ~(1 << ADIE);
	v = adc_converted_value; 
	// Znovu sa prerusenie povoli
	ADCSRA |= (1 << ADIE);
	return v;
}

/**
 * @brief Nastavenie konstantej hodnoty pre otacky ventilatora.
 *
 * Pri nastaveni konstantnej hodnoty sa bude ignorovat hodnota z potenciometra.
 *
 * @param val Konstantna hodnota otacok ventilatora.
 */
void
adc_override_value_set(uint8_t val)
{
	adc_override_value = val;	
	adc_is_value_overriden = true;
}

/**
 * @brief Deaktivacia konstantnej hodnoty pre nastavenie otacok ventilatora.
 *
 * Otacky budu zase nastavovane podla napatia z potenciometra.
 */
void
adc_override_value_delete(void)
{
	adc_is_value_overriden = false;
}
