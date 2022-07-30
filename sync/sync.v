module sync(in_hs, in_vs, out_sync, delay_in, delay_out,
		centronix, strobe, busy,
		ss, 
		sck5, sck3, usb_ss5, usb_ss3, usb_miso3, mosi5, mosi3, miso5, ures5, ures3,
		led,
		rst
	);

	input wire in_hs;
	input wire in_vs;
	output wire out_sync;

	output wire delay_out;
	input wire delay_in;

	input wire [7:0] centronix;
	input wire strobe;
	output reg busy;

	input wire ss;
	wire sck, miso, mosi;
	assign sck = sck5;
	assign mosi = mosi5;

	input wire sck5, usb_ss5, mosi5, usb_miso3, ures5;
	output wire sck3, usb_ss3, mosi3, ures3;
	output wire miso5;

	output reg led;
	input wire rst;

	assign sck3 = sck5;
	assign usb_ss3 = usb_ss5;
	assign mosi3 = mosi5;
	assign ures3 = ures5;

	wire [7:0] received;

	wire ss_valid;
	assign ss_valid = ss ^ usb_ss5;

	assign miso5 = (ss_valid & rst) ? ((usb_miso3 & ~usb_ss5) | (miso & ~ss)) : 1'bZ;

	// CNTRNX
	reg cn_inv_strobe;
	reg cn_inv_busy;
	//reg capt_busy;
	reg beg, inb;

	initial cn_inv_strobe <= 0;
	initial cn_inv_busy <= 0;
	initial busy <= 0;

	wire inner_strobe;
	assign inner_strobe = strobe ^ cn_inv_strobe;

	wire reset_busy;
	assign reset_busy = received[3];

	always @(posedge inner_strobe, posedge ss) begin
		if (inner_strobe)
			busy <= ~cn_inv_busy;
		else if (reset_busy)
			busy <= cn_inv_busy;
	end

	// SPI
	reg [7:0] shiftreg;

	initial shiftreg = 0;
	initial inb = 0;

	always @(negedge sck)
		if (~ss)
			inb <= mosi;

	always @(negedge ss, posedge sck)
		if (sck)
			beg <= 1'b0;
		else
			beg <= ~ss;
	
	always @(posedge sck) if (~ss) begin
		if (beg)
			shiftreg <= centronix;
		else
			shiftreg <= { shiftreg[6:0], inb };
	end

	assign miso = shiftreg[7];
	assign received = { shiftreg[6:0], inb };

	always @(posedge ss) begin
		cn_inv_strobe <= received[0];
		cn_inv_busy <= received[1];
		led <= received[2];
	end

	// SYNC REGEN
	wire inner_vs;
	reg inner_hs;
	reg [1:0] cntr;
	reg prolong;

	always @(negedge in_hs) begin
		if (in_vs)
			cntr <= 2'b00;
		else begin
			if (~&cntr)
				cntr <= cntr + 2'b01;
		end
	end

	always @(posedge in_hs)
		prolong <= ^cntr;

	//always @(negedge inner_hs) begin
	//	inner_vs <= ~in_vs;
	//end	

	assign inner_vs = ^cntr;

	always @(negedge in_hs, posedge delay_in)
		if (delay_in)
			inner_hs <= 1'b1;
		else
			inner_hs <= 1'b0;

	assign out_sync = (inner_vs | prolong) ^ inner_hs;
	assign delay_out = in_hs;
		
endmodule
