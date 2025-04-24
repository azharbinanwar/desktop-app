using System.Text.Json.Serialization;

namespace CSharpFormSaver
{
    [JsonSerializable(typeof(FormData))]
    [JsonSerializable(typeof(List<FormData>))]
    [JsonSerializable(typeof(Dictionary<string, string>))]
    [JsonSerializable(typeof(List<Dictionary<string, string>>))]
    [JsonSerializable(typeof(OperationResponse))]
    internal partial class AppJsonContext : JsonSerializerContext
    {
    }

    public class OperationResponse
    {
        public bool Success { get; set; }
        public string Message { get; set; }
        public string Error { get; set; }
        public List<Dictionary<string, string>> Data { get; set; }
    }
}
