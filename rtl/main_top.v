
`timescale 1ns / 1ps
/*
    Copyright (C) 2020, Stephen J. Leary
    All rights reserved.
    
    This file is part of CD32 USB Riser

    CD32 USB Riser is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.     
    
    You should have received a copy of the GNU General Public License
    along with CD32 USB Riser. If not, see <http://www.gnu.org/licenses/>.
*/

module main_top(

    input CLKCPU_A, 
    input AS20, 
    input DS20, 
    input RW, 
    input [1:0] SIZ,
    input [23:0] A,
    
    inout [31:24] D,
    output [1:0] DSACK,

    // Punting... 
    input PUNT_IN, 
    output PUNT_OUT,

    // ARM UART 

    input UART5_TX, 
    output UART5_RX,

    // CD32 UART

    input TXD, 
    output RXD, 

    output KB_CLOCK, 
    output KB_DATA,


    // SPI COMMS 

    input SPI_CK, 
    input SPI_NSS, 
    input SPI_MOSI, 
    output SPI_MISO

);

wire chipset_decode = A[23:12] != 12'hDFF;
wire ciaa_decode = A[23:12] != 12'hBFE;
wire ciab_decode = A[23:12] != 12'hBFD;

wire [5:0] joya;
wire [5:0] joyb;



//// user io has an extra spi channel outside minimig core ////
user_io user_io(

     .SPI_CLK(SPI_CK),
     .SPI_SS_IO(SPI_NSS),
     .SPI_MISO(SPI_MISO),
     .SPI_MOSI(SPI_MOSI),

     .JOY0(joya),
     .JOY1(joyb),
     
     .MOUSE_BUTTONS(mouse_buttons),
     
     .KBD_MOUSE_DATA(kbd_mouse_data),
     .KBD_MOUSE_TYPE(kbd_mouse_type),
     .KBD_MOUSE_STROBE(kbd_mouse_strobe),
     
     .CORE_TYPE(8'ha1),    // minimig core id
	 
     .BUTTONS(board_buttons),
	 .SWITCHES(board_switches)
  );




// bare min to make the riser work with a card.
assign PUNT_OUT = PUNT_IN ? 1'bz : 1'b0;

// pass the amiga serial port to the ARM so we can see its debug output
assign UART5_RX = TXD;
assign RXD = UART5_TX;

endmodule