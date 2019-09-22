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

#include <fan_controller/delay.h>
#include <fan_controller/error.h>

/** Pocet milisekund pre jeden tik casovaca. */
#define DELAY_MS_FOR_1_TIMER_TICK   16U // ms

/** Pristup k hodnote systemoveho casu priamo bez zakazania presusenia (nie bezpecny). */
#define DELAY_SYSTEM_TIME_UNSAFE    (delay_system_time)

static uint32_t delay_get_system_time(void);

/** Systemovy cas. Inkrementuje sa vzdy s kazdym pretecenim Casovaca 2. */
static volatile uint32_t delay_system_time = 0;

/**
 * @brief Obsluha pretecenie Casovaca 2. Inkrementuje sa systemovy cas.
 */
ISR(TIMER2_OVF_vect)
{
	delay_system_time++;
}

/**
 * @brief Inicializacia cakania a systemoveho casu.
 */
void
delay_init(void)
{
	delay_system_time = 0;
	// Enable interrupt
	TIMSK |= (1 << TOIE2);	
}

/**
 * @brief Blokujuce cakanie zadaneho poctu milisekund. Minimalna hodnota je 16.
 * 
 * @param ms Pocet milisekund ktore sa bude cakat.
 */
void
delay_ms(unsigned int ms)
{	
	const uint32_t d = delay_get_deadline_ms(ms);
	bool end_waiting = false;
	
	while (!end_waiting) {
		// Aby sa nemuselo stale zakazovat a povolovat prerusenie, dosiahnutie konca cakanie sa detekuje najprv len priblizne
		if (DELAY_SYSTEM_TIME_UNSAFE > d) {
			// Potom sa skontroluje aj presne, tzn. hodnota systemoveho casovaca sa ziska so zakazanim prerusenia.
			if (delay_get_system_time() > d) {
				end_waiting = true;		
			}
		}
	}
}

/**
 * @brief Bezpecne ziskanie hodnoty systemoveho casu.
 * 
 * Zakaze sa prerusenie od Casovaca 2 pri pristupe k premennej zdielanej s obsluhou prerusenia.
 *
 * @return Hodnota systemoveho casovaca.
 */
uint32_t
delay_get_system_time(void)
{
	TIMSK &= ~(1 << TOIE2);
	const uint32_t t = DELAY_SYSTEM_TIME_UNSAFE;
	TIMSK |= (1 << TOIE2);
	return t;	
}

/**
 * @brief Ziskanie buducej hodnoty systemoveho casu, ktora sa dosiahne za zadany pocet milisekund.
 *
 * @param ms Pocet milisekund pre cakanie. Minimalna hodnota je 16.
 * @return Hodnota systemoveho casu, ktora sa dosiahne za zadany pocet milisekund.
 */
uint32_t
delay_get_deadline_ms(uint16_t ms)
{
	if (ms < DELAY_MS_FOR_1_TIMER_TICK) {
		error();
	}
	return (delay_get_system_time() + (ms / DELAY_MS_FOR_1_TIMER_TICK));
}

/**
 * @brief Zistenie, ci bola prekrocena zadana hodnota systemoveho casovaca.
 *
 * @param deadline Hodnota systemoveho casu z funkcie delay_get_deadline_ms().
 * @retval true Hodnota bola prekrocena.
 * @retval false Hodnota nebola prekrocena.
 */
bool
delay_has_deadline_expired(uint32_t deadline)
{
	return (delay_get_system_time() > deadline);
}