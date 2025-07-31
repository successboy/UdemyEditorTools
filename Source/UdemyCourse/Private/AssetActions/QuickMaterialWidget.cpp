// Copyright MODogma. All Rights Reserved.

#include "AssetActions/QuickMaterialWidget.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "MaterialEditorModule.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h" // Required for StaticClass()
#include "MaterialGraph/MaterialGraph.h"
#include "MaterialGraph/MaterialGraphNode.h"

#define LOCTEXT_NAMESPACE "QuickMaterialWidget"

#pragma region QuickMaterialCreationCore
void UQuickMaterialWidget::CreateMaterialFromSelectedTextures()
{
	if (bOverrideMaterialName)
	{
		// Use the texture name if no input from user
		if (MaterialName.IsEmpty() || MaterialName.Equals(TEXT("_M")))
		{
			DebugHeader::ShowNotification(
				LOCTEXT("InvalidNameInput", "A custom material name is required!"),
				ELogVerbosity::Warning
			);

			return;
		}
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*> SelectedTextures;
	FString SelectedTexturePath;
	uint32 NumConnectedPins = 0;

	if (!ProcessSelectedData(SelectedAssetsData, SelectedTextures, SelectedTexturePath))
	{
		MaterialName = TEXT("M_");
		// No data processed, exit early
		return;
	}

	if (AssetNameExists(SelectedTexturePath, MaterialName))
	{
		DebugHeader::ShowNotification(
			FText::Format(LOCTEXT("DuplicateAssetName", "A material with name '{0}' exists. Skipping generation!"), FText::FromString(SelectedAssetsData[0].GetAsset()->GetName())),
			ELogVerbosity::Warning
		);
		MaterialName = TEXT("M_");
		return;
	}

	UMaterial* CreatedMaterial = CreateMaterialAsset(MaterialName, SelectedTexturePath);

	if (!CreatedMaterial)
	{
		DebugHeader::ShowNotification(
			FText::Format(LOCTEXT("MaterialCreationFailure", "Failed to create new material: %s"), FText::FromString(MaterialName)),
			ELogVerbosity::Error
		);

		return;
	}

	// Reset to starting value of the first node
	NodeOffsetY = -600;

	// TODO: Convert this to a multimap, instead, and with each material input having its own array of suffixes
	for (UTexture2D* SelectedTexture : SelectedTextures)
	{
		if (!SelectedTexture)
		{
			continue;
		}

		if (bAutoFixTextures)
		{
			FixTextureSettings(SelectedTexture, BaseColorSuffixes);
			FixTextureSettings(SelectedTexture, MetallicSuffixes, TextureCompressionSettings::TC_Grayscale, false);
			FixTextureSettings(SelectedTexture, OpacitySuffixes, TextureCompressionSettings::TC_Grayscale, false);
			FixTextureSettings(SelectedTexture, SpecularSuffixes, TextureCompressionSettings::TC_Grayscale, false);
			FixTextureSettings(SelectedTexture, RoughnessSuffixes, TextureCompressionSettings::TC_Grayscale, false);
			FixTextureSettings(SelectedTexture, EmissiveColorSuffixes);
			FixTextureSettings(SelectedTexture, NormalSuffixes, TextureCompressionSettings::TC_Normalmap, false);
			FixTextureSettings(SelectedTexture, PackedSuffixes, TextureCompressionSettings::TC_Masks, false);
		}

		// Reset for every new material in ConnectNode()
		bPackedPinsConnected = false;

		// Switch packing based on user enum selection
		switch (ChannelPackType)
		{
		case EChannelPackType::ECPT_NoChannelPacking:
			Default_CreateMaterialNodes(CreatedMaterial, SelectedTexture, NumConnectedPins);
			break;
		case EChannelPackType::ECPT_ORM:
			ORM_CreateMaterialNodes(CreatedMaterial, SelectedTexture, NumConnectedPins);
			break;
		case EChannelPackType::ECPT_MRA:
			DebugHeader::ShowNotification(LOCTEXT("TODO", "TODO: Implement MRA packing."));
			break;
		case EChannelPackType::ECPT_MAX:
			break;
		default:
			break;
		}
	}

	// Compile material outside of a loop, once all changes have been made
	CreatedMaterial->PostEditChange();

	if (bCreateMaterialInstance)
	{
		UMaterialInstanceConstant* CreatedMaterialInstance = CreateMaterialInstanceAsset(MaterialName, SelectedTexturePath, CreatedMaterial);

		if (!CreatedMaterialInstance)
		{
			DebugHeader::ShowNotification(
				FText::Format(LOCTEXT("MaterialInstanceNotCreated", "Error creating material instance '{0}'."), FText::FromString(CreatedMaterialInstance->GetName())),
				ELogVerbosity::Error
			);
			return;
		}

		if (bAutosave)
		{
			UEditorAssetLibrary::SaveAsset(CreatedMaterialInstance->GetPathName());
		}

		DebugHeader::ShowNotification(FText::Format(LOCTEXT("MaterialInstanceCreated", "New material instance '{0}' has been created succesfully!"), FText::FromString(CreatedMaterialInstance->GetName())));
	}

	if (bAutosave)
	{
		UEditorAssetLibrary::SaveAsset(CreatedMaterial->GetPathName());
	}

	DebugHeader::ShowNotification(FText::Format(LOCTEXT("MaterialCreated", "New material '{0}' has been created succesfully!"), FText::FromString(CreatedMaterial->GetName())));

	// Reset the name
	MaterialName = TEXT("M_");
}

#pragma endregion

#pragma region QuickMaterialCreation

bool UQuickMaterialWidget::ProcessSelectedData(const TArray<FAssetData>& InSelectedAssetsData, TArray<UTexture2D*>& OutSelectedTextures, FString& OutSelectedPackagePath)
{
	if (InSelectedAssetsData.IsEmpty())
	{
		DebugHeader::ShowNotification(
			LOCTEXT("InvalidSelection", "A texture asset selection is required!"),
			ELogVerbosity::Warning
		);
		return false;
	}

	bool bMaterialRenamed = false;

	for (const FAssetData& AssetData : InSelectedAssetsData)
	{
		UObject* SelectedAsset = AssetData.GetAsset();

		if (!SelectedAsset)
		{
			continue;
		}

		UTexture2D* SelectedTexture = Cast<UTexture2D>(SelectedAsset);

		if (!SelectedTexture)
		{
			return false;
		}

		OutSelectedTextures.Add(SelectedTexture);

		if (OutSelectedPackagePath.IsEmpty())
		{
			OutSelectedPackagePath = AssetData.PackagePath.ToString();
		}

		if (!bOverrideMaterialName && !bMaterialRenamed)
		{
			MaterialName = SelectedTexture->GetName();
			// Remove the texture prefix and add M_
			MaterialName.RemoveFromStart(TEXT("T_"));
			MaterialName.InsertAt(0, TEXT("M_"));
			RemoveSuffixKeyword();
			bMaterialRenamed = true;
		}
	}

	return true;
}

bool UQuickMaterialWidget::AssetNameExists(const FString& InPackagePath, const FString& InMaterialName)
{
	// Check asset names in current path
	TArray<FString> ExistingAssetPaths = UEditorAssetLibrary::ListAssets(InPackagePath, false);

	for (const FString& ExistingAssetPath : ExistingAssetPaths)
	{
		const FString ExistingAssetName = FPaths::GetBaseFilename(ExistingAssetPath);

		// This is actually okay to only check the name string and not the class (instead of using 
		// AssetRegistry), as it's checking for "M_<AssetName>" which will likely be a UMaterial class.
		if (ExistingAssetName.Equals(InMaterialName))
		{
			return true;
		}
	}

	return false;
}

UMaterial* UQuickMaterialWidget::CreateMaterialAsset(const FString& InMaterialName, const FString& InStorePath)
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	// Construct a new UObject for generating material factory
	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
	UObject* CreatedAsset = AssetTools.CreateAsset(InMaterialName, InStorePath, UMaterial::StaticClass(), MaterialFactory);
	UMaterial* CreatedMaterial = Cast<UMaterial>(CreatedAsset);

	if (!CreatedMaterial)
	{
		return nullptr;
	}

	return CreatedMaterial;
}

