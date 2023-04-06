module mouse(
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
	reg [2:0] fadf_b;
	reg [3:0] fadf_w;
	reg [7:0] fbdf, ffdf;

	wire selport, sel_df, sel_fadf, sel_fbdf, sel_ffdf;
	wire [7:0] A;
	wire any_read, any_write;

	assign A = { a7, a6, a5, a4, a3, a2, a1, a0 };
	assign selport = m1_n & ~iorq_n;
	
	// Kempston mouse selectors
	assign sel_df = (A[7:0] == 8'hDF) & dos;
	assign sel_fadf = sel_df & ~a10 & ~a8;
	assign sel_fbdf = sel_df & ~a10 & a8;
	assign sel_ffdf = sel_df & a10 & a8;

	wire rd_fadf, rd_fbdf, rd_fedf, rd_ffdf, rd_any;
	assign rd_fadf = sel_fadf & ~rd_n & selport;
	assign rd_fbdf = sel_fbdf & ~rd_n & selport;
	assign rd_ffdf = sel_ffdf & ~rd_n & selport;

	// Controller link selectors
	assign any_read = rd_fadf | rd_fbdf | rd_ffdf;

	assign iorqge = any_read ? 1'b1 : 1'bZ;

	// SPI
	reg [23:0] shiftreg;
	wire [2:0] regnum;
	wire [23:0] received;
	wire [9:0] tag;

	assign tag = { received[10], received[11], received[12], received[13], received[14], received[15], received[16], received[17], received[18], received[19] };
	assign received = shiftreg[23:0];
	assign regnum = received[9:8];
	assign miso = 1'bZ;
	assign isValid = ((received[19:10] == ~received[9:0]) && (received[23:20] == 4'b1001));

	always @(negedge sck)
		if (~ss_n) 
			shiftreg[0] <= mosi;
		else
			shiftreg[0] <= 1'b0;

	always @(posedge sck)
		if (~ss_n)
			shiftreg[23:1] <= shiftreg[22:0];
		else
			shiftreg[23:1] <= 23'b0;

	always @(posedge ss_n) if (isValid) begin
		case (regnum)
			2'h0: begin 
				fadf_b <= received[2:0];
				fadf_w <= received[7:4];
			end
			2'h1: fbdf <= received[7:0];
			2'h2: ffdf <= received[7:0];
		endcase
	end

	// Data bus output
	always @* begin
		if (rd_fadf)
			D = { fadf_w, 1'h1, fadf_b };
		else if (rd_fbdf)
			D = fbdf;
		else if (rd_ffdf)
			D = ffdf;
		else 
			D = received[7:0];
	end

	assign d = any_read ? D : 8'hZZ;

	assign wait_n = 1'bZ;
	assign intr = 1'bZ;
endmodule
