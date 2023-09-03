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
/*      ESC    F1    F2    F3    F4   XXX    F5    F6    F7    F8   XXX    F9   F10   F11   F12   PRN SCRLK PAUSE   XXX   XXX   XXX   XXX     */
{       96,   97,   98,   99,  104,   -1,  105,  106,  112,  113,   -1,  114,   67,   68,   69,  102,  103,  107,   -1,   -1,   -1,   -1},

/*        `     1     2     3     4     5     6     7     8     9     0     -   +/ˆ   XXX BKSPC INSRT  HOME  PGUP NUMLK    #/    #*    #-     */
{        0,    1,    8,    9,   16,   17,   24,   25,   32,   33,   40,   41,   48,   -1,   49,   56,   57,   64,   -1,   -1,   -1,   -1},

/*      TAB     Q     W     E     R     T     Y     U     I     O     P     [   ]/¨   XXX ENTER DELTE   END  PGDN    #7    #8    #9    #+     */
{        2,    3,   10,   11,   18,   19,   26,   27,   34,   35,   42,   43,   50,   -1,   52,   58,   59,   66,   -1,   -1,   -1,   -1},

/*    CAPLK     A     S     D     F     G     H     J     K     L     ;     '     \   XXX   XXX   XXX   XXX   XXX    #4    #5    #6   XXX     */
{        4,    5,   12,   13,   20,   21,   28,   29,   36,   37,   44,   45,   89,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},

/*    LSHFT     <     Z     X     C     V     B     N     M     ,     .     /   XXX   XXX RSHFT   XXX    UP   XXX    #1    #2    #3 #ENTR     */
{        6,  100,    7,   14,   15,   22,   23,   30,   31,   38,   39,   46,   -1,   -1,   47,   -1,   61,   -1,   -1,   -1,   -1,   -1},

/*    LCTRL  LWIN  LALT   XXX   XXX   XXX SPACE   XXX   XXX   XXX  RALT  RWIN   FNC   XXX RCTRL  LEFT  DOWN RIGHT    #0   XXX    #.   XXX     */
{       91,   90,   92,   -1,   -1,   -1,   93,   -1,   -1,   -1,   94,   95,   60,   -1,   54,   63,   62,   70,   -1,   -1,   -1,   -1},

{	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1},
};
