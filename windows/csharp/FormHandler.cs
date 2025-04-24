using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;

namespace CSharpFormSaver
{
    public class FormData
    {
        public string Name { get; set; }
        public string FullName { get; set; }
        public string Location { get; set; }
        public string DateOfBirth { get; set; }
    }

    public class FormHandler
    {
        private static readonly string DataFilePath = @"C:\Users\azhar\Desktop\flutter_form_data.json";

        public static bool SaveFormDataInternal(string name, string fullName, string location, string dob)
        {
            try {
                // Create file info
                var fileInfo = new FileInfo(DataFilePath);
                // Make sure directory exists
                Directory.CreateDirectory(fileInfo.Directory.FullName);

                // Create a new form data object
                var formData = new FormData
                {
                    Name = name,
                    FullName = fullName,
                    Location = location,
                    DateOfBirth = dob
                };

                // Read existing records or create empty list if file doesn't exist
                List<FormData> records;
                if (File.Exists(DataFilePath)) {
                    string jsonString = File.ReadAllText(DataFilePath);
                    if (!string.IsNullOrEmpty(jsonString)) {
                        records = JsonSerializer.Deserialize(jsonString, AppJsonContext.Default.ListFormData);
                    } else {
                        records = new List<FormData>();
                    }
                } else {
                    records = new List<FormData>();
                }

                // Add new record
                records.Add(formData);

                // Save all records back to file
                string outputJson = JsonSerializer.Serialize(records, AppJsonContext.Default.ListFormData);
                File.WriteAllText(DataFilePath, outputJson);

                return true;
            } catch (Exception ex) {
                Console.WriteLine($"Error saving form data: {ex.Message}");
                throw; // Rethrow to propagate to caller
            }
        }

        public static List<Dictionary<string, string>> GetFormDataInternal()
        {
            List<FormData> records = GetAllRecords();
            var result = new List<Dictionary<string, string>>();

            foreach (var record in records)
            {
                result.Add(new Dictionary<string, string>
                {
                    { "name", record.Name },
                    { "fullName", record.FullName },
                    { "location", record.Location },
                    { "dateOfBirth", record.DateOfBirth }
                });
            }

            return result;
        }

        private static List<FormData> GetAllRecords()
        {
            if (!File.Exists(DataFilePath)) {
                return new List<FormData>();
            }

            string jsonString = File.ReadAllText(DataFilePath);
            if (string.IsNullOrEmpty(jsonString)) {
                return new List<FormData>();
            }

            return JsonSerializer.Deserialize(jsonString, AppJsonContext.Default.ListFormData);
        }
    }
}
