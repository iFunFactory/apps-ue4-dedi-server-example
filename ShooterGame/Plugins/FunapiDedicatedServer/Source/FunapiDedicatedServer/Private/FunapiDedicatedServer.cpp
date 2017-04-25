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
    static TMap<FString, FString> user_data_map_;
    static double heartbeat_seconds_ = 0;
    static double pending_users_seconds_ = 5;

    FString GetUserDataJsonString(const FString &uid)
    {
      FString* user_data_json_string = user_data_map_.Find(uid);
      if (user_data_json_string) {
        return FString(*user_data_json_string);
      }

      return "";
    }

    void AddMap(const TSharedPtr<FJsonObject> json_object)
    {
      TArray<TSharedPtr<FJsonValue>> users;
      int users_count = 0;

      if (json_object->HasField(FString("users")))
      {
        users = json_object->GetArrayField(FString("users"));
        users_count = users.Num();
      }

      TArray<TSharedPtr<FJsonValue>> user_data;
      int user_data_count = 0;

      if (json_object->HasField(FString("user_data")))
      {
        user_data = json_object->GetArrayField(FString("user_data"));
        user_data_count = user_data.Num();
      }

      for (int i = 0; i < users_count; ++i) {
        TSharedPtr<FJsonObject> o = users[i]->AsObject();

        FString uid = o->GetStringField(FString("uid"));
        FString token = o->GetStringField(FString("token"));

        UE_LOG(LogFunapiDedicatedServer, Log, TEXT("%s, %s"), *uid, *token);

        auth_map_.Add(uid, token);

        if (i < user_data_count) {
          FString user_data_json_string;

          if (false == user_data[i]->TryGetString(user_data_json_string)) {
            TSharedPtr<FJsonObject> user_data_object = user_data[i]->AsObject();
            // Convert JSON document to string
            TSharedRef<TJsonWriter<TCHAR>> writer = TJsonWriterFactory<TCHAR>::Create(&user_data_json_string);
            FJsonSerializer::Serialize(user_data_object.ToSharedRef(), writer);
          }

          user_data_map_.Add(uid, user_data_json_string);
        }
      }
    }

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

    void PostPendingUsers();
    void AsyncPendingUsers()
    {
      class FAsyncPendingUsersTask : public FNonAbandonableTask
      {
      public:
        /** Performs work on thread */
        void DoWork()
        {
          FPlatformProcess::Sleep(pending_users_seconds_);
          PostPendingUsers();
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

      (new FAutoDeleteAsyncTask<FAsyncPendingUsersTask>)->StartBackgroundTask();
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

        AsyncPendingUsers();
      }

      return ret;
    }

    bool ParseConsoleCommand(const TCHAR* cmd)
    {
      return ParseConsoleCommand(cmd, "FunapiMatchID", "FunapiManagerServer", "FunapiHeartbeat");
    }

    void Request(const FString &verb,
      const FString &path,
      const FString &json_string,
      const TFunction<void(FHttpRequestPtr request, FHttpResponsePtr response, bool succeed)> &completion_handler)
    {
      if (funapi_manager_server_.IsEmpty()) {
        return;
      }

      FString server_url = funapi_manager_server_ + path;

      /*
      // test code
      if (verb == FString("GET")) {
        // test // url
        // server_url = "http://www.mocky.io/v2/5844042111000073010e6b1e";
        // server_url = "http://www.mocky.io/v2/58fdbad40f0000510c08b90a";
        // server_url = "http://www.mocky.io/v2/58fdd4260f0000e10e08b954";
        server_url = "http://www.mocky.io/v2/58fdd8d90f0000660f08b970";
      }
      // // // //
      */

      auto http_request = FHttpModule::Get().CreateRequest();
      http_request->SetURL(server_url);
      http_request->SetVerb(verb);
      http_request->SetHeader(FString("Content-Type"), FString("application/json; charset=utf-8"));
      http_request->SetHeader(FString("Content-Length"), FString::FromInt(json_string.Len()));
      http_request->SetContentAsString(json_string);

      http_request->OnProcessRequestComplete().BindLambda(
        [verb, path, completion_handler](FHttpRequestPtr request, FHttpResponsePtr response, bool succeed) {
        if (!succeed) {
          UE_LOG(LogFunapiDedicatedServer, Error, TEXT("%s %s : Response was invalid!"), *verb, *path);
        }
        else {
          FString json_fstring = response->GetContentAsString();
          UE_LOG(LogFunapiDedicatedServer, Log, TEXT("%s %s : Response = %s"), *verb, *path, *json_fstring);
        }

        completion_handler(request, response, succeed);
      });

      http_request->ProcessRequest();
    }

    void Get(const FString &path, const FString &json_string, const TFunction<void(FHttpRequestPtr request, FHttpResponsePtr response, bool succeed)> &completion_handler)
    {
      Request("GET", path, json_string, completion_handler);
    }

    void GetGameInfo(const TFunction<void(FHttpRequestPtr request, FHttpResponsePtr response, bool succeed)> &completion_handler) {
      if (funapi_manager_server_.IsEmpty()) {
        completion_handler(nullptr, nullptr, false);
        return;
      }

      Get("", "",
        [completion_handler](FHttpRequestPtr request, FHttpResponsePtr response, bool succeed)
      {
        auth_map_.Empty();

        if (succeed)
        {
          FString json_fstring = response->GetContentAsString();
          TSharedRef< TJsonReader<> > reader = TJsonReaderFactory<>::Create(json_fstring);
          TSharedPtr<FJsonObject> json_object;

          if (FJsonSerializer::Deserialize(reader, json_object))
          {
            TSharedPtr<FJsonObject> data = json_object->GetObjectField(FString("data"));
            AddMap(data);
          }
        }

        completion_handler(request, response, succeed);
      });
    }

    void GetGameInfo(const TFunction<void(FHttpResponsePtr response)> &completion_handler) {
      TFunction<void(FHttpRequestPtr request, FHttpResponsePtr response, bool succeed)> handler =
        [completion_handler](FHttpRequestPtr request, FHttpResponsePtr response, bool succeed){
        completion_handler(response);
      };

      GetGameInfo(handler);
    }

    void Post(const FString &path, const FString &json_string, const TFunction<void(FHttpRequestPtr request, FHttpResponsePtr response, bool succeed)> &completion_handler)
    {
      Request("POST", path, json_string, completion_handler);
    }

    void PostReady()
    {
      Post("ready");
    }

    void PostResult(const FString &json_string, const bool use_exit)
    {
      Post("result", json_string,
        [use_exit](FHttpRequestPtr request, FHttpResponsePtr response, bool succeed)
      {
        if (succeed && use_exit) {
          FGenericPlatformMisc::RequestExit(false);
        }
      });
    }

    void PostHeartbeat() {
      Post("heartbeat", "",
        [](FHttpRequestPtr request, FHttpResponsePtr response, bool succeed)
      {
        AsyncHeartbeat();
      });
    }

    void PostGameState(const FString &json_string)
    {
      Post("state", json_string);
    }

    void PostPendingUsers()
    {
      Post("pending_users", "",
        [](FHttpRequestPtr request, FHttpResponsePtr response, bool succeed)
      {
        if (succeed) {
          FString json_fstring = response->GetContentAsString();
          TSharedRef< TJsonReader<> > reader = TJsonReaderFactory<>::Create(json_fstring);
          TSharedPtr<FJsonObject> json_object;
          if (FJsonSerializer::Deserialize(reader, json_object))
          {
            AddMap(json_object);
          }
        }

        AsyncPendingUsers();
      });
    }

    FString JsonFStringWithUID(const FString &uid) {
      TSharedRef<FJsonObject> json_object = MakeShareable(new FJsonObject);
      json_object->SetStringField(FString("name"), uid);

      // Convert JSON document to string
      FString ouput_fstring;
      TSharedRef<TJsonWriter<TCHAR>> writer = TJsonWriterFactory<TCHAR>::Create(&ouput_fstring);
      FJsonSerializer::Serialize(json_object, writer);

      return ouput_fstring;
    }

    void PostLogin(const FString &uid)
    {
      Post("login", JsonFStringWithUID(uid));
    }

    void PostLogout(const FString &uid)
    {
      Post("logout", JsonFStringWithUID(uid));
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
