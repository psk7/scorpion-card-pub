module ext(
	miso, mosi, sck, ss_n, intr,
	a0, a1, a2, a3, a4, a5, a6, a7, a8, a10,
	d,
	rd_n, wr_n, m1_n, dos, wait_n, iorq_n, iorqge
);

	input wire mosi, sck, ss_n;
	output wire miso, intr;

	input wire a0, a1, a2, a3, a4, a5, a6, a7, a8, a10;
	inout wire [7:0] d;

	input wire rd_n, wr_n, m1_n, dos, iorq_n;
	output wire iorqge, wait_n;

	reg [7:0] D;
	reg [7:0] fadf, fbdf, ffdf;
	reg direct_wait;

	wire [7:0] di;
	assign di = d;

	wire selport, sel_df, sel_fadf, sel_fbdf, sel_fedf, sel_ffdf;
	wire [7:0] A;
	wire sel_any;

	assign A = { a7, a6, a5, a4, a3, a2, a1, a0 };
	assign selport = m1_n & ~iorq_n;
	
	assign sel_df = (A[7:0] == 8'hDF) & dos;
	assign sel_fadf = sel_df & ~a10 & ~a8;
	assign sel_fbdf = sel_df & ~a10 & a8;
	assign sel_fedf = sel_df & a10 & ~a8;
	assign sel_ffdf = sel_df & a10 & a8;

	wire rd_fadf, rd_fbdf, rd_fedf, rd_ffdf, rd_any;
	assign rd_fadf = sel_fadf & ~rd_n & selport;
	assign rd_fbdf = sel_fbdf & ~rd_n & selport;
	assign rd_fedf = sel_fedf & ~rd_n & selport;
	assign rd_ffdf = sel_ffdf & ~rd_n & selport;

	assign sel_any = sel_df;

	assign iorqge = sel_any ? 1'b1 : 1'bZ;

	// SPI
	reg [15:0] shiftreg;
	reg beg, inb;
	wire [2:0] regnum;
	wire [15:0] received;

	assign received = { shiftreg[14:0], inb };
	assign regnum = received[10:8];
	assign miso = ~ss_n ? shiftreg[15] : 1'bZ;

	always @(negedge sck)
		if (~ss_n)
			inb <= mosi;

	always @(negedge ss_n, posedge sck)
		if (sck)
			beg <= 1'b0;
		else
			beg <= ~ss_n;
	
	always @(posedge sck) if (~ss_n) begin
		if (beg)
			shiftreg <= {8'hEE, di};
		else
			shiftreg <= { shiftreg[14:0], inb };
	end

	always @(posedge ss_n) begin
		case (regnum)
			3'h0: fadf <= received[7:0];
			3'h1: fbdf <= received[7:0];
			3'h2: ffdf <= received[7:0];
			3'h3: direct_wait = received[0];
		endcase
	end

	always @* begin
		if (rd_fadf)
			D = fadf ^ 8'hAE;
		else if (rd_fbdf)
			D = fbdf ^ 8'hEA;
		else if (rd_fedf)
			D = 8'h55;
		else if (rd_ffdf)
			D = ffdf ^ 8'h77;
		else 
			D = received[7:0];
	end

	assign d = selport & ~rd_n & sel_any ? D : 8'hZZ;

	assign wait_n = direct_wait ? 1'b0 : 1'bZ;

endmodule
