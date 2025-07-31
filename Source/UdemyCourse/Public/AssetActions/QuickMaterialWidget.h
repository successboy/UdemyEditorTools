// Copyright MODogma. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
// Inherits class members from TextureSampleParameter, with more members
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"

#include "QuickMaterialWidget.generated.h"

UENUM(BlueprintType)
enum class EChannelPackType : uint8
{
	ECPT_NoChannelPacking		UMETA(DisplayName = "No Channel Packing"),
	ECPT_ORM					UMETA(DisplayName = "AO(R) | Rough(G) | Metal(B)"),
	ECPT_MRA					UMETA(DisplayName = "TODO: Metal(R) | Rough(G) | AO(B)"),
	ECPT_MAX					UMETA(Hidden),
};

/**
 *
 */
UCLASS()
class UDEMYCOURSE_API UQuickMaterialWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:

#pragma region QuickMaterialCore

	UFUNCTION(BlueprintCallable, Category = "CreateMaterial", meta = (ToolTip = "Hello."))
	void CreateMaterialFromSelectedTextures();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial", meta = (ToolTip = "Blah.", EditCondition= "bOverrideMaterialName"))
	FString MaterialName = TEXT("M_");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial", meta = (ToolTip = "Test 100."))
	EChannelPackType ChannelPackType = EChannelPackType::ECPT_ORM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial", meta = (ToolTip = "Override the material name with a specified one.\nUse the select texture name as material name if false."))
	bool bOverrideMaterialName = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial", meta = (ToolTip = "Create child material instance after the material is generated."))
	bool bCreateMaterialInstance = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial", meta = (ToolTip = "Optimizes texture settings for best practices depending on texture type.\nThis should nearly-always be enabled."))
	bool bAutoFixTextures = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial", meta = (ToolTip = "Automatically saves the material after generating."))
	bool bAutosave = false;

#pragma endregion

#pragma region SupportedTextureNames
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial|Textures", meta = (ToolTip = "Okay 1."))
	TArray<FString> BaseColorSuffixes = { "_BaseColor", "_BC", "_Color", "_D", "_Diffuse", "_Diff", "_Albedo" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial|Textures", meta = (ToolTip = "Fine 1."))
	TArray<FString> NormalSuffixes = { "_N", "_Normal", "_Norm", "_NM", "_NormalMap" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial|Textures", meta = (ToolTip = "Blur 1."))
	TArray<FString> MetallicSuffixes = { "_Metallic", "Metallness", "_MT", "_Metal" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial|Textures", meta = (ToolTip = "Boo 1."))
	TArray<FString> AmbientOcclusionSuffixes = { "_AO", "_AmbientOcclusion" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial|Textures", meta = (ToolTip = "One 1."))
	TArray<FString> RoughnessSuffixes = { "_R", "_Roughness", "_Rough" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial|Textures", meta = (ToolTip = "Two 1."))
	TArray<FString> OpacitySuffixes = { "_O", "_M", "_Mask", "_A", "_Opacity", "_OpacityMask" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial|Textures", meta = (ToolTip = "Three 1."))
	TArray<FString> EmissiveColorSuffixes = { "_E", "_Emissive", "_EmissiveColor" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial|Textures", meta = (ToolTip = "Four 1."))
	TArray<FString> SpecularSuffixes = { "_S", "_Spec", "_SpecColor", "_SpecCol", "_Specular" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterial|Textures", meta = (ToolTip = "Five 1."))
	TArray<FString> PackedSuffixes = { "_ORM", "_ARM", "_AoRM", "_Packed", "_OcclusionRoughnessMetallic", "_AoRT"};

	// TODO: PackedBSuffixes = _MRAO, _MRA

	// TODO: PackedCSuffixes = _DpRM, _DRM

	// TODO: HeightSuffixes: _Height, _H, _Disp, _Disp, _Dp

	// TODO: SubsurfaceScatteringSuffixes: SSS, Subsurface

#pragma endregion

private:

#pragma region QuickMaterialCreation
	int32 NodeSortPriority = 0;
	int32 NodeOffsetY = 0;
	bool bPackedPinsConnected = false;
	static const int32 OffsetIncrement = 300;

	/** Returns true if asset data was successfully processed */
	bool ProcessSelectedData(const TArray<FAssetData>& InSelectedAssetsData, TArray<UTexture2D*>& OutSelectedTextures, FString& OutSelectedPackagePath);

	/** Returns true if the material name already exists in the specified path */
	bool AssetNameExists(const FString& InPackagePath, const FString& InMaterialName);

	UMaterial* CreateMaterialAsset(const FString& InMaterialName, const FString& InStorePath);
	UMaterialInstanceConstant* CreateMaterialInstanceAsset(const FString& InMaterialInstanceName, const FString& InStorePath, UMaterial* InCreatedParentMaterial);
	void Default_CreateMaterialNodes(UMaterial* InCreatedMaterial, UTexture2D* InSelectedTexture, uint32& InNumConnectedPins);
	void ORM_CreateMaterialNodes(UMaterial* InCreatedMaterial, UTexture2D* InSelectedTexture, uint32& InNumConnectedPins);
	void RemoveSuffixKeyword();

#pragma endregion

#pragma region CreateMaterialNodes

	bool ConnectNode(UMaterialExpressionTextureSampleParameter2D* InTextureSampleNode, UTexture2D* InSelectedTexture, UMaterial* InCreatedMaterial, const TArray<FString>& InSuffixes, const FName& InParameterName, const FName& InGroup, const EMaterialSamplerType& InSamplerType, const EMaterialSamplerType& InVirtualSamplerType, const EMaterialProperty& InNodeToConnect, const int32& InOffsetY = 0, const int32& InSortPriority = 32);
	void FixTextureSettings(UTexture2D* InSelectedTexture, const TArray<FString>& InSuffixes, const TextureCompressionSettings& InCompression = TextureCompressionSettings::TC_Default, const bool& bUseSRGB = true);

	void NewFunction();


#pragma endregion



















};