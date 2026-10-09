# 3-bit Up/Down Counter (SystemC)

本目录将原 Verilog `cntr_3ud` 建模为 SystemC 周期级模块。DUT、激励与仿真顶层分开放置；文件名直接使用模块名和普通 `.h`/`.cpp` 扩展名，不用 `_sc` 作为区分后缀。

RTL 行为是 3 位模 8 计数器：在 Verilog 正边沿采样高有效同步复位，复位为 `1` 时清零；否则过程式 `if (up_down)` 在 `up_down = 1` 时减一，在 `0`、`X` 或 `Z` 时走 `else` 分支加一。SystemC 控制端口使用 `sc_logic`，计数输出使用 `sc_lv<3>`，保留初值及控制信号的四态行为。

## 目录

- `DUT/cntr_3ud.h`：计数器模块。
- `stimulus/cntr_3ud_stimulus.h`：独立激励、参考模型、自检和 CSV 风格日志。
- `stimulus/cntr_3ud_campaign_test.cpp`：SystemC-to-Spec campaign 使用的 20-vector 二值定向测试。
- `cntr_3ud_tb.cpp`：`sc_main`、顶层信号连接和 VCD 配置。
- `interface/interface.json`：从 DUT 端口声明整理的接口契约。
- `feature-analysis/input/`：设计摘要和接口快照。
- `out/`：构建可执行文件、日志和波形。

## 构建与仿真

需要 C++17 编译器、GNU Make 和 SystemC 开发库。从本目录运行：

```sh
make run
```

可用 `CXX` 和 `SYSTEMC_HOME` 覆盖编译器及 SystemC 安装前缀，也可用 `OUT_DIR` 指定输出目录，例如 `make OUT_DIR=out/run-YYMMDD_HHmm run`，以避免覆盖已有日志、波形或可执行文件。`make run` 执行 32 个确定性自检点，成功时以零退出并在日志中写入 `# SUMMARY: PASS`。它覆盖初值、同步复位、加减回绕、时钟边沿、未知复位及未知方向的代表性行为；这不是所有四态序列的穷举，也不是形式等价证明。

当前生成文件为 `out/cntr_3ud_tb`、`out/cntr_3ud_build.log`、`out/cntr_3ud.log` 和 `out/cntr_3ud_wave.vcd`。仿真日志是每次运行覆盖的单次结果。`make clean` 只移除这四个新命名的生成文件，不清除目录中的其他既有产物。

原 Verilog testbench 在 25 ns 的正边沿撤销复位，并在采样边沿驱动 `up_down`，存在调度竞态；本确定性 SystemC 激励在时钟沿之前驱动输入，不复现该随机序列。目录中原有的 `_sc` 命名输出产物已保留且不会被新构建覆盖。

## SystemC-to-Spec

`feature-analysis/input/campaign-config.json`、静态事实、候选 feature 和 test catalog 为 Harness 的显式调用方输入。当前 Harness MVP 是确定性执行切片，不会调用 LLM Agent 自动发现这些内容。campaign 使用独立的二值 20-vector testbench；X/Z、初始复位前状态及未列入该向量序列的时序仍作为 unknown。campaign 最终存放在 `feature-analysis/output/<YYMMDD_HHmm>/`，目录名使用本地时间；Harness 生成的 `<campaign_id>` 目录通过校验后必须重命名为该格式，且重命名后再次校验。重名时停止，不覆盖已有目录。输出停在人工 validation，不自动 finalize 或 promote。