UMaterialInstanceConstant* UQuickMaterialWidget::CreateMaterialInstanceAsset(const FString& InParentMaterialName, const FString& InStorePath, UMaterial* InCreatedParentMaterial)
{
	FString MaterialInstanceName = InParentMaterialName;
	MaterialInstanceName.RemoveFromStart("M_");
	MaterialInstanceName.InsertAt(0, TEXT("MI_"));
	
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UMaterialInstanceConstantFactoryNew* MaterialInstanceFactory = NewObject<UMaterialInstanceConstantFactoryNew>();
	UObject* CreatedAsset = AssetTools.CreateAsset(MaterialInstanceName, InStorePath, UMaterialInstanceConstant::StaticClass(), MaterialInstanceFactory);
	UMaterialInstanceConstant* CreatedMaterialInstance = Cast<UMaterialInstanceConstant>(CreatedAsset);

	if (!CreatedMaterialInstance)
	{
		DebugHeader::PrintDebugMessage(TEXT("Failed to create Material Instance!"), FColor::Red);
		return nullptr;
	}

	if (InCreatedParentMaterial)
	{
		CreatedMaterialInstance->SetParentEditorOnly(InCreatedParentMaterial);
		// Required to propagate changes to full material chain, per comment in SetParentEditorOnly()
		CreatedMaterialInstance->PostEditChange();
	}

	return CreatedMaterialInstance;
}

