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
            // Create file info
            var fileInfo = new FileInfo(DataFilePath);
            // Get directory info
            var directory = fileInfo.Directory;
            // Log directory exists status
            Console.WriteLine($"Directory exists: {directory.Exists}");
            // Log full path
            Console.WriteLine($"Full path: {Path.GetFullPath(DataFilePath)}");

            // Create a new form data object
            var formData = new FormData
            {
                Name = name,
                FullName = fullName,
                Location = location,
                DateOfBirth = dob
            };

            // Read existing records
            List<FormData> records = GetAllRecords();

            // Add new record
            records.Add(formData);

            // Save all records back to file
            string jsonString = JsonSerializer.Serialize(records, new JsonSerializerOptions
            {
                WriteIndented = true
            });

            File.WriteAllText(DataFilePath, jsonString);

            return true;
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
            if (!File.Exists(DataFilePath))
            {
                return new List<FormData>();
            }

            string jsonString = File.ReadAllText(DataFilePath);
            if (string.IsNullOrEmpty(jsonString))
            {
                return new List<FormData>();
            }

            return JsonSerializer.Deserialize<List<FormData>>(jsonString);
        }
    }
}
