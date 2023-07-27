#include "MyGameInstance.h"

#include "ChunkDownloader.h"
#include "Misc/CoreDelegates.h"
#include "AssetRegistryModule.h"

void UMyGameInstance::Init()
 {
         Super::Init();
         const FString DeploymentName = "PatchTestKey";
         const FString ContentBuildId = "PatchTestKey";

         // ������ �÷������� ûũ �ٿ�δ��� �ʱ�ȭ�մϴ�.
         TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetOrCreate();
         Downloader->Initialize("Windows", 8);

         // ĳ�̵� ���� ID�� �ε��մϴ�
         Downloader->LoadCachedBuild(DeploymentName);

         // ���� �Ŵ��佺Ʈ ������ ������Ʈ�մϴ�
         TFunction<void(bool bSuccess)> UpdateCompleteCallback = [&](bool bSuccess) {bIsDownloadManifestUpToDate = true; };
         Downloader->UpdateBuild(DeploymentName, ContentBuildId, UpdateCompleteCallback);
 }

 void UMyGameInstance::Shutdown()
 {
     Super::Shutdown();
     // ûũ �ٿ�δ��� �����մϴ�
     FChunkDownloader::Shutdown();
 }

 void UMyGameInstance::GetLoadingProgress(int32& BytesDownloaded, int32& TotalBytesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const
 {
     // ûũ �ٿ�δ��� ���۷����� ���մϴ�.
     TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

     // �ε� ��� ����ü�� ���մϴ�.
     FChunkDownloader::FStats LoadingStats = Downloader->GetLoadingStats();

     // �ٿ�ε�� ����Ʈ�� �ٿ�ε��� ����Ʈ�� ���մϴ�.
     BytesDownloaded = LoadingStats.BytesDownloaded;
     TotalBytesToDownload = LoadingStats.TotalBytesToDownload;

     // ����Ʈ�� ûũ�� �ٿ�ε��� ûũ�� ���� ���մϴ�.
     ChunksMounted = LoadingStats.ChunksMounted;
     TotalChunksToMount = LoadingStats.TotalChunksToMount;

     // ���� ��踦 ����Ͽ� �ٿ�ε� �� ����Ʈ �ۼ�Ʈ�� ����մϴ�.
     DownloadPercent = ((float)BytesDownloaded / (float)TotalBytesToDownload) * 100.0f;
     MountPercent = ((float)ChunksMounted / (float)TotalChunksToMount) * 100.0f;
 }

 bool UMyGameInstance::PatchGame()
 {
     // �ٿ�ε� �Ŵ��佺Ʈ�� �ֽ� ���·� �����մϴ�.
     if (bIsDownloadManifestUpToDate)
     {
         if (GEngine)
         {
             GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Patch Start"));
         }

         // ûũ �ٿ�δ��� �����ɴϴ�.
         TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

         // ���� ûũ ���¸� �����մϴ�.
         for (int32 ChunkID : ChunkDownloadList)
         {
             int32 ChunkStatus = static_cast<int32>(Downloader->GetChunkStatus(ChunkID));
             UE_LOG(LogTemp, Display, TEXT("Chunk %i status: %i"), ChunkID, ChunkStatus);
         }

         TFunction<void(bool bSuccess)> DownloadCompleteCallback = [&](bool bSuccess) {OnDownloadComplete(bSuccess); };
         Downloader->DownloadChunks(ChunkDownloadList, DownloadCompleteCallback, 1);

         // �ε� ��带 �����մϴ�.
         TFunction<void(bool bSuccess)> LoadingModeCompleteCallback = [&](bool bSuccess) {OnLoadingModeComplete(bSuccess); };
         Downloader->BeginLoadingMode(LoadingModeCompleteCallback);
         return true;
     }

     // �Ŵ��佺Ʈ�� �����ϱ� ���� ������ �����ϴ� �� �����Ͽ� ��ġ�� �� �������ϴ�.
     UE_LOG(LogTemp, Display, TEXT("Manifest Update Failed. Can't patch the game"));

     if (GEngine)
     {
         GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Manifest Update Failed. Can't patch the game"));
     }

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

         // ûũ �ٿ�δ��� �����ɴϴ�.
         TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();
         FJsonSerializableArrayInt DownloadedChunks;

         for (int32 ChunkID : ChunkDownloadList)
         {
             DownloadedChunks.Add(ChunkID);
         }

         // ûũ�� ����Ʈ�մϴ�.
         TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) {OnMountComplete(bSuccess); };
         Downloader->MountChunks(DownloadedChunks, MountCompleteCallback);

         if (GEngine)
         {
             GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Download Complete!"));
         }

         OnPatchComplete.Broadcast(true);
     }
     else
     {
         UE_LOG(LogTemp, Display, TEXT("Load process failed"));

         if (GEngine)
         {
             GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Download failed!"));
         }

         // ��������Ʈ�� ȣ���մϴ�.
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