void UQuickMaterialWidget::Default_CreateMaterialNodes(UMaterial* InCreatedMaterial, UTexture2D* InSelectedTexture, uint32& InNumConnectedPins)
{
	UMaterialExpressionTextureSampleParameter2D* TextureSampleNode = NewObject<UMaterialExpressionTextureSampleParameter2D>(InCreatedMaterial);

	if (!TextureSampleNode)
	{
		return;
	}

	// Create and connect nodes in the new material. Wiring order is
	// hard to determine, and depends on order of InSelectedTexture
	if (!InCreatedMaterial->HasBaseColorConnected())
	{
		if (ConnectNode(TextureSampleNode, InSelectedTexture, InCreatedMaterial, BaseColorSuffixes, TEXT("BaseColor"), TEXT("01 - Texture Inputs"), EMaterialSamplerType::SAMPLERTYPE_Color, EMaterialSamplerType::SAMPLERTYPE_VirtualColor, EMaterialProperty::MP_BaseColor, NodeOffsetY, NodeSortPriority))
		{
			++InNumConnectedPins;
			NodeOffsetY += OffsetIncrement;
			++NodeSortPriority;
			return;
		}
	}

	if (!InCreatedMaterial->HasMetallicConnected())
	{
		if (ConnectNode(TextureSampleNode, InSelectedTexture, InCreatedMaterial, MetallicSuffixes, TEXT("Metallic"), TEXT("01 - Texture Inputs"), EMaterialSamplerType::SAMPLERTYPE_LinearGrayscale, EMaterialSamplerType::SAMPLERTYPE_VirtualLinearGrayscale, EMaterialProperty::MP_Metallic, NodeOffsetY, NodeSortPriority))
		{
			NodeOffsetY += OffsetIncrement;
			++NodeSortPriority;
			++InNumConnectedPins;
			return;
		}
	}

	// There is no opacity member function, so manually check input connection.
	// Material must use a masked blend mode to enable Opacity Mask input
	if (!InCreatedMaterial->IsPropertyConnected(MP_Opacity))
	{
		if (ConnectNode(TextureSampleNode, InSelectedTexture, InCreatedMaterial, OpacitySuffixes, TEXT("Opacity"), TEXT("01 - Texture Inputs"), EMaterialSamplerType::SAMPLERTYPE_LinearGrayscale, EMaterialSamplerType::SAMPLERTYPE_VirtualLinearGrayscale, EMaterialProperty::MP_OpacityMask, NodeOffsetY, NodeSortPriority))
		{
			NodeOffsetY += OffsetIncrement;
			++NodeSortPriority;
			++InNumConnectedPins;
			return;
		}
	}

	if (!InCreatedMaterial->HasSpecularConnected())
	{
		if (ConnectNode(TextureSampleNode, InSelectedTexture, InCreatedMaterial, SpecularSuffixes, TEXT("Specular"), TEXT("01 - Texture Inputs"), EMaterialSamplerType::SAMPLERTYPE_LinearGrayscale, EMaterialSamplerType::SAMPLERTYPE_VirtualLinearGrayscale, EMaterialProperty::MP_Specular, NodeOffsetY, NodeSortPriority))
		{
			NodeOffsetY += OffsetIncrement;
			++NodeSortPriority;
			++InNumConnectedPins;
			return;
		}
	}

	if (!InCreatedMaterial->HasRoughnessConnected())
	{
		if (ConnectNode(TextureSampleNode, InSelectedTexture, InCreatedMaterial, RoughnessSuffixes, TEXT("Roughness"), TEXT("01 - Texture Inputs"), EMaterialSamplerType::SAMPLERTYPE_LinearGrayscale, EMaterialSamplerType::SAMPLERTYPE_VirtualLinearGrayscale, EMaterialProperty::MP_Roughness, NodeOffsetY, NodeSortPriority))
		{
			NodeOffsetY += OffsetIncrement;
			++NodeSortPriority;
			++InNumConnectedPins;
			return;
		}
	}

	if (!InCreatedMaterial->HasAmbientOcclusionConnected())
	{
		if (ConnectNode(TextureSampleNode, InSelectedTexture, InCreatedMaterial, AmbientOcclusionSuffixes, TEXT("AmbientOcclusion"), TEXT("01 - Texture Inputs"), EMaterialSamplerType::SAMPLERTYPE_LinearGrayscale, EMaterialSamplerType::SAMPLERTYPE_VirtualLinearGrayscale, EMaterialProperty::MP_AmbientOcclusion, NodeOffsetY, NodeSortPriority))
		{
			NodeOffsetY += OffsetIncrement;
			++NodeSortPriority;
			++InNumConnectedPins;
			return;
		}
	}

	if (!InCreatedMaterial->HasEmissiveColorConnected())
	{
		if (ConnectNode(TextureSampleNode, InSelectedTexture, InCreatedMaterial, EmissiveColorSuffixes, TEXT("Emissive"), TEXT("01 - Texture Inputs"), EMaterialSamplerType::SAMPLERTYPE_Color, EMaterialSamplerType::SAMPLERTYPE_VirtualColor, EMaterialProperty::MP_EmissiveColor, NodeOffsetY, 6))
		{
			NodeOffsetY += OffsetIncrement;
			++NodeSortPriority;
			++InNumConnectedPins;
			return;
		}
	}

	if (!InCreatedMaterial->HasNormalConnected())
	{
		if (ConnectNode(TextureSampleNode, InSelectedTexture, InCreatedMaterial, NormalSuffixes, TEXT("Normal"), TEXT("01 - Texture Inputs"), EMaterialSamplerType::SAMPLERTYPE_Normal, EMaterialSamplerType::SAMPLERTYPE_VirtualNormal, EMaterialProperty::MP_Normal, NodeOffsetY, NodeSortPriority))
		{
			NodeOffsetY += OffsetIncrement;
			++NodeSortPriority;
			++InNumConnectedPins;
			return;
		}
	}
}

