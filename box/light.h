/* -----------------------------------------------------------------------------
 * The Game Timer Box
 * -----------------------------------------------------------------------------
 * 
 * The HAL: LED functions
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

#ifndef BOX_LIGHT_HEADER
#define BOX_LIGHT_HEADER

#include <LCD_I2C.h>

#include "constants.h"
#include "ports.h"

int red_led_state, green_led_state;

void hal_led_init()
{
	// Setup output ports
	pinMode(PORT_LED_RED, OUTPUT);
	pinMode(PORT_LED_GREEN, OUTPUT);
	
	// Turned off by default
	digitalWrite(PORT_LED_RED, LOW);
	digitalWrite(PORT_LED_GREEN, LOW);

	red_led_state = green_led_state = 0;
}

void hal_led_light(int player, int on) 
{
	int port, level;
	int *state;
	if (player == PLAYER_RED) {
		port = PORT_LED_RED;
		state = &red_led_state;
	} else {
		port = PORT_LED_GREEN;
		state = &green_led_state;
	}
	if (on) {
		level = HIGH;
		*state = 1;
	} else {
		level = LOW;		
		*state = 0;
	}
	digitalWrite(port, level);		
}

void hal_led_toggle(int player)
{
	if (player == PLAYER_RED) {
		hal_led_light(player, !red_led_state);
	} else {
		hal_led_light(player, !green_led_state);
	}
}

#endif
