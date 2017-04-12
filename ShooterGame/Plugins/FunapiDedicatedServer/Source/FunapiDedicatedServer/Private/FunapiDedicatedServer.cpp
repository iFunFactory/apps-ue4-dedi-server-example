// Copyright (C) 2013-2017 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "FunapiDedicatedServerPrivatePCH.h"
#include "FunapiDedicatedServer.h"

namespace fun
{
  namespace FunapiDedicatedServer
  {
    static FString funapi_manager_server_;
    static TMap<FString, FString> auth_map_;
    static double heartbeat_seconds_ = 0;

    void AsyncHeartbeat()
    {
      class FAsyncHeartbeatTask : public FNonAbandonableTask
      {
      public:
        /** Performs work on thread */
        void DoWork()
        {
          FPlatformProcess::Sleep(heartbeat_seconds_);
          PostHeartbeat();
        }

        /** Returns true if the task should be aborted.  Called from within the task processing code itself via delegate */
        bool ShouldAbort() const
        {
          return false;
        }

        TStatId GetStatId() const
        {
          return TStatId();
        }
      };

      (new FAutoDeleteAsyncTask<FAsyncHeartbeatTask>)->StartBackgroundTask();
    }

    bool ParseConsoleCommand(const TCHAR* cmd, const FString &match_id_field, const FString &manager_server_field, const FString &heartbeat_field)
    {
      if (ParseConsoleCommand(cmd, match_id_field, manager_server_field))
      {
        int32 seconds = 0;

        FString heartbeat_field_string(heartbeat_field);
        heartbeat_field_string.Append("=");

        if (FParse::Value(cmd, *heartbeat_field_string, seconds))
        {
          if (seconds > 0)
          {
            heartbeat_seconds_ = static_cast<double>(seconds);
            AsyncHeartbeat();
          }
        }

        return true;
      }

      return false;
    }

    bool ParseConsoleCommand(const TCHAR* cmd, const FString &match_id_field, const FString &manager_server_field)
    {
      bool ret = true;

      FString match_id;
      FString manager_server;

      if (FParse::Value(cmd, *FString(match_id_field), match_id))
      {
        match_id = match_id.Replace(TEXT("="), TEXT("")).Replace(TEXT("\""), TEXT("")); // replace quotes
        UE_LOG(LogFunapiDedicatedServer, Log, TEXT("match_id is '%s'"), *match_id);
      }
      else {
        ret = false;
      }

      if (FParse::Value(cmd, *FString(manager_server_field), manager_server))
      {
        manager_server = manager_server.Replace(TEXT("="), TEXT("")).Replace(TEXT("\""), TEXT("")); // replace quotes
        UE_LOG(LogFunapiDedicatedServer, Log, TEXT("manager_server is '%s'"), *manager_server);
      }
      else {
        ret = false;
      }

      if (ret) {
        funapi_manager_server_ = manager_server + "/match/" + match_id + "/";
        UE_LOG(LogFunapiDedicatedServer, Log, TEXT("Dedicated server manager server : %s"), *funapi_manager_server_);
      }

      return ret;
    }

    bool ParseConsoleCommand(const TCHAR* cmd)
    {
      return ParseConsoleCommand(cmd, "FunapiMatchID", "FunapiManagerServer", "FunapiHeartbeat");
    }

    void GetGameInfo(const TFunction<void(FHttpResponsePtr response)> &completion_handler) {
      if (funapi_manager_server_.IsEmpty()) {
        completion_handler(nullptr);
        return;
      }

      FString server_url = funapi_manager_server_;
      // server_url = "http://www.mocky.io/v2/5844042111000073010e6b1e"; // test url

      static auto http_request = FHttpModule::Get().CreateRequest();
      http_request->SetURL(server_url);
      http_request->SetVerb(FString("GET"));

      http_request->OnProcessRequestComplete().BindLambda(
        [completion_handler](FHttpRequestPtr request, FHttpResponsePtr response, bool succeed) {
        if (!succeed) {
          UE_LOG(LogFunapiDedicatedServer, Error, TEXT("Get : Response was invalid!"));

          completion_handler(nullptr);
        }
        else {
          FString json_fstring = response->GetContentAsString();
          UE_LOG(LogFunapiDedicatedServer, Log, TEXT("Get : Response = %s"), *json_fstring);

          TSharedRef< TJsonReader<> > reader = TJsonReaderFactory<>::Create(json_fstring);
          TSharedPtr<FJsonObject> json_object;

          if (FJsonSerializer::Deserialize(reader, json_object))
          {
            auth_map_.Empty();

            TSharedPtr<FJsonObject> data = json_object->GetObjectField(FString("data"));
            TArray<TSharedPtr<FJsonValue>> users = data->GetArrayField(FString("users"));

            int len = users.Num();
            for (int i = 0; i < len; ++i) {
              auto o = users[i]->AsObject();

              auto uid = o->GetStringField(FString("uid"));
              auto token = o->GetStringField(FString("token"));

              UE_LOG(LogFunapiDedicatedServer, Log, TEXT("%s, %s"), *uid, *token);

              auth_map_.Add(uid, token);
            }
          }

          completion_handler(response);
        }
      });

      http_request->ProcessRequest();
    }

