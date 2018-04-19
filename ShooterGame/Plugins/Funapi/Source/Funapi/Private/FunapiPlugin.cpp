// Copyright (C) 2013-2017 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "FunapiPrivatePCH.h"
#include "IFunapiPlugin.h"

DEFINE_LOG_CATEGORY(LogFunapi);

class FFunapi : public IFunapi
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FFunapi, Funapi )


void FFunapi::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
  // UE_LOG(LogTemp, Log, TEXT("StartupModule"));
}


void FFunapi::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
  // UE_LOG(LogTemp, Log, TEXT("ShutdownModule"));
}
