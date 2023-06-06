using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace WPFMKProto {
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window {
		public MainWindow() {
			InitializeComponent();
			PreviewKeyDown += MainWindow_PreviewKeyDown;
		}

		private void MainWindow_PreviewKeyDown(object sender, KeyEventArgs e) {
			if (e.Key == Key.V && (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control) {
				e.Handled = true;
				var txt = Clipboard.GetText();
				if (txt.Contains("{") && !txt.Contains("}"))
					txt += "\n}\n";
				ConvertText(txt);
			}
		}

		private void Grid_Drop(object sender, DragEventArgs e) {
			if (e.Data.GetDataPresent(DataFormats.FileDrop)) {
				var path = ((string[])e.Data.GetData(DataFormats.FileDrop))[0];
				ConvertFile(path);
			}
		}
		private async void ConvertFile(String path) {
			if (!String.IsNullOrWhiteSpace(path) && File.Exists(path) && (path.EndsWith(".c", StringComparison.CurrentCultureIgnoreCase) || path.EndsWith(".cpp", StringComparison.CurrentCultureIgnoreCase) || path.EndsWith(".h", StringComparison.CurrentCultureIgnoreCase)))
				ConvertText(File.ReadAllText(path));
		}
		private async void ConvertText(String text) {
			Process p = new Process();
			var info = new FileInfo(System.Reflection.Assembly.GetEntryAssembly().Location);
			p.StartInfo.FileName = info.Directory.FullName + "\\mkproto.exe";

			p.StartInfo.ArgumentList.Add("-p");
			p.StartInfo.ArgumentList.Add("-s");
			// p.StartInfo.ArgumentList.Add(path);
			p.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
			p.StartInfo.RedirectStandardOutput = true;
			p.StartInfo.RedirectStandardInput = true;
			p.EnableRaisingEvents = true;
			//p.Exited += (send, evt) => {
			//	Dispatcher.BeginInvoke((Action)(() => {
			//		MessageBox.Show("Done");
			//		txtOurs.Focus();
			//		txtOurs.Text = File.ReadAllText(output);

			//	}));
			//};

			p.StartInfo.CreateNoWindow = true;

			p.Start();
			var outTask = p.StandardOutput.ReadToEndAsync();
			var inTask = WriteInput(p.StandardInput, text);
			await Task.WhenAll(outTask, inTask);
			txtOurs.Focus();
			txtOurs.Text = await outTask;
		}

		private async Task WriteInput(StreamWriter standardInput, string text) {
			await standardInput.WriteLineAsync(text);
			standardInput.Close();
		}

		private void Window_DragEnter(object sender, DragEventArgs e) {
			DragDropEffects effects = DragDropEffects.None;
			if (e.Data.GetDataPresent(DataFormats.FileDrop)) {
				var path = ((string[])e.Data.GetData(DataFormats.FileDrop))[0];
				if (File.Exists(path))
					effects = DragDropEffects.Copy;
			}

			e.Effects = effects;
		}
	}
}
