module sram64k(address, dataout, datain, CS, WE, clk);
input [15:0] address;
output [7:0] dataout;
input [7:0] datain;
input CS;
input WE;
input clk;

wire [14:0]address_in;
wire [7:0]sram_dataout [0:1];

wire [0:1]sram_CS;

assign address_in = address[14:0];
// use 2 32k srams to form one 64k module
assign sram_CS[0] = CS & (address[15] == 0);
assign sram_CS[1] = CS & (address[15] == 1);

assign dataout = (address[15]) ? sram_dataout[1] : sram_dataout[0];

sram sram_lo(
    address_in,
    sram_dataout[0],
    datain,
    sram_CS[0],
    WE,
    clk
);

sram sram_hi(
    address_in,
    sram_dataout[1],
    datain,
    sram_CS[1],
    WE,
    clk
);

endmodule