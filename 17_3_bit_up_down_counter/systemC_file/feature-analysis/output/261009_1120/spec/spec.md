# 功能规格

以下规则仅在明确列出的前置条件和本次测试范围内得到 Evidence Gate 确认。

## `feature.modulo-counter-step`

在本次配置的二值定向序列中，时钟有效上升沿采样到 reset=1 时，bin_count 清零；否则，up_down=0 时计数值按模 8 递增，up_down=1 时按模 8 递减。

适用范围：仅限本 campaign 中明确列出的 20 个二值测试向量，以及输入建立和时钟边沿后采样各 1 ns 的时序。

### `rule.cntr3ud.bounded-step`

在本次 20 条定向测试序列中，上升沿采样到 reset=1 时输出 000；否则，up_down=0 时三位计数值按模 8 递增，up_down=1 时按模 8 递减。所有已配置的 bin_count 期望值均与观测值匹配。

证据状态：confirmed（仅限所列前置条件和观测范围）。

## 未确认内容

- 在第一个采样到 reset=1 的有效时钟沿之前，计数值未初始化。
- 本 campaign 的整数值测试请求未覆盖 clk、reset 和 up_down 的 X/Z 行为；此前 SystemC 自检中的代表性四态检查是独立测试，未作为本 campaign 的证据。
- 本 campaign 未复现或评估原 Verilog testbench 的随机激励及复位/输入与时钟沿同一时刻变化所产生的调度竞态。
- 未测试本 campaign 指定的 1 ns 建立时间和边沿后采样时序以外的时钟/复位时序。
