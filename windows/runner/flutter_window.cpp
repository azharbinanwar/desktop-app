#include "flutter_window.h"

#include <optional>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>
#include <windows.h>
#include <string>

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
                    // Load C# DLL
                    HMODULE module = LoadLibrary(TEXT("CSharpFormSaver.dll"));
                    if (module == NULL) {
                        result->Error("DLL_ERROR", "Failed to load CSharpFormSaver.dll");
                        return;
                    }

                    // Handle form data operations
                    if (call.method_name() == "saveFormData") {
                        // Extract form data from arguments
                        const auto* args = std::get_if<flutter::EncodableMap>(call.arguments());
                        if (!args) {
                            result->Error("INVALID_ARGS", "Invalid arguments");
                            return;
                        }

                        // Execute C# function
                        // Implementation omitted for brevity
                        result->Success(flutter::EncodableValue(true));
                    } else { // getFormData
                        // Implementation omitted for brevity
                        flutter::EncodableList records;
                        result->Success(flutter::EncodableValue(records));
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
