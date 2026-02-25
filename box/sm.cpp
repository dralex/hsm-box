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
	stGame,
	stGameWait,
	stGameRed,
	stGameGreen,
	stResults,
	stTie,
	stRedWon,
	stGreenWon
};

typedef struct _StateHierarchy {
	SMState state;
	SMState parent;
} StateHierarchy;

#define NO_PARENT ((SMState)-1)
#define MAX_STATE_HIERARCHY_DEPTH 10

StateHierarchy state_hierarchy[] = {
	{stError,     NO_PARENT},
	{stSetup,     NO_PARENT},
	{stGame,      NO_PARENT},
	{stGameWait,  stGame},
	{stGameRed,   stGame},
	{stGameGreen, stGame},
	{stResults,   NO_PARENT},
	{stTie,       stResults},
	{stRedWon,    stResults},
	{stGreenWon,  stResults}
};
size_t state_hierarchy_count = sizeof(state_hierarchy) / sizeof(StateHierarchy);

/* -----------------------------------------------------------------------------
 * The state machine events
 * ----------------------------------------------------------------------------- */

enum Event {
	evTick =        0,
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

int time_mode;
int red, green;
int current_player;
int winner;

int on_error(void* arg)
{
	const char* s = (const char*)arg;
	DEBUG_("On Error");
	if (s != NULL) {
		hal_display_error(s);
		DEBUG_(": ");
		DEBUG(s);
	} else {
		DEBUG("");
	}
	hal_led_light(PLAYER_RED, 1);
	hal_led_light(PLAYER_GREEN, 0);
	return 0;
}

int on_setup_init(void* arg)
{
	DEBUG("On setup init");
	hal_led_light(PLAYER_RED, 0);
	hal_led_light(PLAYER_GREEN, 0);	
	hal_display_setup(time_mode, 1);
	return 0;
}

int on_game_init(void* arg)
{
	DEBUG("On game init");
	return 0;
}

int on_game_wait_init(void* arg)
{
	DEBUG("On game wait init");
	DEBUG_("Starting game on ");
	DEBUG_(time_mode);
	DEBUG(" sec.");
	hal_display_game_start();
	hal_audio_play_start();
	red = green = 0;
	current_player = -1;
	hal_timer_start(time_mode);
	hal_display_game(hal_timer_left_sec(), red, green, current_player, 1);
	return 0;
}

int on_results_init(void* arg)
{
	DEBUG("On result init");
	hal_display_final_start();
	hal_audio_play_final();
	winner = -1;
	hal_display_final(winner, red, green);
	return 0;
}

int on_red_won_init(void* arg)
{
	DEBUG("On red won init");
	winner = PLAYER_RED;
	hal_led_light(PLAYER_RED, 1);
	hal_led_light(PLAYER_GREEN, 0);
	hal_display_final(winner, red, green);
	return 0;
}

int on_green_won_init(void* arg)
{
	DEBUG("On green won init");
	winner = PLAYER_GREEN;
	hal_led_light(PLAYER_RED, 0);
	hal_led_light(PLAYER_GREEN, 1);
	hal_display_final(winner, red, green);
 	return 0;
}

int on_tie_init(void* arg)
{
	DEBUG("On tie init");
	return 0;
}

int on_error_sec(void* arg)
{
	DEBUG("On error sec");	
	hal_led_toggle(PLAYER_RED);
	hal_led_toggle(PLAYER_GREEN);
	return 0;
}

int on_setup_left(void* arg)
{
	DEBUG("On setup left");
	if (time_mode <= 3 * 60) {
		time_mode = 60;
	} else if (time_mode == 5 * 60) {
		time_mode = 3 * 60;
	} else {
		time_mode -= 5 * 60;
	}
	hal_display_setup(time_mode, 0);
	return 0;
}

int on_setup_right(void* arg)
{
	DEBUG("On setup right");
	if (time_mode == 60) {
		time_mode = 3 * 60;
	} else if (time_mode == 3 * 60) {
		time_mode = 5 * 60;
	} else if (time_mode <= 3600 - 5 * 60 - 1) {
		time_mode += 5 * 60;
	}
	hal_display_setup(time_mode, 0);
	return 0;	
}

int on_game_sec(void* arg)
{
	DEBUG("On game sec");
	hal_display_game(hal_timer_left_sec(), red, green, current_player, 0);	
	return 0;	
}

int on_game_red(void* arg)
{
	DEBUG("On game red");	
	current_player = PLAYER_RED;
	hal_led_light(PLAYER_RED, 1);
	hal_led_light(PLAYER_GREEN, 0);	
	hal_display_game(hal_timer_left_sec(), red, green, current_player, 0);	
	return 0;
}

int on_game_green(void* arg)
{
	DEBUG("On game green");
	current_player = PLAYER_GREEN;
	hal_led_light(PLAYER_RED, 0);
	hal_led_light(PLAYER_GREEN, 1);	
	hal_display_game(hal_timer_left_sec(), red, green, current_player, 0);	
	return 0;
}

int on_game_red_sec(void* arg)
{
	DEBUG("On game red sec");
	red += 1;
	hal_display_game(hal_timer_left_sec(), red, green, current_player, 0);	
	return 0;	
}

int on_game_green_sec(void* arg)
{
	DEBUG("On game green sec");
	green += 1;
	hal_display_game(hal_timer_left_sec(), red, green, current_player, 0);	
	return 0;
}

int on_game_timer(void* arg)
{
	DEBUG("On game timer");
	return 0;
}

int on_restart(void* arg)
{
	DEBUG("On restart");
	return 0;
}

int on_end_timer(void* arg)
{
	DEBUG("On end timer");
	if (winner >= PLAYER_RED) {
		hal_led_toggle(winner);
	}
	return 0;
}

/* -----------------------------------------------------------------------------
 * Guard conditions
 * ----------------------------------------------------------------------------- */

bool guard_red_won()
{
	return red > green;
}

bool guard_green_won()
{
	return red < green;
}

/* -----------------------------------------------------------------------------
 * Type definitions for function pointers
 * ----------------------------------------------------------------------------- */

typedef int (*Handler)(void* arg);
typedef bool (*GuardHandler)();

/* -----------------------------------------------------------------------------
 * Hierarchical State Machine Support - Data Structures
 * ----------------------------------------------------------------------------- */

typedef struct _Entry {
	SMState state;
	Handler handler;
} Entry;

typedef struct _Exit {
	SMState state;
	Handler handler;
} Exit;

typedef struct _InitialTransition {
	SMState source_state;      /* the parent state */
	GuardHandler guard;        /* Guard  (NULL if no guard or [else])*/
	SMState target_state;      /* Target initial state */
	Handler handler;           /* Behavior on transition (NULL if is not used) */
} InitialTransition;

typedef struct _Transition {
	SMState source_state;
	Event event;
	GuardHandler guard;        /* Guard (NULL if no guard) */
	SMState target_state;
	Handler handler;           /* Behavior on transition (NULL if is not used) */
} Transition;

/* -----------------------------------------------------------------------------
 * Hierarchy helper functions
 * ----------------------------------------------------------------------------- */

SMState get_parent_state(SMState s)
{
	for (size_t i = 0; i < state_hierarchy_count; i++) {
		if (state_hierarchy[i].state == s) {
			return state_hierarchy[i].parent;
		}
	}
	return NO_PARENT;
}

bool is_child_of(SMState child, SMState parent)
{
	SMState current = child;
	while (current != NO_PARENT) {
		current = get_parent_state(current);
		if (current == parent) {
			return true;
		}
	}
	return false;
}

bool is_composite_state(SMState s)
{
	for (size_t i = 0; i < state_hierarchy_count; i++) {
		if (state_hierarchy[i].parent == s) {
			return true;
		}
	}
	return false;
}

/* -----------------------------------------------------------------------------
 * State machine data
 * ----------------------------------------------------------------------------- */

SMState the_state;

/* -----------------------------------------------------------------------------
 * The state machine processing tables & functions
 * ----------------------------------------------------------------------------- */

/* The entry behaviors */
Entry entries[] = {
	{stError,    &on_error},
	{stSetup,    &on_setup_init},
	{stGameWait, &on_game_wait_init},
	{stResults,  &on_results_init},
	{stTie,      &on_tie_init},
	{stRedWon,   &on_red_won_init},
	{stGreenWon, &on_green_won_init},	
};
size_t entries_count = sizeof(entries) / sizeof(Entry);

/* The exit behaviors */
Exit exits[] = {
};
size_t exits_count = sizeof(exits) / sizeof(Exit);

InitialTransition initial_transitions[] = {
	{stGame,    NULL,             stGameWait, NULL},
	{stResults, &guard_red_won,   stRedWon,   NULL},
	{stResults, &guard_green_won, stGreenWon, NULL},
	{stResults, NULL /* else */,  stTie,      NULL}
};
size_t initial_transitions_count = sizeof(initial_transitions) / sizeof(InitialTransition);

/* The transition graph */
Transition transitions[] = {
	{stError,     evTimer1Sec,   NULL, stError,     &on_error_sec},
	
	/* Setup state */
	{stSetup,     evButtonLeft,  NULL, stSetup,     &on_setup_left},
	{stSetup,     evButtonRight, NULL, stSetup,     &on_setup_right},
	{stSetup,     evButtonOK,    NULL, stGame,      NULL},
	
	/* Game state */
	{stGame,      evTimer,       NULL, stResults,   &on_game_timer},
	
	/* GameWait state */
	{stGameWait,  evTimer1Sec,   NULL, stGameWait,  &on_game_sec},
	{stGameWait,  evButtonRed,   NULL, stGameRed,   &on_game_red},
	{stGameWait,  evButtonGreen, NULL, stGameGreen, &on_game_green},
	
	/* GameRed state */
	{stGameRed,   evTimer1Sec,   NULL, stGameRed,   &on_game_red_sec},
	{stGameRed,   evButtonGreen, NULL, stGameGreen, &on_game_green},
	
	/* GameGreen state */
	{stGameGreen, evTimer1Sec,   NULL, stGameGreen, &on_game_green_sec},
	{stGameGreen, evButtonRed,   NULL, stGameRed,   &on_game_red},
	
	/* Results state */
	{stResults,   evButtonOK,    NULL, stSetup,     &on_restart},
	{stResults,   evTimer1Sec,   NULL, stResults,   &on_end_timer},
};
size_t transitions_count = sizeof(transitions) / sizeof(Transition);

/* -----------------------------------------------------------------------------
 * Entry/Exit action execution
 * ----------------------------------------------------------------------------- */

/* Execute entry actions for a state */
int execute_entry_actions(SMState s, void* arg)
{
 	for (size_t i = 0; i < entries_count; i++) {
		if (entries[i].state == s && entries[i].handler) {
			return (*(entries[i].handler))(arg);
		}
	}
	return 0;
}

/* Execute exit actions for a state */
int execute_exit_actions(SMState s, void* arg)
{
	for (size_t i = 0; i < exits_count; i++) {
		if (exits[i].state == s && exits[i].handler) {
			return (*(exits[i].handler))(arg);
		}
	}
	return 0;
}

/* -----------------------------------------------------------------------------
 * Error handling: works as global exception
 * ----------------------------------------------------------------------------- */

void set_error_state(char* format, ...)
{
	char error_string[LCD_ROWS + 1];
	va_list args;
	va_start(args, format);
	vsnprintf(error_string, sizeof(error_string) - 1, format, args);
	va_end(args);
	error_string[LCD_ROWS] = 0;

 	for (size_t i = 0; i < entries_count; i++) {
		if (entries[i].state == stError && entries[i].handler) {
			(*(entries[i].handler))((void*)error_string);
		}
	}	
	the_state = stError;
}

/* -----------------------------------------------------------------------------
 * Initial transition resolution
 * ----------------------------------------------------------------------------- */

/* Get initial transition target for a composite state
 * Evaluates guards and returns the first matching target or [else] target */
bool get_initial_target(SMState composite_state, SMState* target, Handler* handler)
{
	SMState else_target = NO_PARENT;
	
	/* First pass: evaluate all guarded initial transitions */
	for (size_t i = 0; i < initial_transitions_count; i++) {
		if (initial_transitions[i].source_state == composite_state) {
			if (initial_transitions[i].guard != NULL) {
				if (initial_transitions[i].guard()) {
					DEBUG_("Initial transition guard matched, target: ");
					DEBUG(int(initial_transitions[i].target_state));
					*target = initial_transitions[i].target_state;
					*handler = initial_transitions[i].handler;
					return true;
				}
			} else {
				/* Remember [else] or unconditional branch */
				*handler = initial_transitions[i].handler;
				else_target = initial_transitions[i].target_state;
 			}
 		}
 	}
	
	/* Second pass: use [else] or unconditional branch if no guard matched */
	if (else_target != NO_PARENT) {
		DEBUG_("Initial transition using [else] branch, target: ");
		DEBUG(int(else_target));
		*target = else_target;
		return true;
	}

	return false;
}

void transition_enter_state(SMState s_to, SMState s_from, void* arg, int skip_parent);

/* Process initial transitions recursively */
void process_initial_transitions(SMState s, void* arg)
{
	SMState target;
	Handler handler = NULL;
	
	if (get_initial_target(s, &target, &handler)) {

		DEBUG_("Processing initial transition from ");
		DEBUG_(int(s));
		DEBUG_(" to ");
		DEBUG(int(target));

		int result = 0;
		
		if (handler) {
			result = (*handler)(NULL);
		}

		if (result) {
			DEBUG("Initial transition behavior returned error");
			set_error_state("Error: %d", result);
			return ;
		}

		transition_enter_state(target, s, arg, 1);
	}
}

/* -----------------------------------------------------------------------------
 * State transition function
 * ----------------------------------------------------------------------------- */

// Build path from source to LCA (Least Common Ancestor)
void build_exit_path(SMState source, SMState target, SMState* path, size_t* path_len)
{
	*path_len = 0;
	SMState current = source;
	
	/* Build exit path until we reach a common ancestor */
	while (current != NO_PARENT) {
		/* Check if target is a child of current */
		if (current == target || is_child_of(target, current)) {
			break;
		}
		path[(*path_len)++] = current;
		current = get_parent_state(current);
	}
}

void transition_exit_state(SMState s_from, SMState s_to, void* arg)
{
	DEBUG_("State transition from: ");
	DEBUG(int(s_from));
	
	SMState source = s_from;
	SMState target = s_to;
	SMState path[MAX_STATE_HIERARCHY_DEPTH];
	size_t path_len = 0;

	build_exit_path(source, target, path, &path_len);
	
	/* Execute exit actions (from innermost to outermost) */
	for (size_t i = 0; i < path_len; i++) {
		DEBUG_("Exiting state: ");
		DEBUG(int(path[i]));
		int result = execute_exit_actions(path[i], arg);
		if (result) {
			DEBUG("Exit action returned error");
			set_error_state("Error: %d", result);
			return;
		}
	}
}

/* Build path from LCA to target */
void build_entry_path(SMState source, SMState target, SMState* path, size_t* path_len)
{
	*path_len = 0;
	SMState temp_path[MAX_STATE_HIERARCHY_DEPTH];
	size_t temp_len = 0;
	SMState current = target;
	
	/* Build path backwards from target to LCA */
	while (current != NO_PARENT) {
		temp_path[temp_len++] = current;
		/* Check if source is a child of current or current is the source */
		if (current == source || is_child_of(source, current)) {
			break;
		}
		current = get_parent_state(current);
	}
	
	/* Reverse the path */
	for (size_t i = 0; i < temp_len; i++) {
		path[i] = temp_path[temp_len - 1 - i];
	}
	*path_len = temp_len;
}

void transition_enter_state(SMState s_to, SMState s_from, void* arg, int skip_parent)
{
	DEBUG_("State transition to: ");
	DEBUG(int(s_to));

	SMState source = s_from;
	SMState target = s_to;
	SMState path[MAX_STATE_HIERARCHY_DEPTH];
	size_t path_len = 0;
	
	build_entry_path(source, target, path, &path_len);
	
	/* Execute entry actions (from outermost to innermost) */
	for (size_t i = 0; i < path_len; i++) {
		if (skip_parent) {
			if (path[i] != s_to) {
				continue;
			} else {
				skip_parent = 0;
			}
		}
		DEBUG_("Entering state: ");
		DEBUG(int(path[i]));
		int result = execute_entry_actions(path[i], arg);
		if (result) {
			set_error_state("Error: %d", result);
			return;
		}
	}
	
	the_state = s_to;

	if (is_composite_state(the_state)) {
		process_initial_transitions(the_state, arg);
	}
}

/* -----------------------------------------------------------------------------
 * Event processing function
 * ----------------------------------------------------------------------------- */

void process_events(int events)
{
	/* Try to find transition from current state or its ancestors */
	SMState current = the_state;
	
	while (current != NO_PARENT) {
		/* Check all transitions with matching source and event */
		for (size_t i = 0; i < transitions_count; i++) {
			if (transitions[i].source_state == current && (transitions[i].event & events)) {
				/* Check guard condition if present */
				if (transitions[i].guard != NULL) {
					if (!transitions[i].guard()) {
						continue; /* Guard failed, try next transition */
					}
				}
				
				/* Guard passed (or no guard) - take this transition */
				SMState new_state = transitions[i].target_state;

				if (new_state != current) {
					transition_exit_state(current, new_state, NULL);
				}

				Handler handler = transitions[i].handler;
				int result = 0;
				
				if (handler) {
					result = (*handler)(NULL);
				}				
				if (result) {
					set_error_state("Error: %d", result);
					return ;
				}

				if (new_state != current) {
					transition_enter_state(new_state, current, NULL, 0);
					return;
				}
				
				/* Transition taken (even if to same state), don't check other transitions */
				return;
			}
		}
		
		/* Move up the hierarchy to check parent state transitions */
		current = get_parent_state(current);
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
	
	transition_enter_state(stSetup, stSetup, NULL, 0);
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
