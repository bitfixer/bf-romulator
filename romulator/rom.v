module rom(address, data, CS, clk);

parameter DATA_WIDTH = 8;
parameter ADDR_WIDTH = 16;
parameter FNAME = "";

output[DATA_WIDTH-1:0] data;
input[ADDR_WIDTH-1:0] address;
input CS;
input clk;

reg [DATA_WIDTH-1:0] data;
reg [DATA_WIDTH-1:0] ROM[0:(2**ADDR_WIDTH)-1];

always @(negedge(clk))
begin
    if (!CS) begin
        data = ROM[address];
    end
    else begin
        data = 8'hz;
    end
end

initial begin
    if (FNAME != "") begin
        $readmemh(FNAME, ROM);
    end
    
end

endmodule