void UQuickMaterialWidget::ORM_CreateMaterialNodes(UMaterial* InCreatedMaterial, UTexture2D* InSelectedTexture, uint32& InNumConnectedPins)
{
	UMaterialExpressionTextureSampleParameter2D* TextureSampleNode = NewObject<UMaterialExpressionTextureSampleParameter2D>(InCreatedMaterial);

	if (!TextureSampleNode)
	{
		return;
	}

	if (!InCreatedMaterial->HasBaseColorConnected())
	{
		if (ConnectNode(TextureSampleNode, InSelectedTexture, InCreatedMaterial, BaseColorSuffixes, TEXT("BaseColor"), TEXT("01 - Texture Inputs"), EMaterialSamplerType::SAMPLERTYPE_Color, EMaterialSamplerType::SAMPLERTYPE_VirtualColor, EMaterialProperty::MP_BaseColor, NodeOffsetY, NodeSortPriority))
		{
			NodeOffsetY += OffsetIncrement;
			++NodeSortPriority;
			++InNumConnectedPins;
			return;
		}
	}

	///////////////////////
	// Begin packed pins
	///////////////////////
	if (!InCreatedMaterial->HasRoughnessConnected())
	{
		if (ConnectNode(TextureSampleNode, InSelectedTexture, InCreatedMaterial, PackedSuffixes, TEXT("Packed"), TEXT("01 - Texture Inputs"), EMaterialSamplerType::SAMPLERTYPE_Masks, EMaterialSamplerType::SAMPLERTYPE_VirtualMasks, EMaterialProperty::MP_Roughness, NodeOffsetY, NodeSortPriority))
		{
			++InNumConnectedPins;
			bPackedPinsConnected = true;
		}
	}

	if (!InCreatedMaterial->HasAmbientOcclusionConnected())
	{
		if (ConnectNode(TextureSampleNode, InSelectedTexture, InCreatedMaterial, PackedSuffixes, TEXT("Packed"), TEXT("01 - Texture Inputs"), EMaterialSamplerType::SAMPLERTYPE_Masks, EMaterialSamplerType::SAMPLERTYPE_VirtualMasks, EMaterialProperty::MP_AmbientOcclusion, NodeOffsetY, NodeSortPriority))
		{
			++InNumConnectedPins;
			bPackedPinsConnected = true;
		}
	}

	if (!InCreatedMaterial->HasMetallicConnected())
	{
		if (ConnectNode(TextureSampleNode, InSelectedTexture, InCreatedMaterial, PackedSuffixes, TEXT("Packed"), TEXT("01 - Texture Inputs"), EMaterialSamplerType::SAMPLERTYPE_Masks, EMaterialSamplerType::SAMPLERTYPE_VirtualMasks, EMaterialProperty::MP_Metallic, NodeOffsetY, NodeSortPriority))
		{
			++InNumConnectedPins;
			bPackedPinsConnected = true;
		}
	}
	///////////////////////
	// End packed pins
	///////////////////////

	// Only return once all packed pins have attempted connection
	if (bPackedPinsConnected)
	{
		NodeOffsetY += OffsetIncrement;
		++NodeSortPriority;
		return;
	}

	if (!InCreatedMaterial->HasNormalConnected())
	{
		if (ConnectNode(TextureSampleNode, InSelectedTexture, InCreatedMaterial, NormalSuffixes, TEXT("Normal"), TEXT("01 - Texture Inputs"), EMaterialSamplerType::SAMPLERTYPE_Normal, EMaterialSamplerType::SAMPLERTYPE_VirtualNormal, EMaterialProperty::MP_Normal, NodeOffsetY, NodeSortPriority))
		{
			NodeOffsetY += OffsetIncrement;
			++NodeSortPriority;
			++InNumConnectedPins;
			return;
		}
	}
}

