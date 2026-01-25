/* -----------------------------------------------------------------------------
 * The Game Timer Box
 * -----------------------------------------------------------------------------
 * 
 * The game timer box project for Arduino - main functions
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

#include <Arduino.h>

#include "constants.h"
#include "ports.h"
#include "light.h"
#include "display.h"
#include "buttons.h"
#include "audio.h"
#include "timer.h"
#include "errors.h"

/* -----------------------------------------------------------------------------
 * The state machine states 
 * ----------------------------------------------------------------------------- */

enum SMState {
	stError,
	stSetup,
	stGameWait,
	stGameRed,
	stGameGreen,
	stGameEnd
};

void change_state(SMState s);

/* -----------------------------------------------------------------------------
 * The state machine events
 * ----------------------------------------------------------------------------- */

enum Event {
	evButtonRed =   BUTTON_FLAG_RED,
	evButtonGreen = BUTTON_FLAG_GREEN,  
	evButtonOK =    BUTTON_FLAG_OK,
	evButtonLeft =  BUTTON_FLAG_LEFT,
	evButtonRight = BUTTON_FLAG_RIGHT,
	evTimer =       TIMER_FLAG_TIMEOUT,
	evTimer1Sec =   TIMER_FLAG_1SEC
};

/* -----------------------------------------------------------------------------
 * The state handlers - THE MAIN LOGIC IS HERE
 * ----------------------------------------------------------------------------- */

void on_error(void* arg)
{
	const char* s = (const char*)arg;
	if (s != NULL) {
		hal_display_error(s);
	}
	hal_led_light(PLAYER_RED, 1);
	hal_led_light(PLAYER_GREEN, 0);
}

int time_mode;

void on_setup_init(void* arg)
{
	hal_led_light(PLAYER_RED, 0);
	hal_led_light(PLAYER_GREEN, 0);	
	hal_display_setup(time_mode, 1);
}

int red, green;
int current_player;

void on_game_wait_init(void* arg)
{
	DEBUG_("Starting game on ");
	DEBUG_(time_mode);
	DEBUG(" sec.");
	red = green = 0;
	current_player = -1;
	hal_timer_start(time_mode);
	hal_display_game(hal_timer_left_sec(), red, green, current_player, 1);
}

int winner;

void on_game_end_init(void* arg)
{
	winner = -1;
	if (red > green) {
		winner = PLAYER_RED;
		hal_led_light(PLAYER_RED, 1);
		hal_led_light(PLAYER_GREEN, 0);
	} else if (green > red) {
		winner = PLAYER_GREEN;
		hal_led_light(PLAYER_RED, 0);
		hal_led_light(PLAYER_GREEN, 1);
	}	
	hal_display_final(winner, red, green);
}

SMState on_error_sec(void* arg)
{
	hal_led_toggle(PLAYER_RED);
	hal_led_toggle(PLAYER_GREEN);
	return stError;
}

SMState on_setup_left(void* arg)
{
	if (time_mode <= 3 * 60) {
		time_mode = 60;
	} else if (time_mode == 5 * 60) {
		time_mode = 3 * 60;
	} else {
		time_mode -= 5 * 60;
	}
	hal_display_setup(time_mode, 0);
	return stSetup;
}

SMState on_setup_right(void* arg)
{
	if (time_mode == 60) {
		time_mode = 3 * 60;
	} elseif (time_mode == 3 * 60) {
		time_mode = 5 * 60;
	} else if (time_mode <= 3600 - 5 * 60 - 1) {
		time_mode += 5 * 60;
	}
	hal_display_setup(time_mode, 0);
	return stSetup;	
}

SMState on_setup_ok(void* arg)
{
	return stGameWait;
}

SMState on_game_sec(void* arg)
{
	hal_display_game(hal_timer_left_sec(), red, green, current_player, 0);	
	return stGameWait;	
}

SMState on_game_red(void* arg)
{
	current_player = PLAYER_RED;
	hal_led_light(PLAYER_RED, 1);
	hal_led_light(PLAYER_GREEN, 0);	
	hal_display_game(hal_timer_left_sec(), red, green, current_player, 0);	
	return stGameRed;
}

