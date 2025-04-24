#include "flutter_window.h"

#include <optional>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>
#include <windows.h>
#include <string>
#include <Shlwapi.h>  // Added for path functions

#pragma comment(lib, "Shlwapi.lib")  // Link with Shlwapi.lib for path functions

#include "flutter/generated_plugin_registrant.h"

FlutterWindow::FlutterWindow(const flutter::DartProject& project)
        : project_(project) {}

FlutterWindow::~FlutterWindow() {}

bool FlutterWindow::OnCreate() {
    if (!Win32Window::OnCreate()) {
        return false;
    }

    RECT frame = GetClientArea();

    flutter_controller_ = std::make_unique<flutter::FlutterViewController>(
            frame.right - frame.left, frame.bottom - frame.top, project_);
    if (!flutter_controller_->engine() || !flutter_controller_->view()) {
        return false;
    }
    RegisterPlugins(flutter_controller_->engine());

    // System info channel
    system_info_channel_ = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            flutter_controller_->engine()->messenger(),
            "com.example.my_app/system_info",
            &flutter::StandardMethodCodec::GetInstance());

    system_info_channel_->SetMethodCallHandler(
            [](const flutter::MethodCall<flutter::EncodableValue>& call,
               std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
                if (call.method_name() == "getProcessorInfo") {
                    // Existing system info code
                    SYSTEM_INFO sysInfo;
                    GetSystemInfo(&sysInfo);

                    char processorName[64] = "Unknown Processor";
                    HKEY hKey;
                    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                      "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                        DWORD dwSize = sizeof(processorName);
                        RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL,
                                         (LPBYTE)processorName, &dwSize);
                        RegCloseKey(hKey);
                    }

                    char winVer[32];
                    OSVERSIONINFOA osvi;
                    ZeroMemory(&osvi, sizeof(OSVERSIONINFOA));
                    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
#pragma warning(suppress : 4996)
                    GetVersionExA(&osvi);
                    sprintf_s(winVer, "Windows %d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion);

                    std::string systemInfo = std::string(processorName) + " on " + std::string(winVer);
                    result->Success(flutter::EncodableValue(systemInfo));
                } else {
                    result->NotImplemented();
                }
            });

    // Form handler channel
    form_handler_channel_ = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            flutter_controller_->engine()->messenger(),
            "com.example.my_app/form_handler",
            &flutter::StandardMethodCodec::GetInstance());

    form_handler_channel_->SetMethodCallHandler(
            [](const flutter::MethodCall<flutter::EncodableValue>& call,
               std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
                if (call.method_name() == "saveFormData" || call.method_name() == "getFormData") {
                    // Try multiple DLL locations
                    HMODULE module = NULL;

                    // Try current directory first
                    module = LoadLibrary(TEXT("CSharpFormSaver.dll"));

                    // If not found, try application directory
                    if (module == NULL) {
                        TCHAR exePath[MAX_PATH];
                        GetModuleFileName(NULL, exePath, MAX_PATH);
                        PathRemoveFileSpec(exePath);
                        TCHAR dllPath[MAX_PATH];
                        // We need to handle TCHAR which could be char or wchar_t
#ifdef UNICODE
                        wsprintf(dllPath, L"%s\\CSharpFormSaver.dll", exePath);
#else
                        sprintf_s(dllPath, "%s\\CSharpFormSaver.dll", exePath);
#endif
                        module = LoadLibrary(dllPath);
                    }

                    // If still not found, report error with last error code
                    if (module == NULL) {
                        DWORD error = GetLastError();
                        char errorMsg[256];
                        sprintf_s(errorMsg, "Failed to load CSharpFormSaver.dll. Error code: %lu", error);
                        result->Error("DLL_ERROR", errorMsg);
                        return;
                    }

                    // Handle form data operations
                    if (call.method_name() == "saveFormData") {
                        // Extract form data from arguments
                        const auto* args = std::get_if<flutter::EncodableMap>(call.arguments());
                        if (!args) {
                            result->Error("INVALID_ARGS", "Invalid arguments");
                            FreeLibrary(module);
                            return;
                        }

                        // Get function pointer from DLL
                        typedef const char* (*SaveFormDataFunc)(const char*, const char*, const char*, const char*);
                        SaveFormDataFunc saveFormData = (SaveFormDataFunc)GetProcAddress(module, "SaveFormData");
                        if (saveFormData == NULL) {
                            DWORD errorCode = GetLastError();
                            char errorMsg[256];
                            sprintf_s(errorMsg, "SaveFormData function not found. Error code: %lu", errorCode);
                            FreeLibrary(module);
                            result->Error("FUNCTION_NOT_FOUND", errorMsg);
                            return;
                        }

                        // Extract parameters
                        std::string name, fullName, location, dob;
                        auto nameIt = args->find(flutter::EncodableValue("name"));
                        if (nameIt != args->end()) {
                            name = std::get<std::string>(nameIt->second);
                        }

                        auto fullNameIt = args->find(flutter::EncodableValue("fullName"));
                        if (fullNameIt != args->end()) {
                            fullName = std::get<std::string>(fullNameIt->second);
                        }

                        auto locationIt = args->find(flutter::EncodableValue("location"));
                        if (locationIt != args->end()) {
                            location = std::get<std::string>(locationIt->second);
                        }

                        auto dobIt = args->find(flutter::EncodableValue("dateOfBirth"));
                        if (dobIt != args->end()) {
                            dob = std::get<std::string>(dobIt->second);
                        }

                        // Call C# function
                        const char* jsonResult = saveFormData(name.c_str(), fullName.c_str(), location.c_str(), dob.c_str());
                        if (jsonResult == NULL) {
                            result->Error("CALL_ERROR", "Failed to get result from SaveFormData");
                        } else {
                            std::string resultStr(jsonResult);
                            // Free the memory allocated by the C# function
                            CoTaskMemFree((LPVOID)jsonResult);
                            result->Success(flutter::EncodableValue(resultStr));
                        }
                    } else { // getFormData
                        // Get function pointer from DLL - explicitly use GetProcAddress
                        typedef const char* (*GetFormDataFunc)();
                        GetFormDataFunc getFormData = (GetFormDataFunc)GetProcAddress(module, "GetFormData");

                        if (getFormData == NULL) {
                            DWORD errorCode = GetLastError();
                            char errorMsg[256];
                            sprintf_s(errorMsg, "GetFormData function not found. Error code: %lu", errorCode);
                            FreeLibrary(module);
                            result->Error("FUNCTION_NOT_FOUND", errorMsg);
                            return;
                        }

                        // Call C# function
                        try {
                            const char* jsonResult = getFormData();
                            if (jsonResult == NULL) {
                                result->Success(flutter::EncodableValue("[]"));
                            } else {
                                std::string resultStr(jsonResult);
                                // Free the memory allocated by the C# function
                                CoTaskMemFree((LPVOID)jsonResult);
                                result->Success(flutter::EncodableValue(resultStr));
                            }
                        } catch (...) {
                            result->Error("CALL_ERROR", "Exception while calling GetFormData");
                        }
                    }

                    FreeLibrary(module);
                } else {
                    result->NotImplemented();
                }
            });

    SetChildContent(flutter_controller_->view()->GetNativeWindow());
    flutter_controller_->engine()->SetNextFrameCallback([&]() {
        this->Show();
    });

    return true;
}

void FlutterWindow::OnDestroy() {
    if (flutter_controller_) {
        flutter_controller_ = nullptr;
    }
    Win32Window::OnDestroy();
}

LRESULT FlutterWindow::MessageHandler(HWND hwnd, UINT const message,
                                      WPARAM const wparam,
                                      LPARAM const lparam) noexcept {
if (flutter_controller_) {
std::optional<LRESULT> result =
        flutter_controller_->HandleTopLevelWindowProc(hwnd, message, wparam, lparam);
if (result) {
return *result;
}
}

switch (message) {
case WM_FONTCHANGE:
flutter_controller_->engine()->ReloadSystemFonts();
break;
}

return Win32Window::MessageHandler(hwnd, message, wparam, lparam);
}
