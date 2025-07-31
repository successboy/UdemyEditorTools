// Copyright MODogma. All Rights Reserved.

#pragma once

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

class FUdemyCourseStyle
{
public:
	static void InitializeIcons();
	
	// Getter function to get the style set externally
	static FName GetStyleSetName() {return StyleSetName;}
	static TSharedRef<FSlateStyleSet> GetCreatedStyleSet() {return CreatedStyleSet.ToSharedRef();}
	static void Shutdown();

private:
	// Variables are rquired to be static if used within a static method
	static FName StyleSetName;
	static TSharedPtr<FSlateStyleSet> CreatedStyleSet;
	static TSharedRef<FSlateStyleSet> CreateStyleSet();
};