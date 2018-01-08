// Copyright (C) 2013-2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

namespace UnrealBuildTool.Rules
{
  public class FunapiDedicatedServer : ModuleRules
  {
    // public FunapiDedicatedServer(TargetInfo Target) // <= 4.15
    public FunapiDedicatedServer(ReadOnlyTargetRules Target) : base(Target) // >= 4.16
    {
      Definitions.Add("WITH_FUNAPIDEDICATEDSERVER=1");

      PublicIncludePaths.AddRange(
        new string[] {
          // ... add public include paths required here ...
          "FunapiDedicatedServer/Pubilc",
        }
      );

      PrivateIncludePaths.AddRange(
        new string[] {
          // "FunapiDedicatedServer/Private",
          // ... add other private include paths required here ...
        }
      );

      PublicDependencyModuleNames.AddRange(
        new string[] {
          // "Core",
          // "Engine",
          // ... add other public dependencies that you statically link with here ...
        }
      );

      PrivateDependencyModuleNames.AddRange(
        new string[]
        {
          "Core",
          "Engine",
          "Json",
          "Http"
          // ... add private dependencies that you statically link with here ...
        }
      );

      DynamicallyLoadedModuleNames.AddRange(
        new string[]
        {
          // ... add any modules that your module loads dynamically here ...
        }
      );
    }
  }
}