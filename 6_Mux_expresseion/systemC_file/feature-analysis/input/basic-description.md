# mux_always

## 设计行为

`mux_always` 是一个单比特二选一数据开关：`select` 为 0 时，`out` 取 `in0`；`select` 为 1 时，`out` 取 `in1`。这是持续组合行为，RTL 的表达式会随输入变化重新求值，SystemC 也把 `in0`、`in1` 和 `select` 都列入敏感列表，因此任一输入改变都会更新输出。
