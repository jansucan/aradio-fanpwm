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

#include <fan_controller/hw/io.h>
#include <fan_controller/utils.h>

#include <avr/io.h>

static void io_init_button(void);
static void io_init_fan_pwm(void);
static void io_init_fan_tachometer(void);
static void io_init_red_led(void);
static void io_init_green_led_0(void);
static void io_init_green_led_1(void);
static void io_init_green_led_2(void);
static void io_init_green_led_3(void);
static void io_init_green_led_4(void);
static void io_init_green_led_5(void);
static void io_init_green_led_6(void);
static void io_init_green_led_7(void);
static void io_init_green_led_8(void);

/**
 * @brief Inicializacia vstupov a vystupov mikrokontroleru.
 */
void
io_init(void)
{
	io_init_button();
	io_init_fan_pwm();
	io_init_fan_tachometer();
	io_init_red_led();
	io_init_green_led_0();
	io_init_green_led_1();
	io_init_green_led_2();
	io_init_green_led_3();
	io_init_green_led_4();
	io_init_green_led_5();
	io_init_green_led_6();
	io_init_green_led_7();
	io_init_green_led_8();
}

/**
 * @brief Nastavenie vstupu pre tlacitko.
 */
void
io_init_button(void)
{
	// Aktivacia pull-up rezistoru, to je treba, aby vstup "neplaval"
	UTILS_SET_BIT(PORTA, PORTA2);
}

/**
 * @brief Zistenie, ci je tlacitko stlacene, alebo nie.
 *
 * @retval true Tlacitko je stlacene.
 * @retval false Tlacitko nie je stlacene.
 */
bool
io_is_button_pressed(void)
{
	return UTILS_IS_BIT_SET(PINA, PINA2);		
}

/**
 * @brief Nastavenie vystupu pre PWM signal pre ovladanie otacok ventilatora.
 */
void
io_init_fan_pwm(void)
{
	// Nastavenie vystupu pre PWM (Timer0 a OutputCompare0)
	UTILS_SET_BIT(DDRB, PORTB3); // OC0 ako vystup
	UTILS_CLR_BIT(PORTB, PORTB3);
}

/**
 * @brief Odpojenie vystupu pre PWM signal pre ovladanie otacok ventilatora.
 */
void
io_disable_fan_pwm(void)
{
	// Deaktivacia vystupu pre PWM (Timer0 a OutputCompare0)
	UTILS_CLR_BIT(PORTB, PORTB3); // Vypnutie pull-up rezistoru
	UTILS_CLR_BIT(DDRB, PORTB3); // OC0 ako vstup v tretom stave
	
}

/**
 * @brief Nastavenie vstupu pre signal od tachometra ventilatora.
 */
void
io_init_fan_tachometer(void)
{
	// Aktivacia pull-up rezistoru, to je treba, inak je na vstupe "rusenie"
	UTILS_SET_BIT(PORTB, PORTB2);
}

/**
 * @brief Zistenie, ci je signal z tachometra v log. 1, alebo nie.
 *
 * @retval true Signal je v log. 1.
 * @retval false Signal nie je v log. 1.
 */
bool
io_is_fan_tachometer_set(void) 
{
	return UTILS_IS_BIT_SET(PINB, PINB2);	
}

/**
 * @brief Inicializacia cervenej LED.
 */
void
io_init_red_led(void)
{
	UTILS_SET_BIT(DDRA, PORTA3);
	io_off_red_led();
}

/**
 * @brief Zasvietenie cervenej LED.
 */
void
io_on_red_led(void)
{
	 UTILS_CLR_BIT(PORTA, PORTA3);
}

/**
 * @brief Zhasnutie cervenej LED.
 */
void
io_off_red_led(void)
{
	UTILS_SET_BIT(PORTA, PORTA3);
}

/**
 * @brief Inicializacia zelenej LED najviac vlavo.
 *
 * Ta bude ovladana PWM od Casovaca 2, aby sa dal regulovat jej jas.
 * Casovac 2 je zarovej pouziti implementaciu cakania v delay.c.
 */
void
io_init_green_led_0(void)
{
	UTILS_SET_BIT(DDRD, PORTD7);
	
	// LED 0 bude ovladana PWM casovaca 2
	// 62,5 Hz perioda
	TCNT2 = 0;
	// Fast PWM mode
	TCCR2 |= (1 << WGM21) | (1 << WGM20);
	// Inverted PWM
	TCCR2 |= ((1 << COM21) | (1 << COM20));
	// Prescale by 1024 = Zapnutie casovaca
	TCCR2 |= ((1 << CS22) | (1 << CS21) | (1 << CS20));
	
	io_off_green_led_0();	
}

/**
 * @brief Nastavenie maximalneho jasu zelenej LED najviac vlavo.
 *
 * Zasvietenie LED.
 */
void
io_on_green_led_0(void)
{
	// LED 0 je ovladana PWM casovaca 2
	io_set_green_led_0(UINT8_MAX);
}

