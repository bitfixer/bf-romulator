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

module test_flash_reader(
    output [15:0] ram_address,
    input [7:0] ram_dataout,
    output [7:0] ram_datain,
    output ram_cs,
    output ram_we,

    input clk,
    output reg read_complete,
);

reg spi_clk_input;

// SPI transfer state
// controls process of loading from SPI into RAM
localparam XFER_IDLE = 0;
localparam XFER_WAIT1 = 3;
localparam XFER_WAIT2 = 4;
localparam XFER_RAM_WRITE = 11;
localparam XFER_RAM_WRITE_DONE = 12;
localparam XFER_DONE = 15;
reg [4:0] xfer_state;
reg [15:0] counter = 16'h0000;

reg [15:0] ram_address;
reg ram_cs = 0;
reg ram_we = 0;
wire [7:0] ram_datain;
wire [7:0] ram_dataout;

assign ram_datain = testbyte;
reg [7:0] testbyte = 8'h01;

// generate clock input for SPI
always @(posedge clk)
begin
    counter <= counter + 1;
    if (counter == 16)
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

always @(posedge spi_clk_input)
begin
    case(xfer_state)
    XFER_IDLE:
    begin
        ram_cs <= 1;
        xfer_state <= XFER_RAM_WRITE;
    end
    XFER_RAM_WRITE:
    begin
        ram_we <= 1;
        xfer_state <= XFER_WAIT1;
    end
    XFER_WAIT1:
    begin
        ram_we <= 0;
        xfer_state <= XFER_WAIT2;
    end
    XFER_WAIT2:
    begin
        ram_address <= ram_address + 1;
        testbyte <= testbyte + 1;
        xfer_state <= XFER_RAM_WRITE_DONE;
    end
    XFER_RAM_WRITE_DONE:
    begin
        if (ram_address == 0)
        begin
            xfer_state <= XFER_DONE;
        end
        else 
        begin
            xfer_state <= XFER_RAM_WRITE;
        end
    end
    XFER_DONE:
    begin
        ram_cs <= 0;
        read_complete <= 1;
    end
    endcase
end

/*
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
            xfer_state <= XFER_DONE;
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
                testbyte <= testbyte + 1;

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
            ram_cs <= 1;
            ram_we <= 1;
            xfer_state <= XFER_RAM_WRITE_DONE;
        end
    end
    XFER_RAM_WRITE_DONE:
    begin
        // deselect RAM
        ram_cs <= 0;
        ram_we <= 0;
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
*/

initial
begin
    xfer_state <= XFER_IDLE;
    read_complete <= 0;
    ram_address <= 16'h00000;
end


endmodule