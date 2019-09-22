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

#ifndef UTILS_H_
#define UTILS_H_

/** Nastavenie bitu b v registri r na 1. */
#define UTILS_SET_BIT(r, b)  (r |= (1 << b))
/** Nastavenie bitu b v registri r na 0. */
#define UTILS_CLR_BIT(r, b)  (r &= ~(1 << b))
/** Zistenie, ci je bit b v registri r nastaveny na 1. */
#define UTILS_IS_BIT_SET(r, b)  (r & (1 << b))

#endif /* UTILS_H_ */