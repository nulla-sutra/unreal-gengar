# ADR-0001：两端本地 ActorInfo 初始化的蓝图入口

- 日期：2026-10-13。
- 状态：已接受；维护者已构建加载，Wukong 蓝图初始化接入完成。
- 范围：GengarAbilityLibrary 的 InitAbilityActorInfo 和项目角色蓝图的生命周期接入。

## 决策

复用现有 InitAbilityActorInfo，删除 BlueprintAuthorityOnly。原生 ASC 的该 API 建立本地 Owner、Avatar 和控制器等引用，不是授予能力或权威游戏状态修改。服务器和拥有客户端分别初始化自己的 ActorInfo；该缓存不通过 ASC 的状态复制自动保持控制器更新。

调用前检查 ASC 和 Owner 有效，避免空参数进入原生初始化断言。保留函数签名、Avatar 可为空及可选 PlayerController 覆盖的既有行为。项目角色不需要指定 PlayerController 时留空，由原生 GAS 从 Owner 的 Pawn/Controller 关系取得当前控制器。

PCharacter 保持与 GAS 解耦。PCharacter_Wukong 蓝图在 BeginPlay 初始化 Owner=Self、Avatar=Self，服务器分支继续 GiveAbility；原生 ReceiveControllerChanged 事件在服务器和拥有客户端再次调用同一 API。控制器变化会更新本地缓存，不增加初始化 RPC，也不在 JumpAbility 中刷新 ASC。

## 诊断与验证

现有联机 PIE 的服务器和拥有客户端均已收到 JumpAbility，但 ActorInfo.PlayerController 为空。通过正常 Enhanced Input 游戏帧注入，双方均进入 State.Jump.Preparing，客户端 TargetActor 开启 Tick，但 PrimaryPC 为空；项目取点函数因此返回失败。

第二轮诊断采样约 3 分 41 秒，未录制 Trace。无有效目标的确认取消后，两端准备 Tag 清空，TargetActor 销毁，服务器能力不再活动。仅在该 PIE 中临时为 TargetActor 指定 Pawn 当前控制器后，正常输入释放触发原生目标提交，两端达到相同 Landed 计划和进度 1，准备 Tag 清空、TargetActor 销毁。临时数据未写入资产，Agent 启动的 PIE 已停止。

另一次编辑器 Python 直接调用客户端预测激活引发 ServerTryActivateAbility 本地递归。引擎 Python 包装的 FEditorScriptExecutionGuard 会令 Actor RPC 使用本地调用空间，因此此诊断方式不用于验证网络能力激活；后续输入注入仅排队数值，能力在正常游戏帧中执行。该诊断错误不通过修改引擎或放宽预测键断言处理。

维护者已完整构建并重启。Wukong 蓝图接入 BeginPlay 和原生 Controller Changed，Controller 参数留空，Owner/Avatar 为 Self；初始化之后的 GiveAbility 仍受 HasAuthority 约束。蓝图编译保存为 0 错误、0 警告。后续联机 PIE 中，两端 ActorInfo.PlayerController 均与各自 Pawn.Controller 一致，正常输入可进入准备；debug 开关关闭时无跳跃轨迹，开启时恢复绿色轨迹和落点胶囊。

这轮约 73 秒采样在退出 PIE 时发现原生 TargetActor 解绑依赖当前 ActorInfo，触发 GenericDelegateBoundASC == UnboundASC 检查。修复位于项目 AsuraJumpTargetActor.EndPlay，使用原生绑定记录清理自身委托，不改变 Gengar、PCharacter 或跳跃同步。退出清理修复待维护者构建后复测，永久接入后的确认、取消和重复激活仍待验证。Agent 未执行 C++ 构建，未录制 Trace。
