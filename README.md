ShooterGame
========================

Epic Games 에서 제공하는 [ShooterGame](https://docs.unrealengine.com/latest/INT/Resources/SampleGames/ShooterGame/index.html) 샘플 게임 기반으로 데디케이트 서버 연동 예를 보여주는 프로젝트입니다.

## 주의 사항
github 의 용량 제한 문제로 Content 폴더가 빠져있습니다. 예제 실행을 위해서는 **에픽 게임즈 런처**에서 슈터 게임 예제를 다운 받으신 후 Content 폴더를 복사하셔야 합니다

## 아이펀 엔진 서버 설정
[http://www.ifunfactory.com/engine/documents/reference/ko/dedicated-server.html](http://www.ifunfactory.com/engine/documents/reference/ko/dedicated-server.html)

## UE4 플러그인

``ShooterGame`` 폴더의 ``Plugins`` 폴더안에 ``Funapi``와 ``FunapiDedicatedServer`` 플러그인이 포함되어 있는 것을 확인할 수 있습니다.

``Funapi`` 플러그인은 **게임 클라이언트**가 **아이펀서버**와 통신할 때 사용하는 플러그인이고
``FunapiDedicatedServer`` 플러그인은 **데디케이트 서버**가 **데디케이트 서버 매니저**와 통신할 때 사용하는 플러그인 입니다.

## 실행 방법

테스트를 위해 서버와 클라이언트에 파라미터를 주고 실행하는 방법 입니다.

아이펀 서버와 데디케이트 서버 매니저를 통해 실행할때는 [아이펀 엔진 서버 설정](http://www.ifunfactory.com/engine/documents/reference/ko/dedicated-server.html)을 참고하세요.

### 데디케이트 서버
```
"C:\Program Files\Epic Games\UE_4.15\Engine\Binaries\Win64\UE4Editor.exe" "C:\work\apps-ue4-dedi-server-example\ShooterGame\ShooterGame.uproject" HighRise?game=FFA -skipcompile -server -log -port=7777 -BeaconPort=15000 -FunapiMatchID="53d88031-cd49-432e-826a-a5ff6b277250" -FunapiManagerServer="http://harida-vm.ifunfactory.com:8000" -FunapiHeartbeat=10
```
- **FunapiMatchID**
    - 매치 아이디
- **FunapiManagerServer**
    - 데디케이티드 서버 매니저 주소
- **FunapiHeartbeat**
    - Heartbeat HTTP POST 를 보내는 주기(초), 0 이거나 없으면 보내지 않습니다
- **FunapiVersion**
    - 커맨드라인에 이 옵션이 들어있을 경우, **데디케이트 서버**는 처음 실행될 때 **데디케이트 서버 매니저**에 버전 정보를 전송하고 종료합니다

### 게임 클라이언트
```
"C:\test\game\WindowsNoEditor\ShooterGame.exe" -FunapiServer="127.0.0.1" -FunapiServerPort=8012
```
- **FunapiServer**
    - 아이펀 서버 주소
- **FunapiServerPort**
    - 아이펀 서버 포트 번호

이 테스트 프로젝트는 파라미터를 받아서 **아이펀 서버**에 접속을 합니다.

**게임 클라이언트**가 **아이펀 서버**를 통해 *세션 아이디*가 초기화 될 때 *테스트용 메시지*를 보내고
메시지를 받은 **아이펀 서버**는 **데디케이티드 서버 매니저**를 통해 **데디케이트 서버**를 실행합니다.

**데디케이트 서버**가 게임을 생성한 후 *ready 메시지*를 **데디케이티드 서버 매니저**로 보내면
**아이펀 서버**가 **게임 클라이언트**에게 접속할 *데디케이트 서버의 주소와 포트 번호등을 포함한 정보*를 보내주고
**게임 클라이언트**가 그 정보를 바탕으로 **데디케이티드 서버**에 접속하게 됩니다.

## uproject

**uproject** 파일에서 플러그인을 사용하도록 설정해야 합니다.

```json
{
    "Plugins": [
        {
            "Name": "FunapiDedicatedServer",
            "Enabled": true
        },
        {
            "Name": "Funapi",
            "Enabled": true
        }
    ]
}
```

## 빌드 스크립트

**ShooterGame.Build.cs** 파일을 수정해서 Linux 빌드와 Standalone 서버 빌드가 둘 다 아닐때에만 ``Funapi`` 플러그인을 포함하도록 해야 합니다.

```csharp
if (Target.Type != TargetRules.TargetType.Server && Target.Platform != UnrealTargetPlatform.Linux)
{
    PrivateDependencyModuleNames.Add("Funapi");
}
```

## 전처리기 지시문

서버 빌드와 리눅스 빌드일때는 ``Funapi`` 플러그인이 포함되지 않아야 하기 때문에
게임 클라이언트에서 ``Funapi`` 플러그인을 사용하는 코드에는 전처리기 지시문을 사용해서 빌드에 포함되지 않도록 해야 합니다.

```c++
#if WITH_FUNAPI
#include "funapi_session.h"
#include "funapi_tasks.h"
#endif
```

## FunapiDedicatedServer

```c++
bool ParseConsoleCommand(const TCHAR* cmd, const FString &match_id_field, const FString &manager_server_field);
bool ParseConsoleCommand(const TCHAR* cmd, const FString &match_id_field, const FString &manager_server_field, const FString &heartbeat_field);
bool ParseConsoleCommand(const TCHAR* cmd, const FString &match_id_field, const FString &manager_server_field, const FString &heartbeat_field, const FString &version_field);

fun::FunapiDedicatedServer::ParseConsoleCommand(FCommandLine::Get());
fun::FunapiDedicatedServer::ParseConsoleCommand(FCommandLine::Get(), "FunapiMatchID", "FunapiManagerServer");
fun::FunapiDedicatedServer::ParseConsoleCommand(FCommandLine::Get(), "FunapiMatchID", "FunapiManagerServer", "FunapiHeartbeat");
fun::FunapiDedicatedServer::ParseConsoleCommand(FCommandLine::Get(), "FunapiMatchID", "FunapiManagerServer", "FunapiHeartbeat", "FunapiVersion");
```

엔진 기본 커맨드 라인이 앞으로 바뀔 수 있기 때문에 고정된 이름을 사용하지 않고 파라미터를 받도록 합니다.

FunapiMatchID 필드와 FunapiManagerServer 필드가 없으면 false 를 리턴하고 있으면 true를 리턴합니다.

- **cmd**
    - 서버의 커맨드 라인 명령
- **match_id_field**
    - 매치 아이디 필드 이름, 기본값 "FunapiMatchID"
- **manager_server_field**
    - 파이썬 매니저 서버의 주소, 기본값 "FunapiManagerServer"
- **heartbeat_field**
    - Heart Beat 을 보내는 주기(초), 기본값 "FunapiHeartbeat"
- **version_field**
    - 버전 정보를 보내고 서버를 종료하는 기능을 사용, 기본값 "FunapiVersion"
    - 이 필드가 존재하면 다른 GET, POST 요청을 보내지 않습니다

```c++
void SetVersionInfo(const FString &version_string);

fun::FunapiDedicatedServer::SetVersionInfo(FString("1.2.3.2500"));
```

커맨드 라인에 버전 정보 필드가 있는 경우 (기본값 -FunapiVersion)

이 함수를 통해 설정된 버전 스트링을 파이썬 매니저 서버에 전송하고 서버를 종료합니다

이 값이 설정되어 있더라도 버전 정보 필드가 설정되어 있지 않으면 전송하지 않고 종료도 하지 않습니다


```c++
void GetGameInfo(const TFunction<void(FHttpResponsePtr response)> &completion_handler);
```

파이썬 매니저 서버에 설정 JSON 을 요청합니다.

설정 JSON 을 다 가져온 이후에 초기화를 진행해야 하기 때문에 HTTP 요청 완료후 초기화 함수를 실행하도록
completion handler 를 파라미터로 받습니다.

```c++
void PostReady();

fun::FunapiDedicatedServer::PostReady();
```

데디케이트 서버가 유저의 접속을 받을 준비가 되면 파이썬 서버에 ready를 보냅니다.

이 예제 프로젝트는 게임모드가 초기화 될 때를 기준으로 테스트 코드가 들어가 있습니다.

```c++
bool AuthUser(const FString& options, const FString& uid_field, const FString& token_field, FString &error_message);

fun::FunapiDedicatedServer::AuthUser(Options, "FunapiUID", "FunapiToken", ErrorMessage);
fun::FunapiDedicatedServer::AuthUser(Options, ErrorMessage);
```

PreLogin 단계에서 유효한 유저인지 확인을 합니다.

Get을 통해 저장된 uid 와 token 이 일치하는지 확인을 합니다.

엔진의 기본 스펙이 바뀔 수 있기 때문에 마찬가지로 필드 이름을 파라미터로 받습니다.

인증에 실패하면 false 를 리턴하고 에러 내용을 error_message 에 넣습니다.

error_message가 empty가 아니면 에러로 판단하는건 엔진의 기본 기능이기 때문에 PreLogin에서 여기서 설정한 error_message를 사용해도 됩니다.

- **options**
    - PreLogin 단계에서 넘어오는 파라미터값 스트링
- **uid_field**
    - 유저아이디 필드 이름, 기본값 "FunapiUID"
- **token_field**
    - 토큰 필드 이름, 기본값 "FunapiToken"
- **error_message**
    - 에러 메시지 스트링

```c++
void PostResult(const FString &json_string, const bool use_exit);

fun::FunapiDedicatedServer::PostResult(FString("{ \"message\":\"result\"}"), false);
```

게임이 끝나면 결과 내용을 담은 JSON 스트링을 **데디케이티드 서버 매니저**에 보냅니다.

use_exit 파라미터가 true 이면 JSON 스트링을 보내고 서버를 종료합니다.

false 이면 종료하지 않습니다.

```c++
FString GetUserDataJsonString(const FString &uid);

FString user_data_json_string = fun::FunapiDedicatedServer::GetUserDataJsonString(uid);
```

해당 유저의 정보를 JSON 문자열의 형태로 가져옵니다.

```c++
FString GetMatchDataJsonString();

FString game_data_json_string = fun::FunapiDedicatedServer::GetMatchDataJsonString();
```

게임 정보를 JSON 문자열의 형태로 가져옵니다.

```c++
void SetUserDataCallback(const TFunction<void(const FString &uid, const FString &json_string)> &handler);

fun::FunapiDedicatedServer::SetUserDataCallback([](const FString &uid, const FString &user_data_json_string) {
  UE_LOG(LogTemp, Log, TEXT("%s = %s"), *(uid), *(user_data_json_string));
});
```

유저 데이터가 변경 되었을 때 콜백을 받습니다.

```c++
void SetMatchDataCallback(const TFunction<void(const FString &json_string)> &handler);

fun::FunapiDedicatedServer::SetMatchDataCallback([](const FString &match_data_json_string) {
  UE_LOG(LogTemp, Log, TEXT("match_data = %s"), *(match_data_json_string));
});
```

게임 데이터가 변경 되었을 때 콜백을 받습니다.

```c++
void PostJoined(const FString &uid);

fun::FunapiDedicatedServer::PostJoined(uid);
```

유저가 로그인 했을때 uid 를 **데디케이티드 서버 매니저**에 보냅니다.

```c++
void PostLeft(const FString &uid);

fun::FunapiDedicatedServer::PostLeft(uid);
```

유저가 로그아웃 했을때 uid 를 **데디케이티드 서버 매니저**에 보냅니다.

```c++
void PostCustomCallback(const FString &json_string);

fun::FunapiDedicatedServer::PostCustomCallback(FString("{ \"message\":\"callback\"}"));
```

**데디케이티드 서버 매니저**에 콜백을 보냅니다.

### 에디터 모드 호환성
에디터 모드에서도 서버 로직이 돌아가도록 하기 위해서 전처리기 지시문으로 처리하지 않고
커맨드 라인 파싱을 통해 서버 주소가 제대로 초기화 되었는지를 확인하는 기준으로 플러그인의 동작을 판단하도록 합니다.

## Funapi

클라이언트 사이드에서 메시지 수신 콜백 함수에 데디케이트 서버 접속 코드를 넣어야 합니다.

이 부분은 언리얼 엔진 코드를 사용하고 게임마다 URL의 스펙이 다를 수 있기 때문에 플러그인이 아니라 게임에서 치리해야 합니다.

```c++
// JSON
session_->AddJsonRecvCallback([this](const std::shared_ptr<fun::FunapiSession> &session,
                                    const fun::TransportProtocol transport_protocol,
                                    const std::string &msg_type,
                                    const std::string &json_string) {
    if (msg_type.compare("_sc_dedicated_server") == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("'_sc_dedicated_server' msg recved: "));
        TSharedRef<TJsonReader<TCHAR>> reader = TJsonReaderFactory<TCHAR>::Create(FString(json_string.c_str()));
        TSharedPtr<FJsonObject> json_object = MakeShareable(new FJsonObject);
        FJsonSerializer::Deserialize(reader, json_object);

        UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(json_string.c_str()));

        TSharedPtr<FJsonObject> redirect_json_object = json_object->GetObjectField(FString("redirect"));
        if (redirect_json_object.Get() != nullptr)
        {
            UE_LOG(LogTemp, Log, TEXT("Getting redirect object succeed."));

            FString host_ip = redirect_json_object->GetStringField(FString("host"));
            int32 host_port = redirect_json_object->GetIntegerField(FString("port"));
            FString token = redirect_json_object->GetStringField(FString("token"));

            // test code
            // host_ip = "127.0.0.1";
            // host_port = 7777;
            // token = "8fbac64d60401c6fc0e0ae060e78c7ae";
            // playerId = "E886895D4324A6F0521BC6A8CA6645C1";
            // //

            UE_LOG(LogTemp, Log, TEXT("host: %s:%d, token: %s"), *host_ip, host_port, *token);

            FString host_addr = host_ip;
            host_addr += FString(":");
            host_addr += FString::FromInt(host_port);
            host_addr += FString("?FunapiUID=");
            host_addr += playerId;
            host_addr += FString("?FunapiToken=");
            host_addr += token;

            //
            TestRedirect(host_addr);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Getting redirect object failed."));
        }
    }
});

// protocol buffer
session_->AddProtobufRecvCallback([this](const std::shared_ptr<fun::FunapiSession> &session,
                                        const fun::TransportProtocol transport_protocol,
                                        const FunMessage &fun_message) {
    if (fun_message.msgtype().compare("_sc_dedicated_server") == 0) {
        FunDedicatedServerMesseage message = fun_message.GetExtension(_sc_dedicated_server);

        FString host_ip(message.redirect().host().c_str());
        int32 host_port = message.redirect().port();
        FString token(message.redirect().token().c_str());

        // test code
        // host_ip = "127.0.0.1";
        // host_port = 7777;
        // token = "8fbac64d60401c6fc0e0ae060e78c7ae";
        // playerId = "E886895D4324A6F0521BC6A8CA6645C1";
        // //

        UE_LOG(LogTemp, Log, TEXT("host: %s:%d, token: %s"), *host_ip, host_port, *token);

        FString host_addr = host_ip;
        host_addr += FString(":");
        host_addr += FString::FromInt(host_port);
        host_addr += FString("?FunapiUID=");
        host_addr += playerId;
        host_addr += FString("?FunapiToken=");
        host_addr += token;

        //
        TestRedirect(host_addr);
    }
});
```

## 도움말

클라이언트 플러그인의 도움말은 <https://www.ifunfactory.com/engine/documents/reference/ko/client-plugin.html> 를 참고해 주세요.

플러그인에 대한 궁금한 점은 <https://answers.ifunfactory.com> 에 질문을 올려주세요.
가능한 빠르게 답변해 드립니다.

그 외에 플러그인에 대한 문의 사항이나 버그 신고는 <funapi-support@ifunfactory.com> 으로 메일을
보내주세요.
