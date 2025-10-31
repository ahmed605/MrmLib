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
                var type = resources[0].ValueType;
                var value = resources[0].Value;
                var data = resources[0].DataValue;
                var qualifiers = resources[0].Qualifiers;

                var folderPicker = new FolderPicker();
                folderPicker.FileTypeFilter.Add("*");
                if (await folderPicker.PickSingleFolderAsync() is { } folder)
                {
                    var result = await pri.ReplacePathCandidatesWithEmbeddedDataAsync(folder);
                    var candidatesReplaced = result.CandidatesReplaced;

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
