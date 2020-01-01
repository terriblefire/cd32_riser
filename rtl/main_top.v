
`timescale 1ns / 1ps
/*
    Copyright (C) 2020, Stephen J. Leary
    All rights reserved.
    
    This file is part of CD32 USB Riser

    CD32 USB Riser is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    CD32 USB Riser is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty     You should have received a copy of the GNU General Public Licenseof
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.


    along with CD32 USB Riser. If not, see <http://www.gnu.org/licenses/>.
*/

module main_top(

    input CLKCPU_A, 
    input AS20, 
    input DS20, 
    input RW, 
    input [23:0] A,
    
    inout [31:24] D,

    // Punting... 
    input PUNT_IN, 
    output PUNT_OUT,

    // ARM UART 

    input UART5_TX, 
    output UART5_RX,

    // CD32 UART

    input TXD, 
    output RXD, 

    // SPI COMMS 

    input SPI_CK, 
    input SPI_NSS, 
    input SPI_MOSI, 
    output SPI_MISO

);

// bare min to make the riser work with a card.
assign PUNT_OUT = PUNT_IN ? 1'bz : 1'b0;


endmodule