/*
 * This file is part of libcmmk.
 *
 * libcmmk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * libcmmk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libcmmk.  If not, see <http://www.gnu.org/licenses/>.
 */
static keyboard_layout layout_iso_pro_s = {
	/*
	 ESC   F1    F2    F3    F4    XXX   F5    F6    F7    F8    XXX   F9    F10   F11   F12   PRN   SCL   PAU   XXX   XXX   XXX   XXX */
	{0,    8,    16,   24,   32,   -1,   40,   48,   56,   64,   72,   80,   88,   96,   -1,   104,  -1,   14,   -1,   -1,   -1,   -1},

	/*
	 `     1     2     3     4     5     6     7     8     9     0     -     =     XXX   BCK   INS   HOM   PUP   XXX   XXX   XXX   XXX */
	{1,    9,    17,   25,   33,   41,   49,   57,   65,   73,   81,   89,   97,   -1,   113,  114,  22,   30,   -1,   -1,   -1,   -1},

	/*
	 TAB   Q     W     E     R     T     Y     U     I     O     P     [     ]     XXX   XXX   DEL   END   PDN   XXX   XXX   XXX   XXX */
	{2,    10,   18,   26,   34,   42,   50,   58,   66,   74,   82,   90,   98,   -1,   -1,   115,   70,  78,   -1,   -1,   -1,   -1},

	/*
	 CAP   A     S     D     F     G     H     J     K     L     ;     '     #     XXX   ENT   XXX   XXX   XXX   XXX   XXX   XXX   XXX */
	{3,    11,   19,   27,   35,   43,   51,   59,   67,   75,   83,   91,   99,   -1,   107,  -1,   -1,   -1,   -1,   -1,   -1,   -1},

	/*
	 LSHFT \     Z     X     C     V     B     N     M     ,     .     /     XXX   XXX   RSHFT XXX   UP    XXX   XXX   XXX   XXX   XXX */
	{4,    12,   20,   28,   36,   44,   52,   60,   68,   76,   84,   92,   -1,   -1,   108,  -1,   116,  -1,   -1,   -1,   -1,   -1}, // after / was 1 BAD

	/*
	 LCTRL LWIN  LALT  XXX   XXX   XXX   SPACE XXX   XXX   XXX   RALT  RWIN  FN    XXX   RCTRL LEFT  DOWN  RIGHT XXX   XXX   XXX   XXX */
	{5,    13,   21,   -1,   -1,   -1,   53,   -1,   -1,   -1,   77,   85,   93,   -1,   101,  109,  117,  118,  -1,   -1,   -1,   -1},

	{-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
};