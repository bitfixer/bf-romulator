module enable_logic(
  inout spi_clk, 
  input wire spi_miso, 
  output spi_out, 
  output spi_cs,

  input[15:0] address, 
  inout [7:0] data, 
  input wr, 
  input rd, 
  output dataoutenable, 
  output busenable,
  input diag_spi_cs,
  output rwait,
  input m1,
  input bwait,

  output wire led_blue,
  input wire mreq,
  input wire ioreq
  );

// set up data bus
wire[7:0] wdatain;
wire[7:0] wdataout;

wire clk;
wire wdataout_enable;

assign wdataout_enable = !rd;

// set up internal clock
SB_HFOSC inthosc(.CLKHFPU(1'b1), .CLKHFEN(1'b1), .CLKHF(clk));

// set up bidirectional data bus. Data pins switch direction based on wdataout_enable signal.
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[0]
  (
    .PACKAGE_PIN(data[0]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[0]),
    .D_IN_0(wdatain[0])
  );
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[1]
  (
    .PACKAGE_PIN(data[1]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[1]),
    .D_IN_0(wdatain[1])
  );
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[2]
  (
    .PACKAGE_PIN(data[2]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[2]),
    .D_IN_0(wdatain[2])
  );
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[3]
  (
    .PACKAGE_PIN(data[3]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[3]),
    .D_IN_0(wdatain[3])
  );
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[4]
  (
    .PACKAGE_PIN(data[4]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[4]),
    .D_IN_0(wdatain[4])
  );
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[5]
  (
    .PACKAGE_PIN(data[5]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[5]),
    .D_IN_0(wdatain[5])
  );
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[6]
  (
    .PACKAGE_PIN(data[6]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[6]),
    .D_IN_0(wdatain[6])
  );
SB_IO #(
    .PIN_TYPE(6'b 1010_01), // PIN_OUTPUT_TRISTATE - PIN_INPUT
    .PULLUP(1'b 0)
  )
  iobuf_data[7]
  (
    .PACKAGE_PIN(data[7]),
    .OUTPUT_ENABLE(wdataout_enable),
    .D_OUT_0(wdataout[7]),
    .D_IN_0(wdatain[7])
  );

assign led_blue = 0;
assign rwait = bwait;

wire [15:0] ram_address;
wire ram_cs;
wire ram_we;
wire [7:0] ram_datain;
wire [7:0] ram_dataout;

assign ram_address = address;
assign wdataout = ram_dataout;
assign ram_datain = wdatain;

//assign ram_cs = (address >= 18432) && (address <= 18434) && (!wr || !rd);
//assign ram_cs = (address >= 18432 && address <= 18444) && !rd;
assign ram_cs = (address >= 16'h0100 && address < 16'ha000) && (!rd || !wr);

assign dataoutenable = !ram_cs;
assign busenable = !dataoutenable;

assign ram_we = !wr;
sram64k RAM(ram_address, ram_dataout, ram_datain, ram_cs, ram_we, clk);


endmodule