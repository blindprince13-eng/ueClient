#include "RedDotSubsystem.h"

void URedDotSubsystem::RegisterNode(FString ChildKey, FString ParentKey)
{
	if (ChildKey.IsEmpty() || ParentKey.IsEmpty()) return;

	// 记录父子关系
	ParentMap.Add(ChildKey, ParentKey);

	// 初始化计数（防止 Access None，默认为0）
	if (!NodeCounts.Contains(ChildKey)) NodeCounts.Add(ChildKey, 0);
	if (!NodeCounts.Contains(ParentKey)) NodeCounts.Add(ParentKey, 0);
}

void URedDotSubsystem::SetNodeValue(FString NodeKey, int32 NewValue)
{
	// 1. 获取旧值
	int32 OldValue = 0;
	if (int32* FoundVal = NodeCounts.Find(NodeKey))
	{
		OldValue = *FoundVal;
	}

	// 2. 如果值没变，直接返回（节省性能）
	if (OldValue == NewValue) return;

	// 3. 计算差值 (Delta)
	int32 Delta = NewValue - OldValue;

	// 4. 更新当前节点
	NodeCounts.Add(NodeKey, NewValue);

	// 5. 通知 UI
	OnRedDotChanged.Broadcast(NodeKey, NewValue);

	// 6. 处理冒泡：通知父节点加上这个差值
	UpdateParentNode(NodeKey, Delta);
}

void URedDotSubsystem::UpdateParentNode(FString NodeKey, int32 Delta)
{
	// 这里的逻辑是：如果我有父节点，父节点的值也要 += Delta
	// 并且父节点变了，父节点的父节点也要变... 一直到根节点

	FString CurrentNode = NodeKey;

	// 循环向上查找，直到没有父节点
	while (ParentMap.Contains(CurrentNode))
	{
		FString ParentKey = ParentMap[CurrentNode];

		// 获取父节点当前值
		int32 ParentOldValue = 0;
		if (int32* PVal = NodeCounts.Find(ParentKey))
		{
			ParentOldValue = *PVal;
		}

		// 计算父节点新值
		int32 ParentNewValue = ParentOldValue + Delta;

		// 更新父节点数据
		NodeCounts.Add(ParentKey, ParentNewValue);

		// 广播父节点变化 (这样监听 "ActivityCenter" 的 UI 就会收到通知)
		OnRedDotChanged.Broadcast(ParentKey, ParentNewValue);

		// 继续向上一层
		CurrentNode = ParentKey;
	}
}

int32 URedDotSubsystem::GetNodeValue(FString NodeKey) const
{
	if (const int32* Val = NodeCounts.Find(NodeKey))
	{
		return *Val;
	}
	return 0;
}