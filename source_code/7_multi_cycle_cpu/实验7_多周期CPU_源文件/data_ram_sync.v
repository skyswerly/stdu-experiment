`timescale 1ns/1ps
// Synchronous dual-port data RAM with auto-clear FSM
// 256 words x 32 bits, byte write enable, synchronous read
module data_ram_sync(
    input              clk,
    input              resetn,         // active-low
    input       [3:0]  wea,
    input       [7:0]  addra,
    input       [31:0] dina,
    output reg  [31:0] douta,
    input       [3:0]  web,
    input       [7:0]  addrb,
    input       [31:0] dinb,
    output reg  [31:0] doutb,
    output             ram_clear_done
);

reg [31:0] mem [0:255];

// Clear FSM registers
reg [7:0] clear_cnt;
reg       clearing;
reg       resetn_r;
assign ram_clear_done = !clearing;

// 复位同步+清零计数状态机（仅计数，不操作mem写）
always @(posedge clk or negedge resetn) begin
    if(!resetn) begin
        resetn_r <= 1'b0;
        clear_cnt <= 8'd0;
        clearing  <= 1'b1;
    end else begin
        resetn_r <= resetn;
        // 捕获复位下降沿，重新开始清零
        if(resetn_r && !resetn) begin
            clear_cnt <= 8'd0;
            clearing  <= 1'b1;
        end
        // 清零阶段地址自增
        if(clearing) begin
            clear_cnt <= clear_cnt + 8'd1;
            if(clear_cnt == 8'd255) clearing <= 1'b0;
        end
    end
end

// 唯一内存读写时序块：清零写 + A/B端口读写全部收拢在此
always @(posedge clk) begin
    // 同步读，先寄存输出，符合BRAM推断标准模板
    douta <= mem[addra];
    doutb <= mem[addrb];

    if(clearing) begin
        // 清零阶段：覆盖当前计数地址为0，屏蔽端口业务写
        mem[clear_cnt] <= 32'd0;
    end
    else begin
        // 正常工作：Port A 字节粒度写
        if(wea[0]) mem[addra][ 7: 0] <= dina[ 7: 0];
        if(wea[1]) mem[addra][15: 8] <= dina[15: 8];
        if(wea[2]) mem[addra][23:16] <= dina[23:16];
        if(wea[3]) mem[addra][31:24] <= dina[31:24];

        // 正常工作：Port B 字节粒度写
        if(web[0]) mem[addrb][ 7: 0] <= dinb[ 7: 0];
        if(web[1]) mem[addrb][15: 8] <= dinb[15: 8];
        if(web[2]) mem[addrb][23:16] <= dinb[23:16];
        if(web[3]) mem[addrb][31:24] <= dinb[31:24];
    end
end

endmodule