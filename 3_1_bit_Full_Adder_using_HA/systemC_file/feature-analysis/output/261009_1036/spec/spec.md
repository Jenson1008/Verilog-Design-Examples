# 候选功能规格

状态：待人工审核；此文档仅包含通过有界 Evidence Gate 的规则。

## `feature.full-adder.binary`

当 i_a、i_b、i_cin 均为二值输入时，稳定采样的 o_sum 等于三输入整数和的最低位，o_carry 等于该和的进位位。

适用范围：仅限 1 位顶层 full_half_add_1bit、输入值 0/1、全部 8 种组合，以及每组输入驱动后等待 1 ns 再采样。

### `rule.full-adder.truth-table`

对全部 8 种 i_a、i_b、i_cin 的二值组合，稳定采样的 o_sum 等于 i_a+i_b+i_cin 的最低位，o_carry 等于进位位。

证据状态：confirmed（仅限所列前置条件和观测范围）。

## 未确认内容

- 本 campaign 的 oracle 和覆盖域限定为二值输入；尽管接口使用 sc_logic，X/Z 输入的系统性行为不在本次确认范围内。

Feature IR 尚未人工 accept；未生成 promotion record。