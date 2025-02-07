/*****************************************************************************
**
** blueberryMSX
** https://github.com/pokebyte/blueberryMSX
**
** An MSX Emulator for Raspberry Pi based on blueMSX
**
** Copyright (C) 2003-2006 Daniel Vik
** Copyright (C) 2014 Akop Karapetyan
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
******************************************************************************
*/

#ifndef PI_SHORTCUTS_H
#define PI_SHORTCUTS_H

#define KBD_LSHIFT 0x01
#define KBD_RSHIFT 0x02
#define KBD_LCTRL  0x04
#define KBD_RCTRL  0x08
#define KBD_LALT   0x10
#define KBD_RALT   0x20
#define KBD_LWIN   0x40
#define KBD_RWIN   0x80

#define HOTKEY_TYPE_NONE     0
#define HOTKEY_TYPE_KEYBOARD 1
#define HOTKEY_TYPE_JOYSTICK 2


typedef struct Shortcuts Shortcuts;

void shortcutsSetDirectory(char* directory);

Shortcuts* shortcutsCreate();
void shortcutsDestroy(Shortcuts* shortcuts);
//void shortcutCheckDown(Shortcuts* s, int type, int mods, int keySym);
//void shortcutCheckUp(Shortcuts* s, int type, int mods, int keySym);
void shortcutCheckDown(Shortcuts* s, int type, int mods, long keySym);
void shortcutCheckUp(Shortcuts* s, int type, int mods, long keySym);

#endif
