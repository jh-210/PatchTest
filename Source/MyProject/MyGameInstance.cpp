#include "MyGameInstance.h"

#include "ChunkDownloader.h"
#include "Misc/CoreDelegates.h"
#include "AssetRegistryModule.h"

void UMyGameInstance::Init()
 {
         Super::Init();
         const FString DeploymentName = "PatchingDemoLive";
         const FString ContentBuildId = "PatchingDemoKey";

         // 선택한 플랫폼에서 청크 다운로더를 초기화합니다.
         TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetOrCreate();
         Downloader->Initialize("Windows", 8);

         // 캐싱된 빌드 ID를 로딩합니다
         Downloader->LoadCachedBuild(DeploymentName);

         // 빌드 매니페스트 파일을 업데이트합니다
         TFunction<void(bool bSuccess)> UpdateCompleteCallback = [&](bool bSuccess) {bIsDownloadManifestUpToDate = true; };
         Downloader->UpdateBuild(DeploymentName, ContentBuildId, UpdateCompleteCallback);
 }

 void UMyGameInstance::Shutdown()
 {
     Super::Shutdown();
     // 청크 다운로더를 종료합니다
     FChunkDownloader::Shutdown();
 }

 void UMyGameInstance::GetLoadingProgress(int32& BytesDownloaded, int32& TotalBytesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const
 {
     // 청크 다운로더의 레퍼런스를 구합니다.
     TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

     // 로딩 통계 구조체를 구합니다.
     FChunkDownloader::FStats LoadingStats = Downloader->GetLoadingStats();

     // 다운로드된 바이트와 다운로드할 바이트를 구합니다.
     BytesDownloaded = LoadingStats.BytesDownloaded;
     TotalBytesToDownload = LoadingStats.TotalBytesToDownload;

     // 마운트된 청크와 다운로드할 청크의 수를 구합니다.
     ChunksMounted = LoadingStats.ChunksMounted;
     TotalChunksToMount = LoadingStats.TotalChunksToMount;

     // 위의 통계를 사용하여 다운로드 및 마운트 퍼센트를 계산합니다.
     DownloadPercent = ((float)BytesDownloaded / (float)TotalBytesToDownload) * 100.0f;
     MountPercent = ((float)ChunksMounted / (float)TotalChunksToMount) * 100.0f;
 }

 bool UMyGameInstance::PatchGame()
 {
     // 다운로드 매니페스트를 최신 상태로 유지합니다.
     if (bIsDownloadManifestUpToDate)
     {
         // 청크 다운로더를 가져옵니다.
         TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

         // 현재 청크 상태를 제보합니다.
         for (int32 ChunkID : ChunkDownloadList)
         {
             int32 ChunkStatus = static_cast<int32>(Downloader->GetChunkStatus(ChunkID));
             UE_LOG(LogTemp, Display, TEXT("Chunk %i status: %i"), ChunkID, ChunkStatus);
         }

         TFunction<void(bool bSuccess)> DownloadCompleteCallback = [&](bool bSuccess) {OnDownloadComplete(bSuccess); };
         Downloader->DownloadChunks(ChunkDownloadList, DownloadCompleteCallback, 1);

         // 로딩 모드를 시작합니다.
         TFunction<void(bool bSuccess)> LoadingModeCompleteCallback = [&](bool bSuccess) {OnLoadingModeComplete(bSuccess); };
         Downloader->BeginLoadingMode(LoadingModeCompleteCallback);
         return true;
     }

     // 매니페스트를 검증하기 위해 서버와 연락하는 데 실패하여 패치할 수 없었습니다.
     UE_LOG(LogTemp, Display, TEXT("Manifest Update Failed. Can't patch the game"));

     return false;
 }

 void UMyGameInstance::OnManifestUpdateComplete(bool bSuccess)
 {
     bIsDownloadManifestUpToDate = bSuccess;
 }

 void UMyGameInstance::OnDownloadComplete(bool bSuccess)
 {
     if (bSuccess)
     {
         UE_LOG(LogTemp, Display, TEXT("Download complete"));

         // 청크 다운로더를 가져옵니다.
         TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();
         FJsonSerializableArrayInt DownloadedChunks;

         for (int32 ChunkID : ChunkDownloadList)
         {
             DownloadedChunks.Add(ChunkID);
         }

         // 청크를 마운트합니다.
         TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) {OnMountComplete(bSuccess); };
         Downloader->MountChunks(DownloadedChunks, MountCompleteCallback);

         OnPatchComplete.Broadcast(true);
     }
     else
     {
         UE_LOG(LogTemp, Display, TEXT("Load process failed"));

         // 델리게이트를 호출합니다.
         OnPatchComplete.Broadcast(false);
     }
     
 }

 void UMyGameInstance::OnLoadingModeComplete(bool bSuccess)
 {
     OnDownloadComplete(bSuccess);
 }

 void UMyGameInstance::OnMountComplete(bool bSuccess)
 {
     OnPatchComplete.Broadcast(bSuccess);
 }
