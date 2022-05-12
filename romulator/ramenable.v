module ramenable(
    input [15:0] address,
    input phi2,
    input rwbar,
    output cs_ram,
    output cs_bus,
    output we,
    input [4:0] configuration,
    input fpga_clk,
    input [13:0] enable_table_write_address,
    input [1:0] enable_table_value,
    input enable_table_we);

// table of ram and bus enable signals, 64 entries per table
// this is selected by the configuration byte

// specify minimum size of region which can have its own enable setting
localparam ADDR_GRANULARITY_SIZE = 256;

// calculate number of entries based on granularity size
localparam ADDR_NUM_ENTRIES = 2**16 / ADDR_GRANULARITY_SIZE;

// number of bits needed for the number of address entries
localparam ADDR_ENTRY_BITS = $clog2(ADDR_NUM_ENTRIES); // number of bits to use for address granularity.

// number of bits in configuration
localparam CONFIG_BITS = 5;

// enable address is composed of:
// number of bits for address entry
// number of bits for configuration
// 1 bit for read/write
localparam ENABLE_ADDR_BITS = ADDR_ENTRY_BITS + CONFIG_BITS + 1;

reg [1:0] enable_table[0:(2**ENABLE_ADDR_BITS) - 1];
wire[ENABLE_ADDR_BITS-1:0] enable_addr;
reg [CONFIG_BITS-1:0] config_byte;

// module states
localparam READ_CONFIG = 0;
localparam DONE = 1;
reg state = READ_CONFIG;

always @(posedge fpga_clk)
begin
    case(state)
    READ_CONFIG:
    begin
      config_byte <= configuration;
      state <= DONE;
    end
    endcase
end

// when write enable is positive, load in value into the enable table
always @(posedge enable_table_we)
begin
    enable_table[enable_table_write_address] <= enable_table_value;
end

assign we = phi2 & (!rwbar);
assign enable_addr = {configuration, rwbar, address[15:15 - ADDR_ENTRY_BITS + 1]};

assign cs_ram = (!enable_table_we) & phi2 & enable_table[enable_addr][1];
assign cs_bus = (!enable_table_we) & phi2 & enable_table[enable_addr][0];

initial
begin
    $readmemh("../bin/enable_table.txt", enable_table);
end

endmodule