SMState on_game_green(void* arg)
{
	current_player = PLAYER_GREEN;
	hal_led_light(PLAYER_RED, 0);
	hal_led_light(PLAYER_GREEN, 1);	
	hal_display_game(hal_timer_left_sec(), red, green, current_player, 0);	
	return stGameGreen;
}

SMState on_game_red_sec(void* arg)
{
	red += 1;
	hal_display_game(hal_timer_left_sec(), red, green, current_player, 0);	
	return stGameRed;	
}

SMState on_game_green_sec(void* arg)
{
	green += 1;
	hal_display_game(hal_timer_left_sec(), red, green, current_player, 0);	
	return stGameGreen;
}

SMState on_game_timer(void* arg)
{
	DEBUG("Timer event!");
	return stGameEnd;
}

SMState on_restart(void* arg)
{
	DEBUG("Restart on OK event!");
	return stSetup;
}

SMState on_end_timer(void* arg)
{
	if (winner >= PLAYER_RED) {
		hal_led_toggle(winner);
	}
	return stGameEnd;
}

/* -----------------------------------------------------------------------------
 * The state machine processing structs & functions
 * ----------------------------------------------------------------------------- */

typedef void (*EntryHandler)(void* arg);
typedef SMState (*TransHandler)(void* arg);

typedef struct _Entry {
	SMState      state;
	EntryHandler handler;
} Entry;

Entry entries[] = {
	{stError,    &on_error},
	{stSetup,    &on_setup_init},
	{stGameWait, &on_game_wait_init},
	{stGameEnd,  &on_game_end_init}
};
size_t entries_count = sizeof(entries) / sizeof(Entry);

typedef struct _Transition {
	SMState      state;
	Event        event;
	TransHandler handler;
} Transition;

/* The transition graph */
Transition transitions[] = {
	{stError,     evTimer1Sec,   &on_error_sec},
	{stSetup,     evButtonLeft,  &on_setup_left},
	{stSetup,     evButtonRight, &on_setup_right},
	{stSetup,     evButtonOK,    &on_setup_ok},
	{stGameWait,  evTimer,       &on_game_timer},
	{stGameWait,  evTimer1Sec,   &on_game_sec},
	{stGameWait,  evButtonRed,   &on_game_red},
	{stGameWait,  evButtonGreen, &on_game_green},
	{stGameRed,   evTimer,       &on_game_timer},
	{stGameRed,   evTimer1Sec,   &on_game_red_sec},
	{stGameRed,   evButtonGreen, &on_game_green},	
	{stGameGreen, evTimer,       &on_game_timer},
	{stGameGreen, evTimer1Sec,   &on_game_green_sec},	
	{stGameGreen, evButtonRed,   &on_game_red},	
	{stGameEnd,   evButtonOK,    &on_restart},
	{stGameEnd,   evTimer1Sec,   &on_end_timer},
};
size_t transitions_count = sizeof(transitions) / sizeof(Transition);

SMState the_state;

void set_state(SMState s, void* arg = NULL)
{
	DEBUG_("New state: ");
	DEBUG(int(s));
	the_state = s;
	for (size_t i = 0; i < entries_count; i++) {
		if (entries[i].state == s) {
			(*(entries[i].handler))(arg);
		}
	}
}

char error_string[17];

void set_error(char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(error_string, sizeof(error_string), format, args);
	va_end(args);
}

void process_events(int events)
{
	for (size_t i = 0; i < transitions_count; i++) {
		if (transitions[i].state == the_state && (transitions[i].event & events)) {
			SMState new_state = (*(transitions[i].handler))(NULL);
			if (new_state == stError) {
				set_state(stError, error_string);
				return ;
			} else if (new_state != the_state) {
				set_state(new_state);
				return ;
			}
		}
	}
}

void setup()
{
	DEBUG_BEGIN();
	DEBUG("Game Box Started");
	
	hal_led_init();
	hal_display_init();
	hal_audio_init();
	hal_buttons_init();
	hal_timer_init();

	time_mode = 5 * 60;
	
	set_state(stSetup);
}

void loop()
{
	while (1) {
		int pressed = hal_buttons_check();
		int timeout = hal_timer_check();
		process_events(pressed | timeout);
		delay(100);
	}
}
