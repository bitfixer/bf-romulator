module clockgen
    #(parameter CLKS_PER_CYCLE = 1)
(
    input fpga_clk,
    output wire clk_out
);

reg [7:0] clock_counter;
reg clkval;

assign clk_out = clkval;

always @(posedge fpga_clk)
begin
    clock_counter <= clock_counter + 1;
    if (clock_counter == CLKS_PER_CYCLE)
    begin
        clock_counter <= 0;
        if (clkval == 0)
        begin
            clkval <= 1;
        end
        else begin
            clkval <= 0;
        end
    end
end

endmodule