void UQuickMaterialWidget::RemoveSuffixKeyword()
{
	TArray<FString> AllSuffixes;
	AllSuffixes.Append(BaseColorSuffixes);
	AllSuffixes.Append(NormalSuffixes);
	AllSuffixes.Append(MetallicSuffixes);
	AllSuffixes.Append(AmbientOcclusionSuffixes);
	AllSuffixes.Append(RoughnessSuffixes);
	AllSuffixes.Append(OpacitySuffixes);
	AllSuffixes.Append(EmissiveColorSuffixes);
	AllSuffixes.Append(SpecularSuffixes);
	AllSuffixes.Append(PackedSuffixes);

	for (const FString& Suffix : AllSuffixes)
	{
		MaterialName.RemoveFromEnd(Suffix);
	}
}

#pragma endregion

#pragma region CreateMaterialNodes

bool UQuickMaterialWidget::ConnectNode(UMaterialExpressionTextureSampleParameter2D* InTextureSampleNode, UTexture2D* InSelectedTexture, UMaterial* InCreatedMaterial, const TArray<FString>& InSuffixes, const FName& InParameterName, const FName& InGroup, const EMaterialSamplerType& InSamplerType, const EMaterialSamplerType& InVirtualSamplerType, const EMaterialProperty& InNodeToConnect, const int32& InNodeOffsetY, const int32& InSortPriority)
{
	for (const FString& Suffix : InSuffixes)
	{
		if (InSelectedTexture->GetName().EndsWith(Suffix))
		{
			// Assign texture data to the node
			InTextureSampleNode->Texture = InSelectedTexture;

			// Check if input texture asset is virtual before assigning, as sampler type must match
			if (InSelectedTexture->IsCurrentlyVirtualTextured())
			{
				InTextureSampleNode->SamplerType = InVirtualSamplerType;
			}
			else
			{
				InTextureSampleNode->SamplerType = InSamplerType;
			}

			InTextureSampleNode->ParameterName = InParameterName;
			InTextureSampleNode->Group = InGroup;
			InTextureSampleNode->SortPriority = InSortPriority;

			// Gate the offset to only once for packed maps
			if (!bPackedPinsConnected)
			{
				// Offset the node to the left and away from the result node
				InTextureSampleNode->MaterialExpressionEditorX -= 1000;
			}

			InTextureSampleNode->MaterialExpressionEditorY += InNodeOffsetY;
			// Add the texture sample node to the material
			InCreatedMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(InTextureSampleNode);

			// Handle ORM channel packing
			if (ChannelPackType == EChannelPackType::ECPT_ORM && 
				(InNodeToConnect == EMaterialProperty::MP_AmbientOcclusion || InNodeToConnect == EMaterialProperty::MP_Roughness || InNodeToConnect == EMaterialProperty::MP_Metallic))
			{
				// Connect channels to appropriate material inputs based on ORM packing
				if (InNodeToConnect == EMaterialProperty::MP_AmbientOcclusion)
				{
					if (FExpressionInput* ResultNodeInput = InCreatedMaterial->GetExpressionInputForProperty(EMaterialProperty::MP_AmbientOcclusion))
					{
						// Reverse-connect to the texture sample output
						ResultNodeInput->Connect(1/*R channel*/, InTextureSampleNode);
					}
				}
				else if (InNodeToConnect == EMaterialProperty::MP_Roughness)
				{
					if (FExpressionInput* ResultNodeInput = InCreatedMaterial->GetExpressionInputForProperty(EMaterialProperty::MP_Roughness))
					{
						// Reverse-connect to the texture sample output
						ResultNodeInput->Connect(2/*G channel*/, InTextureSampleNode);
					}
				}
				else if (InNodeToConnect == EMaterialProperty::MP_Metallic)
				{
					if (FExpressionInput* ResultNodeInput = InCreatedMaterial->GetExpressionInputForProperty(EMaterialProperty::MP_Metallic))
					{
						// Reverse-connect to the texture sample output
						ResultNodeInput->Connect(3/*B Channel*/, InTextureSampleNode);
					}
				}

				return true;
			}
			else if (ChannelPackType == EChannelPackType::ECPT_MRA && (InNodeToConnect == EMaterialProperty::MP_AmbientOcclusion || InNodeToConnect == EMaterialProperty::MP_Roughness || InNodeToConnect == EMaterialProperty::MP_Metallic))
			{
				DebugHeader::ShowNotification(LOCTEXT("TodoImplementation", "TODO: Implement MRA channel packing."));
				return false; // TODO: Change to true when properly implemented
			}
			else // Standard single-channel connection
			{
				if (FExpressionInput* ResultNodeInput = InCreatedMaterial->GetExpressionInputForProperty(InNodeToConnect))
				{
					// Connect the default index 0 (RGB) of texture sample node to the result node
					ResultNodeInput->Expression = InTextureSampleNode;

					return true;
				}
				else
				{
					DebugHeader::PrintDebugMessage(FString::Printf(TEXT("NodeInput invalid for %s"), *InParameterName.ToString()));
				}
			}
		}
	}

	return false;
}

void UQuickMaterialWidget::FixTextureSettings(UTexture2D* InSelectedTexture, const TArray<FString>& InSuffixes, const TextureCompressionSettings& InCompression, const bool& bUseSRGB)
{
	for (const FString& Suffix : InSuffixes)
	{
		if (InSelectedTexture->GetName().EndsWith(Suffix))
		{
			// TODO: Check if the settings are already fully-the same. If they are, return. Print the settings which have changes.
			// The asset should be saved as well in case it is not.
			InSelectedTexture->CompressionSettings = InCompression;
			InSelectedTexture->SRGB = bUseSRGB;
			InSelectedTexture->PostEditChange();
			DebugHeader::ShowNotification(
				FText::Format(LOCTEXT("TextureFixedUp", "Texture '{0}' has had its texture settings fixed!"), FText::FromString(InSelectedTexture->GetName()))
			);
		}
	}
}

void UQuickMaterialWidget::NewFunction()
{

}

#pragma endregion

#undef LOCTEXT_NAMESPACE