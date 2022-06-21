module ramenable(
    input [15:0] address,
    input phi2,
    input rwbar,
    input mreq,
    output wire cs_ram,
    output wire cs_bus,
    output we,
    input fpga_clk,
    input wire table_we,
    input wire [1:0] table_val,
    input wire [8:0] table_write_addr,
    input wire ram_disable,
    input wire rom_disable);

// table of ram and bus enable signals, 64 entries per table
// this is selected by the configuration byte

// specify minimum size of region which can have its own enable setting
localparam ADDR_GRANULARITY_SIZE = 256;

// calculate number of entries based on granularity size
localparam ADDR_NUM_ENTRIES = 2**16 / ADDR_GRANULARITY_SIZE;

// number of bits needed for the number of address entries
localparam ADDR_ENTRY_BITS = $clog2(ADDR_NUM_ENTRIES); // number of bits to use for address granularity.

// enable address is composed of:
// number of bits for address entry
// number of bits for configuration
// 1 bit for read/write
//localparam ENABLE_ADDR_BITS = ADDR_ENTRY_BITS + CONFIG_BITS + 1;
localparam ENABLE_ADDR_BITS = ADDR_ENTRY_BITS + 1;

reg [1:0] enable_table[0:(2**ENABLE_ADDR_BITS) - 1];
wire[ENABLE_ADDR_BITS-1:0] enable_addr;
reg [1:0] outval;

wire [ENABLE_ADDR_BITS-1:0] write_enable_addr;
wire [ENABLE_ADDR_BITS-1:0] read_enable_addr;
reg disable_region = 0;

always @(posedge fpga_clk)
begin
    if (table_we == 1)
    begin
        enable_table[table_write_addr] <= table_val;
    end
    else 
    begin
        outval <= enable_table[enable_addr];
        if (rom_disable && enable_table[write_enable_addr][1] == 0 && enable_table[read_enable_addr][1] == 1)
        begin
            disable_region <= 1;
        end
        else if  (ram_disable && enable_table[write_enable_addr][1] == 1 && enable_table[read_enable_addr][1] == 1)
        begin
            disable_region <= 1;
        end
        else
        begin
            disable_region <= 0;
        end
    end
end

assign we = phi2 & (!rwbar);
assign enable_addr = {rwbar, address[15:15 - ADDR_ENTRY_BITS + 1]};
assign write_enable_addr = {1'b0, address[15:15 - ADDR_ENTRY_BITS + 1]};
assign read_enable_addr = {0'b0, address[15:15 - ADDR_ENTRY_BITS + 1]};

assign cs_ram = (phi2 & outval[1]) & mreq && !disable_region;
assign cs_bus = ((phi2 & outval[0]) || !mreq) || disable_region;

endmodule