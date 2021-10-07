module simple_ram_dual_clock #(
  parameter DATA_WIDTH=8,                 //width of data bus
  parameter ADDR_WIDTH=8                  //width of addresses buses
)(
  input      [DATA_WIDTH-1:0] data,       //data to be written
  input      [ADDR_WIDTH-1:0] read_addr,  //address for read operation
  input      [ADDR_WIDTH-1:0] write_addr, //address for write operation
  input                       we,         //write enable signal
  input                       read_clk,   //clock signal for read operation
  input                       write_clk,  //clock signal for write operation
  output reg [DATA_WIDTH-1:0] q           //read data
);
    
  reg [DATA_WIDTH-1:0] ram [2**ADDR_WIDTH-1:0]; // ** is exponentiation
    
  always @(posedge write_clk) begin //WRITE
    if (we)
    begin 
      ram[write_addr] <= data;
    end
  end
    
  always @(posedge read_clk) begin //READ
    q <= ram[read_addr];
  end

initial
begin
  $readmemh("../bin/vram_test.txt", ram);
end

endmodule