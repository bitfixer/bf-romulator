module ramenable(
    input [15:0] address,
    input phi2,
    input rwbar,
    output cs_ram,
    output cs_bus,
    output we,
    input [1:0] configuration,
    input fpga_clk);

// table of ram and bus enable signals, 64 entries per table
// this is selected by the configuration byte
reg [1:0] enable_table[0:255];
wire[7:0] enable_addr;
reg [1:0] config_byte;

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

assign we = phi2 & (!rwbar);
assign enable_addr = {config_byte, rwbar, address[11], address[15:12]};

assign cs_ram = phi2 & enable_table[enable_addr][1];
assign cs_bus = phi2 & enable_table[enable_addr][0];

initial
begin
    $readmemh("../bin/enable_table.txt", enable_table);
end

endmodule