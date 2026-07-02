`timescale 1ns / 1ps
//*************************************************************************
//   > 文件名: multi_cycle_cpu.v
//   > 描述  :多周期CPU模块，共实现36条指令
//   >        指令rom和数据ram采用行为级同步RAM，自带上电清零
//   > 作者  : LOONGSON
//   > 日期  : 2016-04-14
//*************************************************************************
module multi_cycle_cpu(  // 多周期cpu
    input clk,           // 时钟
    input resetn,        // 复位信号，低电平有效
    
    //display data
    input  [ 4:0] rf_addr,
    input  [31:0] mem_addr,
    output [31:0] rf_data,
    output [31:0] mem_data,
    output [31:0] IF_pc,
    output [31:0] IF_inst,
    output [31:0] ID_pc,
    output [31:0] EXE_pc,
    output [31:0] MEM_pc,
    output [31:0] WB_pc,
    output [31:0] display_state
);
//----------------------{控制多周期的状态机}begin------------------------//
    reg [2:0] state;       // 当前状态
    reg [2:0] next_state;  // 下一状态

    //(a) 声明RAM清零完成标志线网
    wire ram_clear_done;

    //展示当前处理器正在执行哪个模块
    assign display_state = {29'd0,state};
    // 状态机状态
    parameter IDLE   = 3'd0;  // 开始
    parameter FETCH  = 3'd1;  // 取指
    parameter DECODE = 3'd2;  // 译码
    parameter EXE    = 3'd3;  // 执行
    parameter MEM    = 3'd4;  // 访存
    parameter WB     = 3'd5;  // 写回

    always @ (posedge clk)        // 当前状态
    begin
        if (!resetn) begin        // 如果复位信号有效
            state <= IDLE;       // 当前状态为 开始
        end
        else begin                // 否则
            state <= next_state;  // 为下一状态
        end
    end

    wire IF_over;     // IF模块已执行完
    wire ID_over;     // ID模块已执行完
    wire EXE_over;    // EXE模块已执行完
    wire MEM_over;    // MEM模块已执行完
    wire WB_over;     // WB模块已执行完
    wire jbr_not_link;//分支指令(非link类)，只走IF和ID级
    always @ (*)                             // 下一状态 
    begin
        case (state)
            IDLE : 
            begin
                //(c) RAM未清零完成则停留在IDLE，清零完毕才进入取指
                next_state = ram_clear_done ? FETCH : IDLE;    
            end
            FETCH: 
            begin
                if (IF_over)
                begin
                    next_state = DECODE;   // 取指->译码
                end
                else
                begin
                    next_state = FETCH;    // 保持取指
                end
            end
            DECODE: 
            begin
                if (ID_over)
                begin                      // 译码->执行或取指   
                    next_state = jbr_not_link ? FETCH : EXE;
                end
                else
                begin
                    next_state = DECODE;   // 保持译码
                end
            end
            EXE: 
            begin
                if (EXE_over)
                begin
                    next_state = MEM;      // 执行->访存
                end
                else
                begin
                    next_state = EXE;   // 保持执行
                end
            end
            MEM:
            begin
                if (MEM_over)
                begin
                    next_state = WB;       // 访存->写回
                end
                else
                begin
                    next_state = MEM;   // 保持访存
                end
            end
            WB:
            begin
                if (WB_over)
                begin
                    next_state = FETCH;    // 写回->取指
                end
                else
                begin
                    next_state = WB;   // 保持写回
                end
            end
            default : next_state = IDLE;
        endcase
    end
    //5模块的valid信号
    wire IF_valid;
    wire ID_valid;
    wire EXE_valid;
    wire MEM_valid;
    wire WB_valid;
    assign  IF_valid = (state == FETCH );  // 当前状态为取指时，IF级有效
    assign  ID_valid = (state == DECODE);  // 当前状态为译码时，ID级有效
    assign EXE_valid = (state == EXE   );  // 当前状态为执行时，EXE级有效
    assign MEM_valid = (state == MEM   );  // 当前状态为访存时，MEM级有效
    assign  WB_valid = (state == WB    );  // 当前状态为写回时，WB级有效
//-----------------------{控制多周期的状态机}end-------------------------//

//--------------------------{5级间的总线}begin---------------------------//
    wire [ 63:0] IF_ID_bus;   // IF->ID级总线
    wire [149:0] ID_EXE_bus;  // ID->EXE级总线
    wire [105:0] EXE_MEM_bus; // EXE->MEM级总线
    wire [ 69:0] MEM_WB_bus;  // MEM->WB级总线
    
    //锁存以上总线信号
    reg [ 63:0] IF_ID_bus_r;
    reg [149:0] ID_EXE_bus_r;
    reg [105:0] EXE_MEM_bus_r;
    reg [ 69:0] MEM_WB_bus_r;
    
    //(b) 合并4段锁存逻辑为单个always块，增加复位清零
    always @(posedge clk)
    begin
        if(!resetn) begin
            IF_ID_bus_r    <= 64'd0;
            ID_EXE_bus_r   <= 150'd0;
            EXE_MEM_bus_r  <= 106'd0;
            MEM_WB_bus_r   <= 70'd0;
        end
        else begin
            if(IF_over)
                IF_ID_bus_r <= IF_ID_bus;
            if(ID_over)
                ID_EXE_bus_r <= ID_EXE_bus;
            if(EXE_over)
                EXE_MEM_bus_r <= EXE_MEM_bus;
            if(MEM_over)
                MEM_WB_bus_r <= MEM_WB_bus;
        end
    end
//---------------------------{5级间的总线}end----------------------------//

//--------------------------{其他交互信号}begin--------------------------//
    //跳转总线
    wire [ 32:0] jbr_bus;    

    //IF与inst_rom交互
    wire [31:0] inst_addr;
    wire [31:0] inst;

    //MEM与data_ram_sync交互    
    wire [ 3:0] dm_wen;
    wire [31:0] dm_addr;
    wire [31:0] dm_wdata;
    wire [31:0] dm_rdata;

    //ID与regfile交互
    wire [ 4:0] rs;
    wire [ 4:0] rt;   
    wire [31:0] rs_value;
    wire [31:0] rt_value;
    
    //WB与regfile交互
    wire        rf_wen;
    wire [ 4:0] rf_wdest;
    wire [31:0] rf_wdata;    
//---------------------------{其他交互信号}end---------------------------//

//-------------------------{各模块实例化}begin---------------------------//
    wire next_fetch; //即将运行取指模块，需要先锁存PC值
    assign next_fetch = (state==DECODE & ID_over & jbr_not_link)
                      | (state==WB     & WB_over);
    fetch IF_module(             // 取指级
        .clk       (clk       ),
        .resetn    (resetn    ),
        .IF_valid  (IF_valid  ),
        .next_fetch(next_fetch),
        .inst      (inst      ),
        .jbr_bus   (jbr_bus   ),
        .inst_addr (inst_addr ),
        .IF_over   (IF_over   ),
        .IF_ID_bus (IF_ID_bus ),
        .IF_pc     (IF_pc     ),
        .IF_inst   (IF_inst   )
    );

    decode ID_module(               // 译码级
        .ID_valid    (ID_valid    ),
        .IF_ID_bus_r (IF_ID_bus_r ),
        .rs_value    (rs_value    ),
        .rt_value    (rt_value    ),
        .rs          (rs          ),
        .rt          (rt          ),
        .jbr_bus     (jbr_bus     ),
        .jbr_not_link(jbr_not_link),
        .ID_over     (ID_over     ),
        .ID_EXE_bus  (ID_EXE_bus  ),
        .ID_pc      (ID_pc      )
    );

    exe EXE_module(                   // 执行级
        .EXE_valid   (EXE_valid   ),
        .ID_EXE_bus_r(ID_EXE_bus_r),
        .EXE_over    (EXE_over    ),
        .EXE_MEM_bus (EXE_MEM_bus ),
        .EXE_pc      (EXE_pc      )
    );

    mem MEM_module(                     // 访存级 (f) 添加resetn端口
        .clk          (clk          ),
        .resetn       (resetn      ),
        .MEM_valid    (MEM_valid    ),
        .EXE_MEM_bus_r(EXE_MEM_bus_r),
        .dm_rdata     (dm_rdata     ),
        .dm_addr      (dm_addr      ),
        .dm_wen       (dm_wen       ),
        .dm_wdata     (dm_wdata     ),
        .MEM_over     (MEM_over     ),
        .MEM_WB_bus   (MEM_WB_bus   ),
        .MEM_pc       (MEM_pc       )
    );          
 
    wb WB_module(                     // 写回级
        .WB_valid    (WB_valid    ),
        .MEM_WB_bus_r(MEM_WB_bus_r),
        .rf_wen      (rf_wen      ),
        .rf_wdest    (rf_wdest    ),
        .rf_wdata    (rf_wdata    ),
        .WB_over     (WB_over     ),
        .WB_pc       (WB_pc       )
    );

    inst_rom inst_rom_module(         // 指令存储器IP
        .clka       (clk           ),
        .addra      (inst_addr[9:2]),
        .douta      (inst          )
    );

    regfile rf_module(        // 寄存器堆 (d) 添加resetn端口
        .clk    (clk      ),
        .resetn (resetn   ),
        
        .wen    (rf_wen   ),
        .raddr1 (rs       ),
        .raddr2 (rt       ),
        .waddr  (rf_wdest ),
        .wdata  (rf_wdata ),
        .rdata1 (rs_value ),
        .rdata2 (rt_value ),

        //display rf
        .test_addr(rf_addr),
        .test_data(rf_data)
    );
    
    // 核心修改：实例化 data_ram_sync 而非IP的data_ram，端口完全匹配
    data_ram_sync data_ram_module(   // 带自动清零的行为数据RAM
        .clka        (clk           ),
        .resetn      (resetn        ),
        .ram_clear_done(ram_clear_done),
        .wea         (dm_wen        ),
        .addra       (dm_addr[9:2]  ),
        .dina        (dm_wdata      ),
        .douta       (dm_rdata     ),

        //display mem 读口B
        .clkb   (clk          ),
        .web    (4'd0         ),
        .addrb  (mem_addr[9:2]),
        .doutb  (mem_data     ),
        .dinb   (32'd0        )
    );
//--------------------------{各模块实例化}end----------------------------//
endmodule