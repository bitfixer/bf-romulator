module sram(address, dataout, datain, CS, WE, clk);

input [14:0] address;
output [7:0] dataout;
input [7:0] datain;
input clk;
wire [13:0]address_in;
wire [15:0]spram_datain;
wire [15:0]spram_dataout;
wire [3:0]maskwren;
wire [7:0]zeros;

input CS;
input WE;

assign address_in = address[13:0];
assign dataout = (address[14]) ? spram_dataout[15:8] : spram_dataout[7:0];
assign zeros = 8'h0;
assign spram_datain = (address[14]) ? {datain, zeros} : {zeros, datain};

assign maskwren[3] = address[14];
assign maskwren[2] = address[14];
assign maskwren[1] = !address[14];
assign maskwren[0] = !address[14];

SB_SPRAM256KA  ramfn_inst1(
                .DATAIN(spram_datain),
                .ADDRESS(address_in),
                .MASKWREN(maskwren),
                .WREN(WE),
                .CHIPSELECT(CS),
                .CLOCK(clk),
                .STANDBY(0),
                .SLEEP(0),
                .POWEROFF(1),
                .DATAOUT(spram_dataout)
);

endmodule