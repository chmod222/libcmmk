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
#include <libcmmk/libcmmk.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h> /* usleep() */
#include <signal.h>
#include <time.h> /* do we need this? */
#endif

#include <string.h> /* memset() */
#include <stdio.h>
#include <stdlib.h>

int g_stop = 0;

#ifdef WIN32
BOOL WINAPI ctrl_handler ( DWORD ctrlType )
{
    /* We only care about ctrl+c events. */
    if ( ctrlType == CTRL_C_EVENT ) {
        g_stop = 1;
        return TRUE;
    }
    return FALSE;
}
#else
static void interrupted ( int sig )
{
    ( void ) sig;

    g_stop = 1;
}
#endif

void sleep_ms ( int ms )
{
#ifdef WIN32
    Sleep ( ms );
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec t;
    t.tv_sec = ms / 1000;
    t.tv_nsec = ( ms % 1000 ) * 1000000;
    nanosleep ( &t, NULL );
#else
    usleep ( ms * 1000 );
#endif
}

int main ( int argc, char** argv )
{
    ( void ) argc;
    ( void ) argv;

    struct cmmk state = {0};

    int product;

    if ( cmmk_find_device ( &product ) != 0 ) {
        return 1;
    }
    printf ( "Product: %s\n", cmmk_product_to_str ( product ) );

    if ( cmmk_attach ( &state, product, -1 ) != 0 ) {
        return 1;
    }
    printf ( "Layout: %s\n", cmmk_layout_to_str ( state.layout ) );

#ifdef WIN32
    SetConsoleCtrlHandler ( ctrl_handler, TRUE );
#else
    signal ( SIGINT, interrupted );
#endif

    g_stop = 0;

    struct rgb offcolor = MKRGB ( 0x000000 );
    struct rgb color1 = MKRGB ( 0x00ff00 );
    struct rgb color2 = MKRGB ( 0xff0000 );
    cmmk_set_control_mode ( &state, CMMK_MANUAL );
    cmmk_set_all_single ( &state, &offcolor );

    int max = 200;
    int start = 0;
    int c;

    if ( argc > 1 ) {
        start = atoi ( argv[1] );
    }
    if ( argc > 2 ) {
        max = atoi ( argv[2] );
    }

    if ( start > max ) {
        fprintf ( stderr, "Start (%i) has to be smaller than max (%i) value\n", start, max );
    }

    int step = ( max-start+1 ) /2;
    while ( start < max ) {
        fprintf ( stdout, "Checking from %i to %i\n", start, max -1 );
        for ( int i = start; i < max; ++i ) {
            if ( i < start + step ) {
                //first half
                cmmk_set_single_key_by_id ( &state, i, &color1 );
            } else {
                //second half
                cmmk_set_single_key_by_id ( &state, i, &color2 );
            }
        }
        fprintf ( stdout, "Is your key illuminated in green? (%i to %i)? [y/N] ", start, start + step - 1 );
        fflush ( stdout );
        do {
            c = getchar();
        } while ( !g_stop && c != 'y' && c != 'Y' && c != 'n' && c != 'N' );
        if ( g_stop ) {
            goto finish;
        }
        if ( c == 'y' || c == 'Y' ) {
            max = start + step;
            fprintf ( stdout, "Got it\n" );
        } else {
            //next interval
            start += step;
            fprintf ( stdout, "It is in the other half!   \n" );
        }
        cmmk_set_all_single ( &state, &offcolor );
        //half the interval
        if ( step == 1 ) {
            break;
        }
        step = ( step + 1 ) / 2;
    };
    fprintf ( stdout, "\nYour key is %i\n", start );

finish:
    cmmk_set_control_mode ( &state, CMMK_FIRMWARE );
    cmmk_detach ( &state );

    return 0;
}
