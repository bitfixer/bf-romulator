module input6502(
  inout spi_clk, 
  input wire spi_miso, 
  output spi_out, 
  output spi_cs,

  input[15:0] address, 
  inout [7:0] data, 
  input phi2, 
  input rwbar, 
  output dataoutenable, 
  output busenable,
  inout diag_spi_cs,
  output rdy,
  input rst,

  output wire led_blue
);

reg mreq = 1;

enable_logic romulator(
  spi_clk, 
  spi_miso, 
  spi_out, 
  spi_cs,

  address, 
  data, 
  phi2, 
  rwbar, 
  dataoutenable, 
  busenable,
  diag_spi_cs,
  rdy,
  1'b1,
  rst,

  led_blue,
  mreq
);

endmodule