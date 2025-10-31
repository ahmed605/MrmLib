# MrmLib

**MrmLib** is a WinRT Component library for working with **PRI** (Package Resource Index) files, it can be used to read and modify resources in **PRI** files.

**MrmLib** was originally developed to be used in conjunction with [DynamicXAML](https://github.com/ahmed605/DynamicXaml), the name "Mrm" comes from the system component used for processing **PRI** files which is called "**MRM**" (Modern Resource Manager), it's also sometimes referred to as **MRT** (Modern Resource Technology).

## Installation

NuGet package will be available SOON™, in the meantime you can clone the repository and build the solution instead.

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

    Console.WriteLine($", Value as System.Object: {candidate.Value}");
}
```

### Modifying resources
```csharp
using MrmLib;

...

var priFile = await PriFile.LoadAsync(...);
var candidate = ...;


priFile.ReplaceCandidateValue(candidate, "New String Value");


priFile.ReplaceCandidateValue(candidate, "Assets\\NewFilePath.png");


priFile.ReplaceCandidateValue(candidate, ResourceValueType.Path, "Assets\\NewFilePath.png");


byte[] newData = ...;
priFile.ReplaceCandidateValue(candidate, newData);


StorageFolder assetsFolder = ...;
var result = await priFile.ReplacePathCandidatesWithEmbeddedDataAsync(assetsFolder);


string assetsFolderPath = ...;
var result = await priFile.ReplacePathCandidatesWithEmbeddedDataAsync(assetsFolderPath);
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