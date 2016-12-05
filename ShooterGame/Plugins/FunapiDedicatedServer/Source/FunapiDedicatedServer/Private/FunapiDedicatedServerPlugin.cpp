// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "FunapiDedicatedServerPrivatePCH.h"

DEFINE_LOG_CATEGORY(LogFunapiDedicatedServer);

class FFunapiDedicatedServer : public IFunapiDedicatedServer
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FFunapiDedicatedServer, FunapiDedicatedServer )


void FFunapiDedicatedServer::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
  // UE_LOG(LogTemp, Log, TEXT("StartupModule"));
}


void FFunapiDedicatedServer::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
  // UE_LOG(LogTemp, Log, TEXT("ShutdownModule"));
}
