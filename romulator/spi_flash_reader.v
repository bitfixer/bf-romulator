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

module spi_flash_reader(
    output spi_clk, 
    input wire spi_miso, 
    output spi_mosi, 
    output spi_cs, 

    output reg rx_ready,
    output reg echo_cs,

    output [15:0] ram_address,
    input [7:0] ram_dataout,
    output [7:0] ram_datain,
    output ram_cs,
    output ram_we,

    input clk,
    output reg read_complete,
    input wire [4:0] flash_read_addr,

    output reg table_we
);

reg spi_clk_input;

// Master Specific
reg [7:0] r_Master_TX_Byte = 0;
reg r_Master_TX_DV;
reg w_Master_TX_Ready;
reg w_Master_RX_DV;
reg [7:0] w_Master_RX_Byte;

reg [7:0] spi_recv_byte;

// states for SPI byte transfer
localparam TX_IDLE  = 0;
localparam TX_PULSE = 1;
localparam TX_PULSE_DONE = 2;
localparam TX_DONE = 3;
reg [1:0] state = TX_IDLE;

reg [8:0] xfer_counter;
reg [8:0] xfer_bytes_to_send;
reg [8:0] xfer_bytes_to_echo;

wire rst;
assign rst = 1;
reg [15:0] counter;

reg tx_active;
reg spi_cs_reg;

// SPI transfer state
// controls process of loading from SPI into RAM
localparam XFER_IDLE = 0;
localparam XFER_WAKEUP_WAIT = 1;
localparam XFER_WAKEUP = 2;
localparam XFER_WAIT1 = 3;
localparam XFER_FLASHREAD_BLOCK = 4;
localparam XFER_FLASHREAD = 5;
localparam XFER_FLASHREAD_NEXT = 6;
localparam XFER_READ_BYTES_DONE = 7;
localparam XFER_SEND_BYTES = 8;
localparam XFER_SEND_BYTES_WAIT = 9;
localparam XFER_READ_BYTES = 10;
localparam XFER_RAM_WRITE = 11;
localparam XFER_RAM_WRITE_DONE = 12;
localparam XFER_DONE = 15;
reg [4:0] xfer_state;

reg [3:0] next_xfer_state;
reg [7:0] xfer_send_bytes [0:3];
reg xfer_read = 0;
reg [8:0] xfer_flash_blocks_to_read;

// memory addresses and signals
reg [23:0] flash_address = 0;
reg [15:0] ram_address = 16'h0000;
reg ram_cs = 0;
reg ram_we = 0;
wire [7:0] ram_datain;
wire [7:0] ram_dataout;

reg table_read = 0;

/*
localparam ADDR_GRANULARITY_SIZE = 256;
localparam ADDR_NUM_ENTRIES = 2**16 / ADDR_GRANULARITY_SIZE;
localparam ADDR_ENTRY_BITS = $clog2(ADDR_NUM_ENTRIES); // number of bits to use for address granularity.
localparam CONFIG_BITS = 5;
localparam ENABLE_ADDR_BITS = ADDR_ENTRY_BITS + CONFIG_BITS + 1;
reg [1:0] enable_table[0:(2**ENABLE_ADDR_BITS) - 1];
*/

assign spi_cs = spi_cs_reg;
assign ram_datain = spi_recv_byte;

SPI_Master #(0, 3)
SPIMaster
  (
   // Control/Data Signals,
   .i_Rst_L(rst),                   // FPGA Reset
   .i_Clk(spi_clk_input),           // FPGA Clock
   
   // TX (MOSI) Signals
   .i_TX_Byte(r_Master_TX_Byte),    // Byte to transmit on MOSI
   .i_TX_DV(r_Master_TX_DV),        // Data Valid Pulse with i_TX_Byte
   .o_TX_Ready(w_Master_TX_Ready),  // Transmit Ready for Byte
   
   // RX (MISO) Signals
   .o_RX_DV(w_Master_RX_DV),        // Data Valid pulse (1 clock cycle)
   .o_RX_Byte(w_Master_RX_Byte),    // Byte received on MISO

   // SPI Interface
   .o_SPI_Clk(spi_clk),
   .i_SPI_MISO(spi_miso),
   .o_SPI_MOSI(spi_mosi)
   );

// generate clock input for SPI
always @(posedge clk)
begin
    counter <= counter + 1;
    if (counter == 1)
    begin
        counter <= 0;
        if (spi_clk_input == 0) 
        begin
            spi_clk_input <= 1;
        end
        else begin
            spi_clk_input <= 0;
        end
    end
end

