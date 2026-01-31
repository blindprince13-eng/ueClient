#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RedDotSubsystem.generated.h" // 这个必须是最后一个include

// 声明委托：当某节点的红点数发生变化时广播
// 参数：NodeKey (节点名), NewCount (新的数量)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRedDotChanged, FString, NodeKey, int32, NewCount);

/**
 * 红点系统子系统
 */
UCLASS()
class MULTIPLAYERFPSDEMO_API URedDotSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// --- 供蓝图调用的 API ---

	// 注册节点关系 (构建树)
	UFUNCTION(BlueprintCallable, Category = "RedDot System")
	void RegisterNode(FString ChildKey, FString ParentKey);

	// 设置节点的红点数量 (通常用于叶子节点)
	UFUNCTION(BlueprintCallable, Category = "RedDot System")
	void SetNodeValue(FString NodeKey, int32 NewValue);

	// 获取某个节点的当前值
	UFUNCTION(BlueprintPure, Category = "RedDot System")
	int32 GetNodeValue(FString NodeKey) const;

	// --- 事件分发器 ---
	// UI 在 Construct 时绑定这个事件
	UPROPERTY(BlueprintAssignable, Category = "RedDot System")
	FOnRedDotChanged OnRedDotChanged;

private:
	// 递归/冒泡更新父节点
	void UpdateParentNode(FString NodeKey, int32 Delta);

private:
	// 存储树结构：Key=子节点, Value=父节点
	TMap<FString, FString> ParentMap;

	// 存储当前计数：Key=节点, Value=数量
	TMap<FString, int32> NodeCounts;
};