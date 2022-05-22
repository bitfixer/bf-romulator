module z80_input(
  inout spi_clk, 
  input wire spi_miso, 
  output spi_out, 
  output spi_cs,

  input[15:0] address, 
  inout [7:0] data, 
  input rd, 
  input wr, 
  output dataoutenable, 
  output busenable,
  inout diag_spi_cs,
  output rwait,
  input bwait,
  input m1,

  output wire led_blue,
  input wire mreq,
  input wire ioreq);

wire rwbar;
wire mem_access_active;

assign rwbar = wr;
assign mem_access_active = !wr || !rd;

enable_logic romulator(
    spi_clk,
    spi_miso,
    spi_out,
    spi_cs,

    address,
    data,
    rwbar,
    mem_access_active,
    dataoutenable,
    busenable,
    diag_spi_cs,
    rwait,
    led_blue,
    mreq);


endmodule