module test(clk, data, cs, ka, kd, kj);

	input wire clk;
	input wire data;
	input wire cs;
	
	input wire [7:0] ka;
	output wire [4:0] kd;
	output reg [4:0] kj;

	reg [39:0] kbd;
	
	reg [12:0] tr;
	
	wire [2:0] id;
	wire [9:0] dd;
	assign id = ~tr[12:10];
	assign dd = tr[9:0];

	always @(negedge clk)
		if (~cs)
			tr <= { tr[11:0], ~data };
	
	always @(posedge cs)
		case (id)
			3'h0 : kbd[9:0] <= dd;
			3'h1 : kbd[19:10] <= dd;
			3'h2 : kbd[29:20] <= dd;
			3'h3 : kbd[39:30] <= dd;
			3'h4 : kj <= ~dd[4:0];
			3'h5 : begin kbd[0] = dd[0]; kbd[36] = dd[1]; end
	    endcase
	
	wire kd0, kd1, kd2, kd3, kd4;

	assign kd0 = (kbd[0] | ka[0]) & (kbd[5] | ka[1]) & (kbd[10] | ka[2]) & (kbd[15] | ka[3]) & (kbd[20] | ka[4]) & (kbd[25] | ka[5]) & (kbd[30] | ka[6]) & (kbd[35] | ka[7]);
	assign kd1 = (kbd[1] | ka[0]) & (kbd[6] | ka[1]) & (kbd[11] | ka[2]) & (kbd[16] | ka[3]) & (kbd[21] | ka[4]) & (kbd[26] | ka[5]) & (kbd[31] | ka[6]) & (kbd[36] | ka[7]);
	assign kd2 = (kbd[2] | ka[0]) & (kbd[7] | ka[1]) & (kbd[12] | ka[2]) & (kbd[17] | ka[3]) & (kbd[22] | ka[4]) & (kbd[27] | ka[5]) & (kbd[32] | ka[6]) & (kbd[37] | ka[7]);
	assign kd3 = (kbd[3] | ka[0]) & (kbd[8] | ka[1]) & (kbd[13] | ka[2]) & (kbd[18] | ka[3]) & (kbd[23] | ka[4]) & (kbd[28] | ka[5]) & (kbd[33] | ka[6]) & (kbd[38] | ka[7]);
	assign kd4 = (kbd[4] | ka[0]) & (kbd[9] | ka[1]) & (kbd[14] | ka[2]) & (kbd[19] | ka[3]) & (kbd[24] | ka[4]) & (kbd[29] | ka[5]) & (kbd[34] | ka[6]) & (kbd[39] | ka[7]);

	assign kd[0] = kd0 ? 1'bZ : 1'b0;
	assign kd[1] = kd1 ? 1'bZ : 1'b0;
	assign kd[2] = kd2 ? 1'bZ : 1'b0;
	assign kd[3] = kd3 ? 1'bZ : 1'b0;
	assign kd[4] = kd4 ? 1'bZ : 1'b0;
endmodule
