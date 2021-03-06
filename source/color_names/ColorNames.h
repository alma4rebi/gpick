/*
 * Copyright (c) 2009-2016, Albertas Vyšniauskas
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of the software author nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GPICK_COLOR_NAMES_COLOR_NAMES_H_
#define GPICK_COLOR_NAMES_COLOR_NAMES_H_

#include "../Color.h"
#include <string>
#include <list>

typedef struct ColorNameEntry{
	std::string name;
}ColorNameEntry;
typedef struct ColorEntry{
	Color color;
	ColorNameEntry* name;
}ColorEntry;
typedef struct ColorNames{
	std::list<ColorNameEntry*> names;
	std::list<ColorEntry*> colors[8][8][8];
	void (*color_space_convert)(const Color* a, Color* b);
	float (*color_space_distance)(const Color* a, const Color* b);
}ColorNames;
ColorNames* color_names_new();
int color_names_load_from_file(ColorNames* cnames, const char* filename);
void color_names_destroy(ColorNames* cnames);
std::string color_names_get(ColorNames* cnames, const Color* color, bool imprecision_postfix);

#endif /* GPICK_COLOR_NAMES_COLOR_NAMES_H_ */
