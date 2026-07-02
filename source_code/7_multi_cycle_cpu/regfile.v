`timescale 1ns / 1ps
//*************************************************************************
//   > 文件名: regfile.v
//   > 描述  ：寄存器堆模块，同步写，异步读
//   > 作者  : LOONGSON
//   > 日期  : 2016-04-14
//*************************************************************************
module regfile(
    input             clk,
    input             resetn,
    input             wen,
    input      [4 :0] raddr1,
    input      [4 :0] raddr2,
    input      [4 :0] waddr,
    input      [31:0] wdata,
    output reg [31:0] rdata1,
    output reg [31:0] rdata2,
    input      [4 :0] test_addr,
    output reg [31:0] test_data
);
reg [31:0] rf[31:0];

// 同步复位（和你最初代码逻辑一致，仅clk上升沿触发复位/写）
generate
    genvar i;
    for(i = 0; i < 32; i = i + 1) begin : rf_write_loop
        always @(posedge clk) begin
            if (!resetn) begin
                rf[i] <= 32'd0;
            end else begin
                if (wen && (waddr == i)) begin
                    rf[i] <= wdata;
                end
            end
        end
    end
endgenerate

// 读端口1：组合逻辑，x0恒为0，其余直接数组寻址
always @(*) begin
    if (raddr1 == 5'd0)
        rdata1 = 32'd0;
    else
        rdata1 = rf[raddr1];
end

// 读端口2
always @(*) begin
    if (raddr2 == 5'd0)
        rdata2 = 32'd0;
    else
        rdata2 = rf[raddr2];
end

// 调试测试端口
always @(*) begin
    if (test_addr == 5'd0)
        test_data = 32'd0;
    else
        test_data = rf[test_addr];
end

endmodule