/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_WS2811STRIP WS2811 LED Strip Functions
 * @brief Hardware functions to deal with the intelligent LED stip
 * @{
 *
 * @file       pios_ws2811strip.h
 * @author     Liam BEGUIN <liambeguin@gmail.com>
 * @brief      WS2811 LED Strip Routines based on Cleanflight
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef PIOS_WS2811STRIP_H
#define PIOS_WS2811STRIP_H

/* Public Types */
typedef enum {
    RGB_RED = 0,
    RGB_GREEN,
    RGB_BLUE
} colorComponent_e;
#define RGB_COLOR_COMPONENT_COUNT (RGB_BLUE + 1)

struct rgbColor24bpp_s {
    uint8_t r;
	uint8_t g;
    uint8_t b;
};

typedef union {
    struct rgbColor24bpp_s rgb;
    uint8_t raw[RGB_COLOR_COMPONENT_COUNT];
} rgbColor24bpp_t;

/* Public Functions */
extern void PIOS_WS2811STRIP_Init(void);

#endif /* PIOS_WS2811STRIP_H */

/**
  * @}
  * @}
  */