// Send 1 byte over SPI
always @(posedge spi_clk_input)
begin
    case (state)
    TX_IDLE:
    begin
        if (tx_active == 1)
        begin
            r_Master_TX_DV <= 1;
            state <= TX_PULSE;
        end
    end
    TX_PULSE:
    begin
        r_Master_TX_DV <= 1'b0;
        state <= TX_PULSE_DONE;
    end
    TX_PULSE_DONE:
    begin
        if (w_Master_TX_Ready == 1'b1)
        begin
            state <= TX_DONE;
        end
    end
    TX_DONE:
    begin
        if (tx_active == 0)
        begin
            state = TX_IDLE;
        end
    end
    endcase
end

always @(posedge spi_clk_input)
begin
    case(xfer_state)
    XFER_WAIT1:
    begin
        xfer_counter <= xfer_counter - 1;
        if (xfer_counter == 0)
        begin
            xfer_state = XFER_IDLE;
        end
    end
    XFER_IDLE:
    begin
        // do wakeup command
        r_Master_TX_Byte <= 8'hab;
        spi_cs_reg <= 0;
        tx_active <= 1;
        xfer_state <= XFER_WAKEUP_WAIT;
    end
    XFER_WAKEUP_WAIT:
    begin
        // wait until command is finished
        if (state == TX_DONE)
        begin
            // indicate done with transfer, move to next
            tx_active <= 0;
            spi_cs_reg <= 1;

            // reading 4096 bytes
            xfer_flash_blocks_to_read <= 256;
            flash_address <= 24'h20000 + (flash_read_addr << 16);

            xfer_state = XFER_FLASHREAD_BLOCK;
        end
    end
    XFER_FLASHREAD_NEXT:
    begin
        xfer_flash_blocks_to_read = xfer_flash_blocks_to_read - 1;
        if (xfer_flash_blocks_to_read == 0)
        begin
            //xfer_state <= XFER_DATA_ECHO;
            //xfer_state <= XFER_DONE;

            if (table_read == 1)
            begin
                xfer_state <= XFER_DONE;
            end
            else 
            begin
                xfer_flash_blocks_to_read = 64;
                rx_ready <= 1;
                flash_address <= 24'h220000;
                table_read <= 1;
                ram_address <= 0;
                xfer_state <= XFER_FLASHREAD_BLOCK;
            end
        end
        else 
        begin
            rx_ready <= 1;
            flash_address <= flash_address + 256;
            xfer_state <= XFER_FLASHREAD_BLOCK;
        end
    end
    XFER_FLASHREAD_BLOCK:
    begin
        // send command to read a specific address
        // to spi flash
        if (state == TX_IDLE)
        begin
            xfer_counter <= 0;
            spi_cs_reg <= 0;
            xfer_bytes_to_send <= 4;

            // 1 byte command for flash read
            xfer_send_bytes[0] <= 8'h03;
            
            // set flash address to read
            xfer_send_bytes[1] <= flash_address[23:16];
            xfer_send_bytes[2] <= flash_address[15:8];
            xfer_send_bytes[3] <= flash_address[7:0];

            // next state - read bytes from flash
            next_xfer_state <= XFER_READ_BYTES;

            // send specified number of bytes
            xfer_state <= XFER_SEND_BYTES;
            xfer_read <= 0;
        end
    end
    XFER_READ_BYTES:
    begin
        if (state == TX_IDLE)
        begin
            xfer_counter <= 0;
            xfer_bytes_to_send <= 256;
            
            next_xfer_state <= XFER_READ_BYTES_DONE;
            xfer_state <= XFER_SEND_BYTES;
            xfer_read <= 1;
        end
    end
    XFER_READ_BYTES_DONE:
    begin
        spi_cs_reg <= 1;
        xfer_state <= XFER_FLASHREAD_NEXT;
    end
    XFER_SEND_BYTES:
    begin
        // send bytes while counter > 0
        if (xfer_counter >= xfer_bytes_to_send)
        begin
            xfer_state <= next_xfer_state;
        end
        else 
        begin
            // send the next byte
            tx_active <= 1;
            if (xfer_read == 1)
            begin
                // this is a read
                // send a 0 on SPI, read the MISO byte
                // after byte read, write it to RAM
                r_Master_TX_Byte <= 0;
                xfer_state <= XFER_RAM_WRITE;
            end
            else
            begin
                r_Master_TX_Byte <= xfer_send_bytes[xfer_counter];
                xfer_state <= XFER_SEND_BYTES_WAIT;
            end
            xfer_counter <= xfer_counter + 1;
            
        end
    end
    // write single byte to ram
    XFER_RAM_WRITE:
    begin
        if (state == TX_DONE)
        begin
            // if this is a read, get current value and
            // write to appropriate place in RAM

            // address, CS, WE to select RAM
            if (table_read == 1)
            begin
                //enable_table[ram_address[13:0]] <= spi_recv_byte[1:0];
                table_we <= 1;
            end
            else 
            begin
                ram_cs <= 1;
                ram_we <= 1;
            end
            xfer_state <= XFER_RAM_WRITE_DONE;
        end
    end
    XFER_RAM_WRITE_DONE:
    begin
        // deselect RAM
        ram_cs <= 0;
        ram_we <= 0;
        table_we <= 0;
        ram_address = ram_address + 1;
        xfer_state <= XFER_SEND_BYTES_WAIT;
    end
    XFER_SEND_BYTES_WAIT:
    begin
        if (state == TX_DONE)
        begin
            tx_active <= 0;
            xfer_state <= XFER_SEND_BYTES;
        end
    end
    XFER_DONE:
    begin
        spi_cs_reg <= 1;
        echo_cs <= 1;
        read_complete <= 1;
    end
    endcase
end

// capture received byte from SPI when it is valid
always @(negedge w_Master_RX_DV)
begin
    // when we are signalled that the recv byte is valid,
    // read the byte and present to RAM
    spi_recv_byte <= w_Master_RX_Byte;
end

initial
begin
    state <= TX_IDLE;
    xfer_state <= XFER_WAIT1;
    tx_active <= 0;
    xfer_counter <= 100;
    spi_cs_reg <= 1;
    echo_cs <= 1;
    read_complete <= 0;
end


endmodule