    void PostReady()
    {
      if (funapi_manager_server_.IsEmpty()) {
        return;
      }

      FString server_url = funapi_manager_server_ + "ready";

      auto http_request = FHttpModule::Get().CreateRequest();
      http_request->SetURL(server_url);
      http_request->SetVerb(FString("POST"));
      http_request->SetHeader(FString("Content-Type"), FString("application/json; charset=utf-8"));

      http_request->OnProcessRequestComplete().BindLambda(
        [](FHttpRequestPtr request, FHttpResponsePtr response, bool succeed) {
        if (!succeed) {
          UE_LOG(LogFunapiDedicatedServer, Error, TEXT("Ready : Response was invalid!"));
        }
        else {
          FString json_fstring = response->GetContentAsString();
          UE_LOG(LogFunapiDedicatedServer, Log, TEXT("Ready : Response = %s"), *json_fstring);
        }
      });

      http_request->ProcessRequest();
    }

    void PostResult(const FString &json_string, const bool use_exit)
    {
      if (funapi_manager_server_.IsEmpty()) {
        return;
      }

      FString server_url = funapi_manager_server_ + "result";

      auto http_request = FHttpModule::Get().CreateRequest();
      http_request->SetURL(server_url);
      http_request->SetVerb(FString("POST"));
      http_request->SetHeader(FString("Content-Type"), FString("application/json; charset=utf-8"));
      http_request->SetHeader(FString("Content-Length"), FString::FromInt(json_string.Len()));
      http_request->SetContentAsString(json_string);

      http_request->OnProcessRequestComplete().BindLambda(
        [use_exit](FHttpRequestPtr request, FHttpResponsePtr response, bool succeed) {
        if (!succeed) {
          UE_LOG(LogFunapiDedicatedServer, Error, TEXT("Result : Response was invalid!"));
        }
        else {
          FString json_fstring = response->GetContentAsString();
          UE_LOG(LogFunapiDedicatedServer, Log, TEXT("Result : Response = %s"), *json_fstring);

          // exit
          if (use_exit) {
            FGenericPlatformMisc::RequestExit(false);
          }
        }
      });
      http_request->ProcessRequest();
    }

    void PostHeartbeat() {
      if (funapi_manager_server_.IsEmpty()) {
        return;
      }

      FString server_url = funapi_manager_server_ + "heartbeat";

      auto http_request = FHttpModule::Get().CreateRequest();
      http_request->SetURL(server_url);
      http_request->SetVerb(FString("POST"));
      http_request->SetHeader(FString("Content-Type"), FString("application/json; charset=utf-8"));

      http_request->OnProcessRequestComplete().BindLambda(
        [](FHttpRequestPtr request, FHttpResponsePtr response, bool succeed) {
        if (!succeed) {
          UE_LOG(LogFunapiDedicatedServer, Error, TEXT("Heartbeat : Response was invalid!"));
        }
        else {
          FString json_fstring = response->GetContentAsString();
          UE_LOG(LogFunapiDedicatedServer, Log, TEXT("Heartbeat : Response = %s"), *json_fstring);
        }

        AsyncHeartbeat();
      });

      http_request->ProcessRequest();
    }

    bool AuthUser(const FString& options, FString &error_message)
    {
      return FunapiDedicatedServer::AuthUser(options, "FunapiUID", "FunapiToken", error_message);
    }

    bool AuthUser(const FString& options, const FString& uid_field, const FString& token_field, FString &error_message)
    {
      if (funapi_manager_server_.IsEmpty()) {
        return true;
      }

      FString uid = UGameplayStatics::ParseOption(options, *uid_field);
      if (uid.Len() <= 0)
      {
        error_message = FString("User id does not set.");
        UE_LOG(LogFunapiDedicatedServer, Error, TEXT("[Auth] ERROR: %s"), *error_message);
        return false;
      }
      else {
        UE_LOG(LogFunapiDedicatedServer, Log, TEXT("[Auth] uid: %s"), *uid);
      }

      FString token = UGameplayStatics::ParseOption(options, token_field);
      if (token.Len() <= 0)
      {
        error_message = FString("User token does not set.");
        UE_LOG(LogFunapiDedicatedServer, Error, TEXT("[Auth] ERROR: %s"), *error_message);
        return false;
      }
      else {
        UE_LOG(LogFunapiDedicatedServer, Log, TEXT("[Auth] token: %s"), *token);
      }

      bool ret = false;
      FString* t = auth_map_.Find(uid);
      if (t) {
        if (token.Compare(*t) == 0) {
          ret = true;
        }
      }

      if (ret == false) {
        error_message = FString("There is no valid user id & token in this dedicated server.");
        UE_LOG(LogFunapiDedicatedServer, Error, TEXT("[Auth] ERROR: %s"), *error_message);
      }

      return ret;
    }

  }
}
