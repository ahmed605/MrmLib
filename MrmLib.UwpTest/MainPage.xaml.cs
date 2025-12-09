using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Storage;
using Windows.Storage.Pickers;
using Windows.UI.Xaml.Controls;
using WinRT;

namespace MrmLib.UwpTest
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a <see cref="Frame">.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            InitializeComponent();
            Loaded += MainPage_Loaded;
        }

        private async void MainPage_Loaded(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            var picker = new FileOpenPicker();
            picker.FileTypeFilter.Add(".pri");

            if (await picker.PickSingleFileAsync() is { } file)
            {
                var pri = await PriFile.LoadAsync(file);
                var resources = pri.ResourceCandidates;
                var name = resources[0].ResourceName;
                var type = resources[0].ValueType;
                var value = resources[0].Value;
                var data = resources[0].DataValue;
                var qualifiers = resources[0].Qualifiers;
                var version = pri.Version;
                var checksum = pri.Checksum;
                var simpleId = pri.SimpleId;
                var uniqueId = pri.UniqueId;
                var magic = pri.Magic;
                var magicString = pri.MagicString;
                var priType = pri.Type;

                //var newId = "new_id_test_123";
                //pri.UniqueId = uniqueId.Replace(simpleId, newId);
                //pri.SimpleId = newId;

                var folderPicker = new FolderPicker();
                folderPicker.FileTypeFilter.Add("*");
                if (await folderPicker.PickSingleFolderAsync() is { } folder)
                {
                    var result = await pri.ReplacePathCandidatesWithEmbeddedDataAsync(folder);
                    var candidatesReplaced = result.CandidatesReplaced;

                    var customResourceName = "Files/CustomImageResource.png";
                    using var client = new System.Net.Http.HttpClient();
                    client.DefaultRequestHeaders.UserAgent.ParseAdd("Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
                    var bytes = await client.GetByteArrayAsync("https://upload.wikimedia.org/wikipedia/en/0/02/Homer_Simpson_2006.png");
                    var customCandidate = ResourceCandidate.Create(customResourceName, bytes);
                    pri.ResourceCandidates.Add(customCandidate);

                    var localizedResourceName = "Resources/CustomLocalizedResource";
                    var qualiferEn = Qualifier.Create(QualifierAttribute.Language, "en-US");
                    var qualiferFr = Qualifier.Create(QualifierAttribute.Language, "fr-FR");
                    var candidateEn = ResourceCandidate.Create(localizedResourceName, ResourceValueType.String, "Hello!", new[] { qualiferEn });
                    var candidateFr = ResourceCandidate.Create(localizedResourceName, ResourceValueType.String, "Bonjour!", new[] { qualiferFr });
                    pri.ResourceCandidates.Add(candidateEn);
                    pri.ResourceCandidates.Add(candidateFr);

                    var fileSavePicker = new FileSavePicker();
                    fileSavePicker.FileTypeChoices.Add("PRI File", new List<string>() { ".pri" });
                    if (await fileSavePicker.PickSaveFileAsync() is { } saveFile)
                    {
                        await pri.WriteAsync(saveFile);
                    }
                }

                GC.KeepAlive(pri);
            }
        }
    }
}
