// Copyright (C) 2013-2016 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "FunapiDedicatedServerPrivatePCH.h"
#include "FunapiDedicatedServer.h"

namespace fun {

  static FString funapi_manager_server;
  static TMap<FString, FString> auth_map;

  bool FunapiDedicatedServer::CommandLine(const TCHAR* cmd, const FString &match_id_field, const FString &manager_server_field)
  {
    bool ret = true;

    FString match_id;
    FString manager_server;

    if (FParse::Value(cmd, *match_id_field, match_id))
    {
      match_id = match_id.Replace(TEXT("="), TEXT("")).Replace(TEXT("\""), TEXT("")); // replace quotes
      UE_LOG(LogFunapiDedicatedServer, Log, TEXT("match_id is '%s'"), *match_id);
    }
    else {
      ret = false;
    }

    if (FParse::Value(cmd, *manager_server_field, manager_server))
    {
      manager_server = manager_server.Replace(TEXT("="), TEXT("")).Replace(TEXT("\""), TEXT("")); // replace quotes
      UE_LOG(LogFunapiDedicatedServer, Log, TEXT("manager_server is '%s'"), *manager_server);
    }
    else {
      ret = false;
    }

    if (ret) {
      funapi_manager_server = manager_server + "/match/" + match_id + "/";
      UE_LOG(LogFunapiDedicatedServer, Log, TEXT("Dedicated server manager server : %s"), *funapi_manager_server);
    }

    return ret;
  }

  void FunapiDedicatedServer::Get(const TFunction<void(FHttpResponsePtr response)> &completion_handler) {
    if (funapi_manager_server.IsEmpty()) {
      completion_handler(nullptr);
      return;
    }

    FString server_url = funapi_manager_server;
    // server_url = "http://www.mocky.io/v2/5844042111000073010e6b1e"; // test url

    auto http_request = FHttpModule::Get().CreateRequest();
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
          auth_map.Empty();

          TSharedPtr<FJsonObject> data = json_object->GetObjectField(FString("data"));
          TArray<TSharedPtr<FJsonValue>> users = data->GetArrayField(FString("users"));

          int len = users.Num();
          for (int i = 0; i < len; ++i) {
            auto o = users[i]->AsObject();

            auto uid = o->GetStringField(FString("uid"));
            auto token = o->GetStringField(FString("token"));

            UE_LOG(LogFunapiDedicatedServer, Log, TEXT("%s, %s"), *uid, *token);

            auth_map.Add(uid, token);
          }
        }

        completion_handler(response);
      }
    });

    http_request->ProcessRequest();
  }

  void FunapiDedicatedServer::Ready()
  {
    if (funapi_manager_server.IsEmpty()) {
      return;
    }

    FString server_url = funapi_manager_server + "ready";

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

  void FunapiDedicatedServer::Result(const FString &json_string, const bool use_exit)
  {
    if (funapi_manager_server.IsEmpty()) {
      return;
    }

    FString server_url = funapi_manager_server + "result";

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

  bool FunapiDedicatedServer::Auth(const FString& options, const FString& uid_field, const FString& token_field, FString &error_message)
  {
    if (funapi_manager_server.IsEmpty()) {
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
      UE_LOG(LogFunapiDedicatedServer, Log, TEXT("[PreLogin] token: %s"), *token);
    }

    bool ret = false;
    FString* t = auth_map.Find(uid);
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
