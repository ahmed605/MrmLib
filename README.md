# MrmLib

**MrmLib** is a WinRT Component library for working with **PRI** (Package Resource Index) files, it can be used to read and modify resources in **PRI** files.

**MrmLib** was originally developed to be used in conjunction with [DynamicXAML](https://github.com/ahmed605/DynamicXaml), the name "Mrm" comes from the system component used for processing **PRI** files which is called "**MRM**" (Modern Resource Manager), it's also sometimes referred to as **MRT** (Modern Resource Technology).

## Installation

[![MrmLib](https://img.shields.io/nuget/v/MrmLib?style=for-the-badge&label=MrmLib&color=blue&link=https%3A%2F%2Fwww.nuget.org%2Fpackages%2FMrmLib%2F)](https://www.nuget.org/packages/MrmLib/)

## Usage

### Loading a PRI file

```csharp
using MrmLib;

...

StorageFile priStorageFile = ...;
var priFile = await PriFile.LoadAsync(priStorageFile);

string priPath = ...;
var priFile = await PriFile.LoadAsync(priPath);

byte[] priData = ...;
var priFile = await PriFile.LoadAsync(priData);
```

### Reading resources

```csharp
using MrmLib;

...

var priFile = await PriFile.LoadAsync(...);
foreach (var candidate in priFile.ResourceCandidates)
{
    Console.Write($"Resource Name: {candidate.ResourceName}, Resource Type: {candidate.ValueType}");
    
    if (candidate.ValueType == ResourceValueType.EmbeddedData)
    {
        byte[] data = candidate.DataValue
        Console.Write($", Data Value Length: {data.Length} bytes");
    }
    else
    {
        string data = candidate.StringData;
        Console.Write($", String Value: {data}");
    }

    Console.Write($", Qualifiers Count: {candidate.Qualifiers.Count}");
    Console.WriteLine($", Value as System.Object: {candidate.Value}");
}
```

### Modifying resources
```csharp
using MrmLib;

...

var priFile = await PriFile.LoadAsync(...);
var candidate = priFile.ResourceCandidates[0];


candidate.SetValue("New String Value");


candidate.SetValue(candidate, "Assets\\NewFilePath.png");


candidate.SetValue(candidate, ResourceValueType.Path, "Assets\\NewFilePath.png");


byte[] newData = ...;
candidate.SetValue(candidate, newData);


candidate.StringValue = "Another Way To Set a New String Value";
candidate.ValueType = ResourceValueType.String;


StorageFolder assetsFolder = ...;
var result = await priFile.ReplacePathCandidatesWithEmbeddedDataAsync(assetsFolder);


string assetsFolderPath = ...;
var result = await priFile.ReplacePathCandidatesWithEmbeddedDataAsync(assetsFolderPath);
```

### Adding new resources
```csharp
using MrmLib;

...

var priFile = await PriFile.LoadAsync(...);

var customResourceName = "Files/CustomImageResource.png";
using var client = new System.Net.Http.HttpClient();
client.DefaultRequestHeaders.UserAgent.ParseAdd("Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
var bytes = await client.GetByteArrayAsync("https://upload.wikimedia.org/wikipedia/en/0/02/Homer_Simpson_2006.png");
var customCandidate = ResourceCandidate.Create(customResourceName, bytes);
priFile.ResourceCandidates.Add(customCandidate);

var localizedResourceName = "Resources/CustomLocalizedResource";
var qualiferEn = Qualifier.Create(QualifierAttribute.LanguageList, "en-US");
var qualiferFr = Qualifier.Create(QualifierAttribute.LanguageList, "fr-FR");
var candidateEn = ResourceCandidate.Create(localizedResourceName, ResourceValueType.String, "Hello!", new[] { qualiferEn });
var candidateFr = ResourceCandidate.Create(localizedResourceName, ResourceValueType.String, "Bonjour!", new[] { qualiferFr });
priFile.ResourceCandidates.Add(candidateEn);
priFile.ResourceCandidates.Add(candidateFr);
```

### Saving the modified PRI file
```csharp
using MrmLib;

...

var priFile = await PriFile.LoadAsync(...);

StorageFile file = ...;
await priFile.WriteAsync(file);

string filePath = ...;
await priFile.WriteAsync(filePath);

IOutputStream outputStream = ...;
await priFile.WriteAsync(outputStream);

byte[] priData = priFile.Write();
```