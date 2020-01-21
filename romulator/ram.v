module ram(address, dataa, datain, CS, WE, clk);

parameter DATA_WIDTH = 8;
parameter ADDR_WIDTH = 16;
parameter FNAME = "";

output[DATA_WIDTH-1:0] dataa;
input [DATA_WIDTH-1:0] datain;
input[ADDR_WIDTH-1:0] address;
input CS;
input WE;
input clk;

reg [DATA_WIDTH-1:0] ddd;

reg [DATA_WIDTH-1:0] dataa;
reg [DATA_WIDTH-1:0] RAM[0:(2**ADDR_WIDTH)-1];

always @(negedge(clk))
begin
    if (!CS) begin
        if (!WE) begin
            RAM[address] <= datain;
        end
        dataa = RAM[address];
    end
    else begin
        dataa = 8'hz;
    end
end

initial begin
    if (FNAME != "") begin
        $readmemh(FNAME, RAM);
    end
    
end

endmodule