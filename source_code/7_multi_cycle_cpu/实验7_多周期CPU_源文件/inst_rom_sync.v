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
        mem[0]  = 32'h24010001;
        mem[1]  = 32'h00011100;
        mem[2]  = 32'h00411821;
        mem[3]  = 32'h00022082;
        mem[4]  = 32'h28990005;
        mem[5]  = 32'h07210010;
        mem[6]  = 32'h00642823;
        mem[7]  = 32'hac050014;
        mem[8]  = 32'h00a23027;
        mem[9]  = 32'h00c33825;
        mem[10] = 32'h00e64026;
        mem[11] = 32'hac08001c;
        mem[12] = 32'h11030002;
        mem[13] = 32'h00c7482a;
        mem[14] = 32'h24010008;
        mem[15] = 32'h8c2a0014;
        mem[16] = 32'h15450004;
        mem[17] = 32'h00415824;
        mem[18] = 32'hac2b001c;
        mem[19] = 32'hac240010;
        mem[20] = 32'h0c000019;
        mem[21] = 32'h3c0c000c;
        mem[22] = 32'h004cd007;
        mem[23] = 32'h003ad804;
        mem[24] = 32'h0360f809;
        mem[25] = 32'ha07a0005;
        mem[26] = 32'h0063682b;
        mem[27] = 32'h1da00003;
        mem[28] = 32'h00867004;
        mem[29] = 32'h000e7883;
        mem[30] = 32'h002f8006;
        mem[31] = 32'h1a000008;
        mem[32] = 32'h002f8007;
        mem[33] = 32'h240b008c;
        mem[34] = 32'h06000006;
        mem[35] = 32'h8d5c0003;
        mem[36] = 32'h179d0007;
        mem[37] = 32'ha0af0008;
        mem[38] = 32'h80b20008;
        mem[39] = 32'h90b30008;
        mem[40] = 32'h2df8ffff;
        mem[41] = 32'h0185e825;
        mem[42] = 32'h01600008;
        mem[43] = 32'h31f4ffff;
        mem[44] = 32'h35f5ffff;
        mem[45] = 32'h39f6ffff;
        mem[46] = 32'h08000000;
    end

    // Synchronous read
    always @(posedge clka) begin
        douta <= mem[addra];
    end

endmodule
