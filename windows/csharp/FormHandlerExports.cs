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
                string response = JsonSerializer.Serialize(new {
                    success = result,
                    message = "Data saved successfully",
                    error = ""
                });
                return Marshal.StringToCoTaskMemUTF8(response);
            }
            catch (Exception ex)
            {
                string response = JsonSerializer.Serialize(new {
                    success = false,
                    message = "",
                    error = ex.Message
                });
                return Marshal.StringToCoTaskMemUTF8(response);
            }
        }

        [UnmanagedCallersOnly(EntryPoint = "GetFormData")]
        public static IntPtr GetFormData()
        {
            try
            {
                var data = FormHandler.GetFormDataInternal();
                string json = JsonSerializer.Serialize(new {
                    success = true,
                    data = data,
                    error = ""
                });

                // Allocate memory that won't be garbage collected
                IntPtr ptr = Marshal.StringToCoTaskMemUTF8(json);
                return ptr;
            }
            catch (Exception ex)
            {
                string errorJson = JsonSerializer.Serialize(new {
                    success = false,
                    data = new List<Dictionary<string, string>>(),
                    error = ex.Message
                });
                return Marshal.StringToCoTaskMemUTF8(errorJson);
            }
        }
    }
}
