/* -----------------------------------------------------------------------------
 * The Game Timer Box
 * -----------------------------------------------------------------------------
 * 
 * The HAL: Display functions
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

#ifndef BOX_DISPLAY_HEADER
#define BOX_DISPLAY_HEADER

#include "constants.h"
#include "errors.h"

#define LCD_DEVICE       0x27
#define LCD_ROWS         16
#define LCD_COLUMNS      2

LCD_I2C lcd(LCD_DEVICE, LCD_ROWS, LCD_COLUMNS);

void time_to_buffer(char* buffer, int sec)
{
	int min = sec / 60;
	sec = sec % 60;
	snprintf(buffer, 6, "%02d:%02d", min, sec);
}

void hal_display_init()
{
	lcd.begin();
	lcd.backlight();
	lcd.clear();
	lcd.print("Game Timer Box");
}

void hal_display_setup(int t, int clear)
{
	char buffer[6];
	if (clear) {
		lcd.clear();
		lcd.setCursor(0, 1);
		lcd.print("Press 'OK'");
	}
	time_to_buffer(buffer, t);
	lcd.setCursor(0, 0);
	lcd.print("SETUP  ");
	lcd.print(buffer);
	lcd.print(" min");
}

void hal_display_game_start()
{
	lcd.clear();
	lcd.print("Starting MATCH..");
}

void hal_display_game(uint32_t t, int red, int green, int player, int clear)
{
	char buffer[6];
	if (clear) {
		lcd.clear();
	}
	time_to_buffer(buffer, t);
	lcd.setCursor(0, 0);
	if (player == PLAYER_RED) {
		lcd.print("RED  ");
		lcd.print(buffer);
		lcd.print(" Green");
	} else if (player == PLAYER_GREEN) {
		lcd.print("Red  ");
		lcd.print(buffer);
		lcd.print(" GREEN");
	} else {
		lcd.print("Red  ");
		lcd.print(buffer);
		lcd.print(" Green");
	}
	DEBUG_("Display time: ");
	DEBUG(buffer);
	lcd.setCursor(0, 1);
	time_to_buffer(buffer, red);
	lcd.print(buffer);
	lcd.setCursor(11, 1);
	time_to_buffer(buffer, green);
	lcd.print(buffer);
}

void hal_display_final_start()
{
	lcd.clear();
	lcd.print("MATCH is OVER!!!");
}

void hal_display_final(int winner, int red, int green)
{
	char buffer[6];
	lcd.clear();
	lcd.setCursor(0, 0);
	if (winner == PLAYER_RED) {
		lcd.print("RED WON!!!");
	} else if (winner == PLAYER_GREEN) {
		lcd.print("GREEN WON!!!");
	} else {
		lcd.print("TIE!!!");
	}
	lcd.setCursor(0, 1);
	lcd.print("R");
	time_to_buffer(buffer, red);
	lcd.print(buffer);
	lcd.print(" vs G");
	time_to_buffer(buffer, green);
	lcd.print(buffer);
}

void hal_display_error(const char* s)
{
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Error:");
	lcd.setCursor(0, 1);
	lcd.print(s);
}

#endif
