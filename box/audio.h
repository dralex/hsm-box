/* -----------------------------------------------------------------------------
 * The Game Timer Box
 * -----------------------------------------------------------------------------
 * 
 * The HAL: Audio functions
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

#ifndef BOX_AUDIO_HEADER
#define BOX_AUDIO_HEADER

#include "constants.h"
#include "ports.h"

#define NOTES           8
#define NOTE_LENGTH     500
#define START_DELAY     100
#define MELODY_DELAY    100
#define SILENCE_DELAY   100 * 2

int notes_tab[] = {1911, 1702, 1516, 1431, 1275, 1136, 1012}; // BASS C-B

#define REST   -1
#define NOTE_C 0
#define NOTE_D 1
#define NOTE_E 2
#define NOTE_F 3
#define NOTE_G 4
#define NOTE_A 5
#define NOTE_B 6

//int8_t start_melody[] = {NOTE_C, NOTE_G, NOTE_E, NOTE_F, NOTE_F, NOTE_E};
//int8_t start_melody[] = {NOTE_E, NOTE_F, NOTE_F, NOTE_E, NOTE_G, NOTE_C};
//int8_t start_melody[] = {NOTE_C, NOTE_D, NOTE_F, NOTE_D, NOTE_A, REST, NOTE_A, NOTE_G, REST,
//						 NOTE_C, NOTE_D, NOTE_F, NOTE_D, NOTE_G, REST, NOTE_G, NOTE_G, NOTE_F};
//int8_t start_melody[] = {NOTE_C, NOTE_E, NOTE_G, NOTE_G, REST, NOTE_G, NOTE_F, NOTE_G, NOTE_A, NOTE_G};
int start_melody[] = {NOTE_C, 100, NOTE_G, 100, NOTE_A, 100,
					  NOTE_G, 100, NOTE_A, 100, NOTE_B, 100}; 
size_t start_melody_size = sizeof(start_melody) / sizeof(int);

int final_melody[] = {NOTE_E, 100, NOTE_F, 100, NOTE_F, 100,
					  NOTE_E, 100, NOTE_G, 100, NOTE_C, 100};
size_t final_melody_size = sizeof(final_melody) / sizeof(int);

void __audio_play_note(uint8_t note_index, size_t length)
{
	for(size_t i = 0; i < length; i++) {
		digitalWrite(PORT_AUDIO, HIGH);
		delayMicroseconds(notes_tab[note_index]);
		digitalWrite(PORT_AUDIO, LOW);
		delayMicroseconds(notes_tab[note_index]);
	}
}

void __audio_play_melody(int* melody, size_t melody_size)
{
	for(size_t i = 0; i < melody_size; i += 2) {
		int note_index = melody[i];
		int note_length = melody[i + 1];
		if (note_index >= 0 && note_index < NOTES) {
			__audio_play_note((uint8_t)note_index, (size_t)note_length);
		} else {
			delay(note_length);
		}
		delay(note_length);
	}
}

void __audio_power_on()
{
	digitalWrite(PORT_AUDIO_POWER, HIGH);
	delay(START_DELAY);
}

void __audio_power_off()
{
	digitalWrite(PORT_AUDIO_POWER, LOW);
}

void hal_audio_init()
{
	pinMode(PORT_AUDIO, OUTPUT);
	pinMode(PORT_AUDIO_POWER, OUTPUT);
	digitalWrite(PORT_AUDIO, LOW);	
}

void hal_audio_play_start()
{
	__audio_power_on();
	__audio_play_melody(start_melody, start_melody_size);
	__audio_power_off();
}

void hal_audio_play_final()
{
	__audio_power_on();
	__audio_play_melody(final_melody, final_melody_size);
	__audio_power_off();
}

#endif