/**
 * @brief Nastavenie jasu zelenej LED najviac vlavo.
 *
 * @param light Jas zelenej LED z rozsahu 0 az 255.
 */
void
io_set_green_led_0(uint8_t light)
{
	OCR2 = light;
}

/**
 * @brief Nastavenie minimalneho jasu zelenej LED najviac vlavo.
 *
 * Zhasnutie LED.
 */
void
io_off_green_led_0(void)
{
	io_set_green_led_0(0);
}

/**
 * @brief Inizializacia zelej LED 1.
 */
void
io_init_green_led_1(void)
{
	UTILS_SET_BIT(DDRC, PORTC0);
	io_off_green_led_1();
}

/**
 * @brief Zasvietenie zelej LED 1.
 */
void
io_on_green_led_1(void)
{
	UTILS_CLR_BIT(PORTC, PORTC0);
}

/**
 * @brief Zhasnutie zelej LED 1.
 */
void
io_off_green_led_1(void)
{
	UTILS_SET_BIT(PORTC, PORTC0);
}

/**
 * @brief Inizializacia zelej LED 2.
 */
void
io_init_green_led_2(void)
{
	UTILS_SET_BIT(DDRC, PORTC1);
	io_off_green_led_2();
}

/**
 * @brief Zasvietenie zelej LED 2.
 */
void
io_on_green_led_2(void)
{
	UTILS_CLR_BIT(PORTC, PORTC1);
}

/**
 * @brief Zhasnutie zelej LED 2.
 */
void
io_off_green_led_2(void)
{
	UTILS_SET_BIT(PORTC, PORTC1);
}

/**
 * @brief Inizializacia zelej LED 3.
 */
void
io_init_green_led_3(void)
{
	UTILS_SET_BIT(DDRC, PORTC3);
	io_off_green_led_3();
}

/**
 * @brief Zasvietenie zelej LED 3.
 */
void
io_on_green_led_3(void)
{
	UTILS_CLR_BIT(PORTC, PORTC3);
}

/**
 * @brief Zhasnutie zelej LED 3.
 */
void
io_off_green_led_3(void)
{
	UTILS_SET_BIT(PORTC, PORTC3);
}

/**
 * @brief Inizializacia zelej LED 4.
 */
void
io_init_green_led_4(void)
{
	UTILS_SET_BIT(DDRC, PORTC5);
	io_off_green_led_4();
}

/**
 * @brief Zasvietenie zelej LED 4.
 */
void
io_on_green_led_4(void)
{
	UTILS_CLR_BIT(PORTC, PORTC5);
}

/**
 * @brief Zhasnutie zelej LED 4.
 */
void
io_off_green_led_4(void)
{
	UTILS_SET_BIT(PORTC, PORTC5);
}

/**
 * @brief Inizializacia zelej LED 5.
 */
void
io_init_green_led_5(void)
{
	UTILS_SET_BIT(DDRC, PORTC6);
	io_off_green_led_5();
}

/**
 * @brief Zasvietenie zelej LED 5.
 */
void
io_on_green_led_5(void)
{
	UTILS_CLR_BIT(PORTC, PORTC6);
}

/**
 * @brief Zhasnutie zelej LED 5.
 */
void
io_off_green_led_5(void)
{
	UTILS_SET_BIT(PORTC, PORTC6);
}

/**
 * @brief Inizializacia zelej LED 6.
 */
void
io_init_green_led_6(void)
{
	UTILS_SET_BIT(DDRC, PORTC7);
	io_off_green_led_6();
}

/**
 * @brief Zasvietenie zelej LED 6.
 */
void
io_on_green_led_6(void)
{
	UTILS_CLR_BIT(PORTC, PORTC7);
}

/**
 * @brief Zhasnutie zelej LED 6.
 */
void
io_off_green_led_6(void)
{
	UTILS_SET_BIT(PORTC, PORTC7);
}

/**
 * @brief Inizializacia zelej LED 7.
 */
void
io_init_green_led_7(void)
{
	UTILS_SET_BIT(DDRA, PORTA7);
	io_off_green_led_7();
}

/**
 * @brief Zasvietenie zelej LED 7.
 */
void
io_on_green_led_7(void)
{
	UTILS_CLR_BIT(PORTA, PORTA7);
}

/**
 * @brief Zhasnutie zelej LED 8.
 */
void
io_off_green_led_7(void)
{
	UTILS_SET_BIT(PORTA, PORTA7);
}

/**
 * @brief Inizializacia zelej LED 8.
 */
void
io_init_green_led_8(void)
{
	UTILS_SET_BIT(DDRA, PORTA5);
	io_off_green_led_8();
}

/**
 * @brief Zasvietenie zelej LED 8.
 */
void
io_on_green_led_8(void)
{
	UTILS_CLR_BIT(PORTA, PORTA5);
}

/**
 * @brief Zhasnutie zelej LED 8.
 */
void
io_off_green_led_8(void)
{
	UTILS_SET_BIT(PORTA, PORTA5);
}