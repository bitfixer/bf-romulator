module reset_signal
    #(parameter CLKS_PER_RESET = 1)
(
    input fpga_clk,
    output wire reset_out
);

reg [31:0] clock_counter;
reg resetval;

assign reset_out = resetval;

always @(posedge fpga_clk)
begin
    if (resetval == 0)
    begin
        clock_counter <= clock_counter + 1;
        if (clock_counter == CLKS_PER_RESET)
        begin
            clock_counter <= 0;
            resetval <= 1;
        end
    end
end

initial
begin
  resetval <= 0;
end

endmodule