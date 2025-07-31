// Copyright MODogma. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"

// Required for the prefix map
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/Texture.h"
#include "Blueprint/UserWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"

#include "QuickAssetAction.generated.h"

/**
 * 
 */
UCLASS()
class UDEMYCOURSE_API UQuickAssetAction : public UAssetActionUtility
{
	GENERATED_BODY()


public:
	UFUNCTION(CallInEditor, Category = "Udemy", meta = (ToolTip = "Duplicate selected assets for a specified number of times."))
	void DuplicateAssets(int32 NumDuplicates);

	UFUNCTION(CallInEditor, Category = "Udemy", meta = (ToolTip = "Modifies prefix/suffix in asset name to match standard naming convention."))
	void AddPrefixes();

	UFUNCTION(CallInEditor, Category = "Udemy", meta = (ToolTip = "Delete unreferenced assets."))
	void DeleteUnusedAssets(bool bInGetSelectedAssets);

	UFUNCTION(CallInEditor, Category = "Udemy", meta = (ToolTip = "Fix up all redirectors in the project."))
	void FixUpAllRedirectors();

	/** Test tooltip: Likely doesn't work for UAssetActionUtility */
	UFUNCTION(CallInEditor, Category = "Udemy", meta = (ToolTip = "Rename assets."))
	void RenameSelection(bool bAddPrefixes, FString NewName);

private:
	TMap<UClass*, FString> PrefixMap =
	{
		{UBlueprint::StaticClass(), TEXT("BP_")},
		{UStaticMesh::StaticClass(), TEXT("SM_")},
		{UMaterial::StaticClass(), TEXT("M_")},
		{UMaterialInstanceConstant::StaticClass(), TEXT("MI_")},
		{UMaterialFunctionInterface::StaticClass(), TEXT("MF_")},
		{UParticleSystem::StaticClass(), TEXT("PS_")},
		{USoundCue::StaticClass(), TEXT("SC_")},
		{USoundWave::StaticClass(), TEXT("SW_")},
		{UTexture::StaticClass(), TEXT("T_")},
		{UTexture2D::StaticClass(), TEXT("T_")},
		{UUserWidget::StaticClass(), TEXT("WBP_")},
		{USkeletalMeshComponent::StaticClass(), TEXT("SK_")},
		{UNiagaraSystem::StaticClass(), TEXT("NS_")},
		{UNiagaraEmitter::StaticClass(), TEXT("NE_")},
		// Add more common asset classes here. Will require including their headers
		// and modules (for classes existing in their own module)
	};
};
