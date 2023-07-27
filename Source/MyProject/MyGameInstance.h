// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPatchCompleteDelegate, bool, Succeeded);

UCLASS()
class MYPROJECT_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public :
	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintPure, Category = "Patching|Stats")
		void GetLoadingProgress(int32& BytesDownloaded, int32& TotalBytesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const;

	// ��������Ʈ
	// ��ġ ���μ����� ���� �Ǵ� �����ϸ� �߻��մϴ�.
	UPROPERTY(BlueprintAssignable, Category = "Patching");
	FPatchCompleteDelegate OnPatchComplete;

	// ���� ��ġ ���μ����� �����մϴ�. ��ġ �Ŵ��佺Ʈ�� �ֽ� ���°� �ƴ� ��� false�� ��ȯ�մϴ�. */
	UFUNCTION(BlueprintCallable, Category = "Patching")
		bool PatchGame();

protected:
	bool bIsDownloadManifestUpToDate;

	void OnManifestUpdateComplete(bool bSuccess);

	// �õ� �� �ٿ�ε��� ûũ ID ���
	UPROPERTY(EditDefaultsOnly, Category = "Patching")
		TArray<int32> ChunkDownloadList;

	// ûũ �ٿ�ε� ���μ����� �Ϸ�Ǹ� ȣ��˴ϴ�.
	void OnDownloadComplete(bool bSuccess);

	// ChunkDownloader�� �ε� ��尡 �Ϸ�� ������ ȣ��˴ϴ�.
	void OnLoadingModeComplete(bool bSuccess);

	// ChunkDownloader�� ûũ ����Ʈ�� �Ϸ��ϸ� ȣ��˴ϴ�.
	void OnMountComplete(bool bSuccess);
};