#include <string.h>
#include <stdarg.h>
#include "stdlibo.h"
#include "splitter.h"

#define ETYPE_ANY 0
#define ETYPE_NUMBER 1
#define ETYPE_HEXB 2

/**
 * Split string to elements by splitter
 *
 * @param str string for splitting
 * @param splitter char used for splitting, for now support only one char splitting
 * @param maxels max split elemet size. Overflow this value, will break the memory
 * @param maxelc max elements in string.  Overflow this value, will break the memory
 * @return string splitter definition
 *
 * Example:
 *
 * StringSplitter *splitter = Splitter_splitString("aaa  bbb  ccc", " ", 100, 100);
 *
 * TODO: splitting by few char
 * TODO: memory overflow control
 */
StringSplitter* Splitter_splitString(char *str, char *splitter, uint32_t maxels, uint32_t maxelc) {
	uint32_t sspl = strlen(splitter);
	char *sraw = (char*) malloc(sspl);
	char *left = (char*) malloc(maxels);
	char **elems = (char**) malloc(maxelc);
	memcpy(sraw, splitter, sspl);

	uint32_t elc = 0;
	uint32_t splp = 0;
	uint32_t leftp = 0;

	for (uint32_t i = 0; i < strlen(str); i++) {
		if (splp == 0) {
			if (str[i] != sraw[splp]) {
				left[leftp] = str[i];
				leftp++;
			} else {
				if (leftp > 0) {
					left[leftp] = 0;
					elems[elc] = left;
					left = (char*) malloc(maxels);
					elc++;
					leftp = 0;
					splp++;
				}
			}
		} else {
			if (splp + 1 > sspl)
				splp = sspl - 1;
			if (str[i] != sraw[splp]) {
				splp = 0;
				i--;
			} else {
				splp++;
			}
		}
	}

	if (leftp > 0) {
		left[leftp] = 0;
		elems[elc] = left;
		elc++;
	} else {
		free(left);
	}

	free(sraw);

	StringSplitter *ssplitter = (StringSplitter*) malloc(sizeof(StringSplitter));
	ssplitter->size = elc;
	ssplitter->elems = elems;

	return ssplitter;
}

/**
 * Compare string splitter with sequense of some elements. Each element is a regexp pattern
 *
 * @param splitter string spitter for comparasion
 * @param minc minimum size of the slitter requred for comparasion
 * @param cmplen conut of elements in the function call
 * @cmp sequemnse of the camaraised elements
 * @return comarasion result definition
 *
 * Example 1 (result = CMPR_SUCCESS, diff = 0):
 *
 * StringSplitter *splitter = Splitter_splitString("adc_r 1 2", " ", 100, 100);
 * StringCmpResult *cmpr = Splitter_compareString(splitter, 2, 3, "adc_r", "[0-9]+", "[0-9]+");
 *
 *
 * Example 2 (result = CMPR_SUCCESS, diff = 1):
 *
 * StringSplitter *splitter = Splitter_splitString("adc_r 1" , " ", 100, 100);
 * StringCmpResult *cmpr = Splitter_compareString(splitter, 2, 3, "adc_r", "[0-9]+", "[0-9]+");
 *
 *
 * Example 3 (result = CMPR_ERR_SPLLEN, errp = 0):
 *
 * StringSplitter *splitter = Splitter_splitString("adc_r" , " ", 100, 100);
 * StringCmpResult *cmpr = Splitter_compareString(splitter, 2, 3, "adc_r", "[0-9]+", "[0-9]+");
 *
 *
 * Example 4 (result = CMPR_ERR_MATCH, errp = 2):
 *
 * StringSplitter *splitter = Splitter_splitString("adc_r 1 x" , " ", 100, 100);
 * StringCmpResult *cmpr = Splitter_compareString(splitter, 2, 3, "adc_r", "[0-9]+", "[0-9]+");
 *
 *
 * Example 5 (result = CMPR_ERR_REGEXP, errp = 2):
 *
 * StringSplitter *splitter = Splitter_splitString("adc_r 1 x" , " ", 100, 100);
 * StringCmpResult *cmpr = Splitter_compareString(splitter, 2, 3, "adc_r", "[0-9]+", "[0+");
 *
 *
 */
StringCmpResult* Splitter_compareString(StringSplitter *splitter, uint8_t minc, uint8_t cmplen, char *cmp, ...) {
	va_list args;
	va_start(args, cmp);

	StringCmpResult *r = (StringCmpResult*) malloc(sizeof(StringCmpResult));
	r->result = CMPR_SUCCESS;
	r->perr = 0;
	r->diff = 0;

	uint8_t etype = ETYPE_ANY;

	if (splitter->size < minc) {
		r->result = CMPR_ERR_SPLLEN;
		r->perr = 0;
		r->diff = minc - splitter->size;
		va_end(args);
		return r;
	}

	for (uint8_t i = 0; i < cmplen; i++) {
		if (i > splitter->size - 1) {
			r->result = CMPR_SUCCESS;
			r->diff = cmplen - i;
			va_end(args);
			return r;
		}

		char *cmpe;
		if (i == 0)
			cmpe = cmp;
		else
			cmpe = va_arg(args, char*);

		char *cmps = splitter->elems[i];
		uint8_t err = 0;
		uint8_t regerr = 0;

		if (cmpe[0] == '*') {
			if (cmpe[1] == 'n')
				etype = ETYPE_NUMBER;
			else if (cmpe[1] == 'h')
				etype = ETYPE_HEXB;
			else if (cmpe[1] == 'a')
				etype = ETYPE_ANY;
			else
				err = 1;
		}

		if (err != 0) {
			r->result = CMPR_ERR_REGEXP;
			r->perr = i;
			va_end(args);
			return r;
		}
		if (etype == ETYPE_NUMBER) {
			for (uint8_t j = 0; j < strlen(cmps); j++) {
				if (cmps[j] < 0x30 || cmps[j] > 0x39) {
					regerr = 1;
					break;
				}
			}
		}

		if (etype == ETYPE_HEXB) {
			if (cmps[0] != '0' && cmps[1] != 'x')
				regerr = 1;
			else {
				for (uint8_t j = 2; j < strlen(cmps); j++) {
					if (cmps[j] < 0x30 || cmps[j] > 0x39) {
						if (cmps[j] < 0x41 || cmps[j] > 0x46) {
							if (cmps[j] < 0x61 || cmps[j] > 0x66) {
								regerr = 1;
								break;
							}
						}
					}
				}
			}
		}

		if (regerr != 0) {
			r->result = CMPR_ERR_MATCH;
			r->perr = i;
			va_end(args);
			return r;
		}
	}

	va_end(args);
	return r;
}


/**
 * Return splitter element or NULL if it is position is out of the size of the splitter
 *
 * @param splitter n/c
 * @param pos position
 */
char* Splitter_getStringElem(StringSplitter *splitter, uint32_t pos) {
	if (splitter->size < pos)
		return NULL;

	return splitter->elems[pos];
}
