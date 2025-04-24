using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text.Json;

namespace CSharpFormSaver
{
    public static class FormHandlerExports
    {
        [UnmanagedCallersOnly(EntryPoint = "SaveFormData")]
        public static IntPtr SaveFormData(IntPtr namePtr, IntPtr fullNamePtr, IntPtr locationPtr, IntPtr dobPtr)
        {
            try
            {
                string name = Marshal.PtrToStringUTF8(namePtr);
                string fullName = Marshal.PtrToStringUTF8(fullNamePtr);
                string location = Marshal.PtrToStringUTF8(locationPtr);
                string dob = Marshal.PtrToStringUTF8(dobPtr);

                bool result = FormHandler.SaveFormDataInternal(name, fullName, location, dob);

                var response = new OperationResponse {
                    Success = result,
                    Message = "Data saved successfully",
                    Error = ""
                };

                string jsonResponse = JsonSerializer.Serialize(response, AppJsonContext.Default.OperationResponse);
                return Marshal.StringToCoTaskMemUTF8(jsonResponse);
            }
            catch (Exception ex)
            {
                var response = new OperationResponse {
                    Success = false,
                    Message = "",
                    Error = ex.Message
                };

                string jsonResponse = JsonSerializer.Serialize(response, AppJsonContext.Default.OperationResponse);
                return Marshal.StringToCoTaskMemUTF8(jsonResponse);
            }
        }

        [UnmanagedCallersOnly(EntryPoint = "GetFormData")]
        public static IntPtr GetFormData()
        {
            try
            {
                var data = FormHandler.GetFormDataInternal();

                var response = new OperationResponse {
                    Success = true,
                    Data = data,
                    Error = ""
                };

                string jsonResponse = JsonSerializer.Serialize(response, AppJsonContext.Default.OperationResponse);
                return Marshal.StringToCoTaskMemUTF8(jsonResponse);
            }
            catch (Exception ex)
            {
                var response = new OperationResponse {
                    Success = false,
                    Data = new List<Dictionary<string, string>>(),
                    Error = ex.Message
                };

                string jsonResponse = JsonSerializer.Serialize(response, AppJsonContext.Default.OperationResponse);
                return Marshal.StringToCoTaskMemUTF8(jsonResponse);
            }
        }
    }
}
