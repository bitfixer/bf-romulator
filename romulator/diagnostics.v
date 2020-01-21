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

module diagnostics(
     output reg halt,   // io line to halt the cpu

     // spi slave
     input      fpga_reset,
     input      fpga_clk,
     input      spi_select,
     input      spi_clk,
     output     spi_miso,
     input wire spi_mosi,

     // ram control
     output reg [15:0] address,
     input      [7:0] data,
     output reg [7:0] data_out,
     output reg we,
     output reg cs,

     input      [1:0] configuration
);

// module states
localparam RUNNING = 0;
localparam HALTED = 1;
localparam WRITE_MEMORY_BYTE = 2;
localparam WRITE_MEMORY_BYTE_WAIT = 3;
localparam WRITE_MEMORY_BYTE_NEXT = 4;
localparam WRITE_CONFIG_BYTE = 5;
localparam OUTPUT_MEMORY_BYTE = 6;
localparam OUTPUT_MEMORY_BYTE_WRITEDONE = 7;
localparam OUTPUT_MEMORY_BYTE_WAIT = 8;
localparam OUTPUT_MEMORY_BYTE_NEXT = 9;
localparam STARTUP = 10;
reg [7:0] state = RUNNING;

// spi slave setup
wire rx_dv;
wire [7:0] rx_byte;
reg tx_dv = 0;
reg [7:0] tx_byte = 8'h00;

reg read_done = 0;
reg write_started = 0;
reg [1:0] config_byte;

localparam HALT_CPU = 8'haa;
localparam RESUME_CPU = 8'h55;
localparam READ_MEMORY = 8'h66;
localparam READ_CONFIG = 8'h77;
localparam WRITE_MEMORY = 8'h99;

SPI_Slave spiSlave(
  .i_Rst_L(fpga_reset),
  .i_Clk(fpga_clk),
  .o_RX_DV(rx_dv),
  .o_RX_Byte(rx_byte),
  .i_TX_DV(tx_dv),
  .i_TX_Byte(tx_byte),

  .i_SPI_Clk(spi_clk),
  .o_SPI_MISO(spi_miso),
  .i_SPI_MOSI(spi_mosi),
  .i_SPI_CS_n(spi_select)
  );

// main state machine for diagnostics
always @(posedge fpga_clk)
begin
    case(state)
    STARTUP:
    begin
      config_byte <= configuration;
      state <= RUNNING;
    end
    RUNNING:
    begin
        // check for halt command
        if (rx_dv == 1'b1) // byte received from spi
        begin
            if (rx_byte == HALT_CPU) // halt command
            begin
                halt <= 1;
                state <= HALTED;
            end
            else if (rx_byte == READ_CONFIG)
            begin
                tx_dv <= 1;
                tx_byte <= config_byte;
                state <= WRITE_CONFIG_BYTE;
            end
        end
    end
    WRITE_CONFIG_BYTE:
    begin
        tx_dv <= 0;
        if (rx_dv == 1'b1)
        begin
          state <= RUNNING;
        end
    end
    HALTED:
    // in halt state, waiting for next command
    // from here, can resume or send contents of memory map.
    begin
        if (rx_dv == 1'b1)
        begin
          if (rx_byte == RESUME_CPU) // resume command
          begin
            halt <= 0;
            state <= RUNNING;
          end
          else if (rx_byte == READ_MEMORY) // read out a full memory map
          begin
            state <= WRITE_MEMORY_BYTE;
            cs <= 1;
            we <= 0;
            read_done <= 0;
            address <= 0;
          end
          else if (rx_byte == WRITE_MEMORY) // write a memory map, retrieved from spi
          begin
            state <= OUTPUT_MEMORY_BYTE_NEXT;
            cs <= 1;
            we <= 0;
            read_done <= 0;
            write_started <= 0;
            address <= 0;
          end
        end
    end
    OUTPUT_MEMORY_BYTE:
    begin
      tx_dv <= 1;
      tx_byte <= data_out;

      // write the byte currently on the data out bus
      we <= 1;
      state <= OUTPUT_MEMORY_BYTE_WRITEDONE;
    end
    OUTPUT_MEMORY_BYTE_WRITEDONE:
    begin
      tx_dv <= 0;
      // disable write
      we <= 0;
      state <= OUTPUT_MEMORY_BYTE_WAIT;
    end
    OUTPUT_MEMORY_BYTE_WAIT:
    // now waiting for next byte to be receieved
    begin
      // increment write address
      address <= address + 1;
      state <= OUTPUT_MEMORY_BYTE_NEXT;
    end
    OUTPUT_MEMORY_BYTE_NEXT:
    // received a byte from spi. This is one byte in the memory map we are receiving.
    // put this byte on the data output
    begin
      if (rx_dv == 1'b1) // received a byte over spi
      begin
        if (write_started == 1 && address == 0)
        begin
          // done writing memory map
          // back to halt state
          read_done <= 0;
          address <= 0;
          cs <= 0;
          we <= 0;
          state <= HALTED;
        end
        else
        begin
          // place received byte onto data bus
          write_started <= 1;
          data_out <= rx_byte;
          state <= OUTPUT_MEMORY_BYTE;
        end
      end
    end
    WRITE_MEMORY_BYTE:
    begin
      // present memory byte
      tx_dv <= 1;
      tx_byte <= data; // read from data bus
      state <= WRITE_MEMORY_BYTE_WAIT;
    end
    WRITE_MEMORY_BYTE_WAIT:
    begin
      tx_dv <= 0;
      address <= address + 1; // increment address
      state <= WRITE_MEMORY_BYTE_NEXT;
    end
    WRITE_MEMORY_BYTE_NEXT:
    begin
      if (rx_dv == 1'b1)
      begin
        if (address == 0)
        begin
          // done reading
          read_done <= 0;
          address <= 0;
          cs <= 0;
          we <= 0;
          state <= HALTED;
        end
        else 
        begin
          state <= WRITE_MEMORY_BYTE;
        end
      end
    end
    endcase
end

initial
begin
    halt <= 0;
    address <= 0;
    we <= 0;
    cs <= 0;
end

endmodule