// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class FunapiDedicatedServer : ModuleRules
	{
		public FunapiDedicatedServer(TargetInfo Target)
		{
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
				new string[]
				{
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