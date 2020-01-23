// ROMulator - RAM/ROM replacement and diagnostic for 8-bit CPUs
// Copyright (C) 2019  Michael Hill

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

module enable_logic(
  inout spi_clk, 
  input wire spi_miso, 
  output spi_out, 
  output spi_cs,

  input[15:0] address, 
  inout [7:0] data, 
  input phi2, 
  input rwbar, 
  output dataoutenable, 
  output busenable,
  input diag_spi_cs,
  output rdy,

  output wire led_blue,
  output wire led_green,
  output wire led_red
  );

integer i;
wire read_complete;

// set up in/out for SPI interface
// SPI Master active until RAM image loaded from flash
// then SPI slave waits for commands

wire spi_master_active = !read_complete;
wire spi_slave_active = read_complete;

wire spi_clk_in;
wire spi_clk_out;
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_spi_clk
  (
    .PACKAGE_PIN(spi_clk),
    .OUTPUT_ENABLE(spi_master_active),
    .D_OUT_0(spi_clk_out),
    .D_IN_0(spi_clk_in)
);

wire[7:0] wdatain;
wire[7:0] wdataout;

wire clk;
wire wdataout_enable;

assign wdataout_enable = read_complete & rwbar;

SB_HFOSC inthosc(.CLKHFPU(1'b1), .CLKHFEN(1'b1), .CLKHF(clk));

SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[0]
  (
    .PACKAGE_PIN(data[0]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[0]),
    .D_IN_0(wdatain[0])
  );
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[1]
  (
    .PACKAGE_PIN(data[1]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[1]),
    .D_IN_0(wdatain[1])
  );
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[2]
  (
    .PACKAGE_PIN(data[2]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[2]),
    .D_IN_0(wdatain[2])
  );
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[3]
  (
    .PACKAGE_PIN(data[3]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[3]),
    .D_IN_0(wdatain[3])
  );
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[4]
  (
    .PACKAGE_PIN(data[4]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[4]),
    .D_IN_0(wdatain[4])
  );
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[5]
  (
    .PACKAGE_PIN(data[5]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[5]),
    .D_IN_0(wdatain[5])
  );
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[6]
  (
    .PACKAGE_PIN(data[6]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[6]),
    .D_IN_0(wdatain[6])
  );
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[7]
  (
    .PACKAGE_PIN(data[7]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[7]),
    .D_IN_0(wdatain[7])
  );

// how to instantiate roms living at different addresses?
// create ROMS and RAMs of different sizes, specify addresses

wire cs_enable;
wire cs_enable_bus;
wire cs;
wire cs_bus;
wire we;

wire rx_ready;
wire [15:0] ram_address;
wire ram_cs;
wire ram_we;
wire [7:0] ram_datain;
wire [7:0] ram_dataout;

wire [15:0] flash_ram_address;
wire flash_ram_cs;
wire flash_ram_we;
wire [7:0] flash_ram_datain;
wire [7:0] diag_ram_datain;

wire [7:0] ram_input;

assign cs = read_complete & cs_enable;
assign cs_bus = read_complete & cs_enable_bus;

wire halt;

// only output data from RAM when CPU is not halted
assign wdataout = (halt) ? 8'h00 : ram_dataout;
//assign dataoutenable = !cs;
//assign busenable = cs;

assign dataoutenable = !cs;
assign busenable = !cs_bus;

// ram bus connection logic
// before read_complete, address/data/control connected to flash reader
// after, when cpu not halted, ram is connected to 6502 cpu
// if cpu halted, connect to diagnostics module

wire [15:0] diag_ram_address;
wire diag_ram_cs;
wire diag_ram_we;

wire cpu_ram_cs;
wire cpu_ram_we;

// select address bus between diagnostics reader and CPU bus
wire [15:0] cpu_ram_address;
assign cpu_ram_address = (halt) ? diag_ram_address : address;
assign cpu_ram_cs = (halt) ? diag_ram_cs : cs;
assign cpu_ram_we = (halt) ? diag_ram_we : we;

assign ram_address = (read_complete) ? cpu_ram_address : flash_ram_address;
//assign ram_datain = (read_complete) ? wdatain : flash_ram_datain;

assign ram_input = (halt) ? diag_ram_datain : wdatain;
assign ram_datain = (read_complete) ? ram_input : flash_ram_datain;

assign ram_cs = (read_complete) ? cpu_ram_cs : flash_ram_cs;
assign ram_we = (read_complete) ? cpu_ram_we : flash_ram_we;

wire flash_spi_out;
wire diag_spi_out;

assign spi_out = (spi_master_active) ? flash_spi_out : diag_spi_out;

wire echo_cs;
assign rdy = !halt;


assign led_blue = read_complete;
assign led_green = 1;
assign led_red = 1;

reg [3:0] configuration;

sram64k RAM(ram_address, ram_dataout, ram_datain, ram_cs, ram_we, clk);
ramenable enable(address, phi2, rwbar, cs_enable, cs_enable_bus, we, configuration, clk);
// create spi flash reader
// this fills RAM with selected ROM images
spi_flash_reader flashReader(
    spi_clk_out,
    spi_miso,
    flash_spi_out,
    spi_cs,

    rx_ready,
    echo_cs,

    flash_ram_address,
    ram_dataout,
    flash_ram_datain,
    flash_ram_cs,
    flash_ram_we,
    clk,

    read_complete,
    configuration
);

// fpga reset (unused now)
wire reset;
assign reset = 1;

// connect diagnostics module for halting cpu and reading ram
diagnostics diag(
  halt,
  reset,
  clk,
  diag_spi_cs,
  spi_clk_in,
  diag_spi_out,
  spi_miso,

  diag_ram_address,
  ram_dataout,
  diag_ram_datain,
  diag_ram_we,
  diag_ram_cs,

  configuration
);

initial
begin
  configuration <= ~wdatain[3:0];
end

endmodule