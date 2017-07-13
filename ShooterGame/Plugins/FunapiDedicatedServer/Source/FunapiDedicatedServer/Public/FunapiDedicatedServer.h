// Copyright (C) 2013-2017 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#pragma once

namespace fun {
  namespace FunapiDedicatedServer {
    extern FUNAPIDEDICATEDSERVER_API FString GetUserDataJsonString(const FString &uid);
    extern FUNAPIDEDICATEDSERVER_API FString GetMatchDataJsonString();
    extern FUNAPIDEDICATEDSERVER_API void SetUserDataCallback(const TFunction<void(const FString &uid, const FString &json_string)> &handler);
    extern FUNAPIDEDICATEDSERVER_API void SetMatchDataCallback(const TFunction<void(const FString &json_string)> &handler);
    extern FUNAPIDEDICATEDSERVER_API bool ParseConsoleCommand(const TCHAR* cmd);
    extern FUNAPIDEDICATEDSERVER_API bool ParseConsoleCommand(const TCHAR* cmd, const FString &match_id_field, const FString &manager_server_field);
    extern FUNAPIDEDICATEDSERVER_API bool ParseConsoleCommand(const TCHAR* cmd, const FString &match_id_field, const FString &manager_server_field, const FString &heartbeat_field);
    extern FUNAPIDEDICATEDSERVER_API void GetGameInfo(const TFunction<void(FHttpResponsePtr response)> &completion_handler);
    extern FUNAPIDEDICATEDSERVER_API void GetGameInfo(const TFunction<void(FHttpRequestPtr request, FHttpResponsePtr response, bool succeed)> &completion_handler);
    extern FUNAPIDEDICATEDSERVER_API void Post(const FString &path, const FString &json_string = "",
      const TFunction<void(FHttpRequestPtr request, FHttpResponsePtr response, bool succeed)> &completion_handler = [](FHttpRequestPtr request, FHttpResponsePtr response, bool succeed) {});
    extern FUNAPIDEDICATEDSERVER_API void PostReady();
    extern FUNAPIDEDICATEDSERVER_API void PostResult(const FString &json_string, const bool use_exit);
    extern FUNAPIDEDICATEDSERVER_API void PostHeartbeat();
    extern FUNAPIDEDICATEDSERVER_API void PostGameState(const FString &json_string);
    extern FUNAPIDEDICATEDSERVER_API void PostJoined(const FString &uid);
    extern FUNAPIDEDICATEDSERVER_API void PostLeft(const FString &uid);
    extern FUNAPIDEDICATEDSERVER_API bool AuthUser(const FString& options, const FString& uid_field, const FString& token_field, FString &error_message);
    extern FUNAPIDEDICATEDSERVER_API bool AuthUser(const FString& options, FString &error_message);
    extern FUNAPIDEDICATEDSERVER_API void PostCallback(const FString &json_string);
  }
}
