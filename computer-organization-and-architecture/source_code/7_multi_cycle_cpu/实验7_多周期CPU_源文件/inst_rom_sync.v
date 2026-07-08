// Synchronous instruction ROM - behavioral replacement for NGC
// 256 words x 32 bits, initialized from test program, synchronous read
module inst_rom(
    input              clka,
    input       [7:0]  addra,
    output reg  [31:0] douta
);

    reg [31:0] mem [0:255];

    // Initialize test program (47 MIPS instructions from inst_rom.mif)
    integer i;
    initial begin
        for (i = 0; i < 256; i = i + 1) mem[i] = 32'd0;
        mem[0]  = 32'hac010000;
mem[1]  = 32'hac020004;
mem[2]  = 32'hac030008;
mem[3]  = 32'hac04000c;
mem[4]  = 32'hac050010;
mem[5]  = 32'hac060018;
mem[6]  = 32'hac070070;
mem[7]  = 32'hac190074;
mem[8]  = 32'hac0d0078;
mem[9]  = 32'h40017000;
mem[10] = 32'h24210004;
mem[11] = 32'h40817000;
mem[12] = 32'h42000018;
mem[13] = 32'h24010001;
mem[14] = 32'h00011100;
mem[15] = 32'h00411821;
mem[16] = 32'h00022082;
mem[17] = 32'h28990005;
mem[18] = 32'h0721000e;
mem[19] = 32'h00642823;
mem[20] = 32'hac050014;
mem[21] = 32'h00a23027;
mem[22] = 32'h00c33825;
mem[23] = 32'h00e64026;
mem[24] = 32'h11030002;
mem[25] = 32'hac08001c;
mem[26] = 32'h0022482a;
mem[27] = 32'h8c0a001c;
mem[28] = 32'h15450002;
mem[29] = 32'h00415824;
mem[30] = 32'hac0b001c;
mem[31] = 32'h0c000026;
mem[32] = 32'hac040010;
mem[33] = 32'h3c0c000c;
mem[34] = 32'h004cd007;
mem[35] = 32'h275b0044;
mem[36] = 32'h0360f809;
mem[37] = 32'h24010008;
mem[38] = 32'ha07a0005;
mem[39] = 32'h0143682b;
mem[40] = 32'h1da00002;
mem[41] = 32'h00867004;
mem[42] = 32'h000e7883;
mem[43] = 32'h002f8006;
mem[44] = 32'h1a000007;
mem[45] = 32'h002f8007;
mem[46] = 32'h06000006;
mem[47] = 32'h001a5900;
mem[48] = 32'h8d5c0003;
mem[49] = 32'h179d0007;
mem[50] = 32'ha0af0008;
mem[51] = 32'h80b20008;
mem[52] = 32'h90b30008;
mem[53] = 32'h2df8ffff;
mem[54] = 32'h0185e825;
mem[55] = 32'h01600008;
mem[56] = 32'h31f4ffff;
mem[57] = 32'h35f5ffff;
mem[58] = 32'h39f6ffff;
mem[59] = 32'h019d0018;
mem[60] = 32'h0000b812;
mem[61] = 32'h0000f010;
mem[62] = 32'h03400013;
mem[63] = 32'h03600011;
mem[64] = 32'h40807000;
mem[65] = 32'h0000000c;
mem[66] = 32'h40027000;
mem[67] = 32'h40036800;
mem[68] = 32'h40046000;
mem[69] = 32'h24010020;
mem[70] = 32'h01ee882a;
mem[71] = 32'h3c111234;
mem[72] = 32'h26315678;
mem[73] = 32'hac310000;
mem[74] = 32'h00118900;
mem[75] = 32'h1e20fffd;
mem[76] = 32'h24210004;
mem[77] = 32'h2402003c;
mem[78] = 32'h8c31ffe4;
mem[79] = 32'h00118902;
mem[80] = 32'hac510000;
mem[81] = 32'h1620fffd;
mem[82] = 32'h24420004;
mem[83] = 32'h24060044;
mem[84] = 32'h24070064;
mem[85] = 32'h8c23ffe4;
mem[86] = 32'h8c44fffc;
mem[87] = 32'h00642825;
mem[88] = 32'ha0e50000;
mem[89] = 32'h24e70001;
mem[90] = 32'h24210004;
mem[91] = 32'h1446fff9;
mem[92] = 32'h2442fffc;
mem[93] = 32'h24090064;
mem[94] = 32'h91290003;
mem[95] = 32'h240d0068;
mem[96] = 32'h8dad0000;
mem[97] = 32'h00094e00;
mem[98] = 32'h39ad0009;
mem[99] = 32'haced0001;
mem[100] = 32'h8c010000;
mem[101] = 32'h8c020004;
mem[102] = 32'h8c030008;
mem[103] = 32'h8c04000c;
mem[104] = 32'h8c050010;
mem[105] = 32'h8c060018;
mem[106] = 32'h8c070070;
mem[107] = 32'h8c190074;
mem[108] = 32'h8c0d0078;
mem[109] = 32'h0800000d;
    end

    // Synchronous read
    always @(posedge clka) begin
        douta <= mem[addra];
    end

endmodule