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

	// 델리게이트
	// 패치 프로세스가 성공 또는 실패하면 발생합니다.
	UPROPERTY(BlueprintAssignable, Category = "Patching");
	FPatchCompleteDelegate OnPatchComplete;

	// 게임 패치 프로세스를 시작합니다. 패치 매니페스트가 최신 상태가 아닌 경우 false를 반환합니다. */
	UFUNCTION(BlueprintCallable, Category = "Patching")
		bool PatchGame();

protected:
	bool bIsDownloadManifestUpToDate;

	void OnManifestUpdateComplete(bool bSuccess);

	// 시도 및 다운로드할 청크 ID 목록
	UPROPERTY(EditDefaultsOnly, Category = "Patching")
		TArray<int32> ChunkDownloadList;

	// 청크 다운로드 프로세스가 완료되면 호출됩니다.
	void OnDownloadComplete(bool bSuccess);

	// ChunkDownloader의 로딩 모드가 완료될 때마다 호출됩니다.
	void OnLoadingModeComplete(bool bSuccess);

	// ChunkDownloader가 청크 마운트를 완료하면 호출됩니다.
	void OnMountComplete(bool bSuccess);
};
