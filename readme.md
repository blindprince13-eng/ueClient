# 🎮 UE Multiplayer Combat & Notification Framework

这是一个基于 Unreal Engine 开发的高性能多人对战框架，集成了完善的战斗逻辑、服务器同步机制、AI 感知系统，以及一个支持深度嵌套的 **C++ 红点通知子系统**。

---

## 🚀 核心系统模块

### 1. 核心战斗与生命值管理 (Combat & Health)
项目实现了严谨的 **Server-Authoritative (服务器授权)** 战斗逻辑：
* **伤害处理：** 通过 `Event AnyDamage` 统一处理，由服务器判断权限 (`Switch Has Authority`)，确保数值安全性。
* **死亡表现：** 集成了 **Ragdoll (物理布娃娃)** 系统，死亡时自动开启物理模拟并卸载控制权 (`Un Possess`)。
* **状态同步：** 实时更新 `WBP_EnemyHealth` 进度条，并根据胜负状态动态弹出结算界面。

### 2. 多人联机与射击机制 (Networking & Weaponry)
* **RPC 射击逻辑：** 使用 `Server_Fire` 自定义事件进行客户端到服务器的射线检测 (`Line Trace`) 请求。
* **动态装备：** 在 `BeginPlay` 时通过服务器生成武器并自动附加 (`Attach`) 到角色的 `GripPoint` 插槽。
* **位置校验：** 射线检测起点严格绑定于武器的 `Muzzle` Socket，确保视觉与逻辑统一。

### 3. AI 感知与行为 (AI System)
* **视觉感知：** 利用 `Pawn Sensing` 组件实现 AI 发现玩家的功能。
* **行为逻辑：** 包含自动寻路 (`AIMoveTo`)、旋转对准玩家及执行 `AI_Fire` 的完整闭环。
* **自动重启：** 在 GameMode 中通过 `OnPostLogin` 逻辑，支持自动清理旧 Actor 并重新生成玩家。

### 4. 后端接口集成 (VaRest & Heartbeat)
项目通过 **VaRest 插件** 实现了轻量级的后端通信：
* **心跳包机制：** 通过计时器周期性发送 JSON 数据包（包含用户名与地址）。
* **动态红点拉取：** 异步请求 `red_dots` 列表，并将返回的 JSON 数组解析为游戏内逻辑节点。

---

## 🔴 特色功能：红点子系统 (C++ Red Dot Subsystem)

项目内置了一个纯 C++ 编写的 `URedDotSubsystem`，用于处理复杂的 UI 提示逻辑。

### 设计亮点
* **树形结构：** 支持父子节点绑定 (`RegisterNode`)，子节点更新会自动引起父节点“冒泡”变化。
* **冒泡更新 (Bubble-up)：** 自动计算增量 (`Delta`)，递归更新所有层级的父节点计数，无需手动遍历。
* **解耦设计：** 采用 `FOnRedDotChanged` 动态多播委托，UI 仅在数值变化时被动刷新，极佳地优化了性能。

### 核心代码参考 (C++)
#### 接口定义 (`.h`)
```cpp
// 注册节点关系 (构建树)
UFUNCTION(BlueprintCallable, Category = "RedDot System")
void RegisterNode(FString ChildKey, FString ParentKey);

// 设置节点值 (自动触发冒泡逻辑)
UFUNCTION(BlueprintCallable, Category = "RedDot System")
void SetNodeValue(FString NodeKey, int32 NewValue);

// 红点变化通知委托
UPROPERTY(BlueprintAssignable, Category = "RedDot System")
FOnRedDotChanged OnRedDotChanged;
