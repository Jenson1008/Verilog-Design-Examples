# 功能规格

以下规则仅在明确列出的前置条件和测试范围内通过证据门禁（Evidence Gate）确认。

## `feature.half_adder.binary`

对于每一组二态输入 i_a、i_b ∈ {0,1}，施加该输入并等待 1 ns 稳定后，o_sum 等于 i_a XOR i_b，o_carry 等于 i_a AND i_b。

适用范围：仅涵盖 half_adder 顶层输出在四种二态输入组合下的行为；每组输入施加后 1 ns 采样输出。

### `rule.half_adder.binary`

对于每一组二态输入 (i_a, i_b)，在施加后 1 ns 采样时，o_sum 等于 i_a XOR i_b，o_carry 等于 i_a AND i_b。

证据状态：confirmed（仅限所列前置条件和观测范围）。
证据：[`evidence.half_adder.binary.truth_table`](../evidence/evidence-manifest.json)

## 未确认内容

- 精确的 delta-cycle 更新次数和延迟尚未测量；本次 campaign 在施加每组二态输入后 1 ns 采样输出。
