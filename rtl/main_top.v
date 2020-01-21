
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

wire reset = 1'b1;
wire chipset_decode = A[23:12] != 12'hDFF;
wire ciaa_decode = A[23:12] != 12'hBFE;
wire ciab_decode = A[23:12] != 12'hBFD;

// chipset locations
localparam JOY0DAT_H = 12'h00A;
localparam JOY0DAT_L = JOY0DAT_H + 1;
localparam JOY1DAT_H = 12'h00C;
localparam JOY1DAT_L = JOY1DAT_H + 1;

wire [5:0] joya;
wire [5:0] joyb;

wire [2:0] mouse_buttons;
wire [15:0] mouse0dat;


//// user io has an extra spi channel outside minimig core ////
user_io user_io(

     .SPI_CLK(SPI_CK),
     .SPI_SS_IO(1'b0),
     .SPI_MISO(SPI_MISO),
     .SPI_MOSI(SPI_MOSI),

     .JOY0(joya),
     .JOY1(joyb),

     .MOUSE_BUTTONS(mouse_buttons),
     
     .CORE_TYPE(8'ha1),    // minimig core id
	 
     .BUTTONS(board_buttons),
	 .SWITCHES(board_switches),

     .MOUSE_DATA(mouse0dat)

  );

wire _mleft = ~mouse_buttons[0];
wire _mright = ~mouse_buttons[1];
wire _mthird = ~mouse_buttons[2];

reg	joy1enable = 0;

//port 1 automatic mouse/joystick switch
always @(posedge CLKCPU_A) begin 

	if (!_mleft || reset) //when left mouse button pushed, switch to mouse (default)
		joy1enable = 0;
	else if (!joya[4])//when joystick 1 fire pushed, switch to joystick
		joy1enable = 0;

end

wire reading_data = chipset_decode | ~RW | ~PUNT_IN;
wire reading_cia = (A[23:0] != 24'hBFE001) | ~RW | ~PUNT_IN;

reg [7:0] data_out;
reg       punt_int;
reg       dsack_int;
reg [1:0] dsack_int_d;

wire fire0 = joya[4] & _mleft;
wire fire1 = joyb[4];

always @(posedge CLKCPU_A or posedge AS20) begin 

    if (AS20 == 1'b1) begin 

        punt_int <= 1'b1;
        dsack_int <= 1'b1;
        dsack_int_d <= 2'b11;

    end else begin 

        if (reading_data == 1'b0) begin 

            punt_int <= 1'b0;

            case (A[11:0]) 
                
                // basic joypad directional data. 
                JOY0DAT_H: data_out <= mouse0dat[15:8];
                JOY0DAT_L: data_out <= mouse0dat[7:0];
                JOY1DAT_H: data_out <= {6'b000000,~joyb[0],joyb[2]^joyb[0]};
                JOY1DAT_L: data_out <= {6'b000000,~joyb[1],joyb[3]^joyb[1]};
                default: punt_int <= 1'b1;

            endcase 

        end 

      /*if (reading_cia == 1'b0) begin 

            punt_int <= 1'b0;
            data_out <= {fire1, fire0, 6'b00_0001};

        end 
*/
        dsack_int <= DS20 | punt_int;
        dsack_int_d <=  {dsack_int_d[0], dsack_int};
        
    end 

end

// punt works by respecting the accelerator punt over our punt.
assign PUNT_OUT = PUNT_IN ? punt_int : 1'b0;

// pass the amiga serial port to the ARM so we can see its debug output
assign UART5_RX = TXD ? 1'bz : 1'b0;
assign RXD = UART5_TX ? 1'bz : 1'b0;

assign KB_DATA = 1'bz;
assign KB_CLOCK = 1'bz;

assign D[31:24] = punt_int ? {8{1'bz}} : data_out;
assign DSACK = punt_int ? 2'bzz : {1'b1, dsack_int_d[1]};

endmodule