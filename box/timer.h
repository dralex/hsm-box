/* -----------------------------------------------------------------------------
 * The Game Timer Box
 * -----------------------------------------------------------------------------
 * 
 * The HAL: Timer functions
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

#ifndef BOX_TIMER_HEADER
#define BOX_TIMER_HEADER

#include "constants.h"
#include "errors.h"

uint32_t timer_counter, timer_last, timer_length;
uint32_t timer_1sec_counter, timer_1sec_last;

void hal_timer_init()
{
	timer_counter = timer_last = timer_length = 0;
	timer_1sec_last = millis();
	timer_1sec_counter = 0;
}

void hal_timer_start(int sec)
{
	timer_length = (uint32_t)sec * 1000;
	timer_counter = 0;
	timer_last = millis();
	DEBUG_("Start timer on ");
	DEBUG_(timer_length);
	DEBUG(" ms.");
}

uint32_t hal_timer_left_sec()
{
	if (timer_length == 0) {
		return 0;
	}
	return (timer_length - timer_counter) / 1000;
}

void hal_timer_stop()
{
	timer_counter = timer_last = timer_length = 0;	
}

int hal_timer_check()
{
	int result = 0;
	uint32_t ms = millis();

	timer_1sec_counter += ms - timer_1sec_last;
	if (timer_1sec_counter >= 1000) {
		timer_1sec_counter -= 1000;
		result |= TIMER_FLAG_1SEC;
	}
	timer_1sec_last = ms;
	
	if (timer_length) {
		// DEBUG_("Timer counter before: ");
		// DEBUG_(timer_counter);
		timer_counter += ms - timer_last;
		// DEBUG_(" after: ");
		// DEBUG(timer_counter);
		if (timer_counter >= timer_length) {
			hal_timer_stop(); /* one shot timer */
			result |= TIMER_FLAG_TIMEOUT;
		}
		timer_last = ms;
	}
	return result;
}

#endif
