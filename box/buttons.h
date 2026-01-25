/* -----------------------------------------------------------------------------
 * The Game Timer Box
 * -----------------------------------------------------------------------------
 * 
 * The HAL: Buttons functions
 *
 * Copyright (C) 2026 Alexey Fedoseev <aleksey@fedoseev.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see https://www.gnu.org/licenses/
 *
 * ----------------------------------------------------------------------------- */

#ifndef BOX_BUTTONS_HEADER
#define BOX_BUTTONS_HEADER

#include "constants.h"
#include "ports.h"

#define BUTTONS           5
#define BUTTON_TIMEOUT    50

int button_states[BUTTONS];
uint32_t button_times[BUTTONS];

void hal_buttons_init()
{
	// Setup buttons
	for (size_t i = 0; i < BUTTONS; i++) {
		int port = PORT_BUTTON_RED + i;
		pinMode(port, INPUT);
		button_times[i] = 0;
		button_states[i] = 0;
	}
}

int hal_buttons_check()
{
	int result = 0;
	uint32_t ms = millis();
	for (size_t i = 0; i < BUTTONS; i++) {
		int port = PORT_BUTTON_RED + i;
		int state = digitalRead(port);		
		uint32_t bs = button_times[i];
		if (state == HIGH) {
			if (!button_states[i]) {
				bs = button_times[i];
				if (bs == 0) {
					bs = button_times[i] = ms;
				}
				if (ms - bs >= BUTTON_TIMEOUT) {
					result |= (1 << i);
					button_states[i] = 1;
					button_times[i] = millis();
				}
			}
		} else {
			if (button_states[i]) {
				bs = button_times[i];
				if (bs == 0) {
					bs = button_times[i] = ms;
				}
				if (ms - bs >= BUTTON_TIMEOUT) {
					button_states[i] = 0;
					button_times[i] = millis();
				}
			}
		}
	}
	return result;
}

#endif
