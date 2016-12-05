// Copyright (C) 2013-2016 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#pragma once

namespace fun {

  class FUNAPIDEDICATEDSERVER_API FunapiDedicatedServer
  {
  public:
    static bool CommandLine(const TCHAR* cmd, const FString &match_id_field, const FString &manager_server_field);
    static void Get(const TFunction<void(FHttpResponsePtr response)> &completion_handler);
    static void Ready();
    static void Result(const FString &json_string, const bool use_exit);
    static bool Auth(const FString& options, const FString& uid_field, const FString& token_field, FString &error_message);
  };

}
