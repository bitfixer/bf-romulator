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
  input rd, 
  input wr, 
  output dataoutenable, 
  output busenable,
  inout diag_spi_cs,
  output rwait,
  input bwait,
  input m1,

  output wire led_blue,
  input wire mreq,
  input wire ioreq
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

wire diag_spi_cs_in;
wire diag_spi_cs_out;
SB_IO #(
    .PIN_TYPE(6'b 1010_01),
    .PULLUP(1'b 1)
  )
  iobuf_diag_spi_cs
  (
    .PACKAGE_PIN(diag_spi_cs),
    .OUTPUT_ENABLE(0),
    .D_OUT_0(diag_spi_cs_out),
    .D_IN_0(diag_spi_cs_in)
);

wire[7:0] wdatain;
wire[7:0] wdataout;

wire clk;
wire wdataout_enable;

wire rwbar;
wire phi2;
assign rwbar = wr;
assign wdataout_enable = read_complete & rwbar;

// todo: use mreq
assign phi2 = (!wr || !rd);

// set up internal clock
SB_HFOSC inthosc(.CLKHFPU(1'b1), .CLKHFEN(1'b1), .CLKHF(clk));

// set up bidirectional data bus. Data pins switch direction based on wdataout_enable signal.
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 1)
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
    .PULLUP(1'b 1)
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
    .PULLUP(1'b 1)
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
    .PULLUP(1'b 1)
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
    .PULLUP(1'b 1)
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
    .PULLUP(1'b 1)
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
    .PULLUP(1'b 1)
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
    .PULLUP(1'b 1)
  )
  iobuf_data[7]
  (
    .PACKAGE_PIN(data[7]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[7]),
    .D_IN_0(wdatain[7])
  );

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
//assign rdy = !halt && read_complete;
assign rwait = bwait || !(!halt && read_complete);

assign led_blue = read_complete && !rwait;

// number of bits in configuration
localparam CONFIG_BITS = 5;

reg [CONFIG_BITS-1:0] configuration;
wire [CONFIG_BITS-1:0] config_byte;

sram64k RAM(ram_address, ram_dataout, ram_datain, ram_cs, ram_we, clk);
ramenable enable(address, phi2, rwbar, cs_enable, cs_enable_bus, we, config_byte, clk);
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

// write enable for video ram section
// use regular we signal, also check that this is the right section of memory
reg [15:0] vram_start[15:0];
reg [15:0] vram_end[15:0];
wire vram_we;

assign vram_we = ((ram_address >= vram_start[config_byte] && ram_address < vram_end[config_byte]) 
    || ram_address == 59468)
    && ram_we;

wire [10:0]vram_read_address;
wire [7:0]vram_output;
wire vram_read_clock;


// the address within the VRAM region to write a byte.
// normally this is the offset from the designated start address for this configuration.
// on the PET, address 59468 indicates which character bank to use. This is mapped to the last byte in VRAM.
wire[10:0]vram_write_address = ram_address == 59468 ? (vram_size-1) : ram_address - vram_start[config_byte];
wire [10:0]vram_size = vram_end[config_byte] - vram_start[config_byte];

// include dual ported ram for the vram section
simple_ram_dual_clock #(8, 11)
videoRam
(
    .data(ram_datain),
    .read_addr(vram_read_address),
    .write_addr(vram_write_address),
    .we(vram_we),
    .read_clk(vram_read_clock),
    .write_clk(clk),
    .q(vram_output),
);

// fpga reset (unused now)
wire reset;
assign reset = 1;

// connect diagnostics module for halting cpu and reading ram
// diagnostics also has the register for config_byte which is used to select
// which memory configuration to use.
diagnostics diag(
  halt,
  reset,
  clk,
  diag_spi_cs_in,
  spi_clk_in,
  diag_spi_out,
  spi_miso,

  diag_ram_address,
  ram_dataout,
  diag_ram_datain,
  diag_ram_we,
  diag_ram_cs,

  configuration,

  vram_read_address,
  vram_output,
  vram_read_clock,

  config_byte,
  vram_size
);

initial
begin
  configuration <= ~wdatain[CONFIG_BITS-1:0];
  $readmemh("../bin/vram_start_addr.txt", vram_start);
  $readmemh("../bin/vram_end_addr.txt", vram_end);
end

endmodule