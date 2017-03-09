// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterTypes.h"
#include "ShooterPlayerController_Menu.generated.h"

UCLASS()
class AShooterPlayerController_Menu : public APlayerController
{
	GENERATED_UCLASS_BODY()

	/** After game is initialized */
	virtual void PostInitializeComponents() override;
};

