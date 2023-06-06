using ICSharpCode.AvalonEdit;
using ICSharpCode.AvalonEdit.Folding;
using ICSharpCode.AvalonEdit.Highlighting;
using ICSharpCode.AvalonEdit.Highlighting.Xshd;
using ICSharpCode.AvalonEdit.Indentation.CSharp;
using ICSharpCode.AvalonEdit.Sample;

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

using System.Xml;

namespace WPFMKProto {
	public class OurTextEditor : TextEditor, INotifyPropertyChanged {
		public static readonly DependencyProperty TextProperty = DependencyProperty.Register("Text", typeof(string), typeof(OurTextEditor),
		 new FrameworkPropertyMetadata(default(string), FrameworkPropertyMetadataOptions.None, (obj, args) => {
			 var target = (OurTextEditor)obj;
			 if (target.we_are_changing)
				 return;
			 target.Text = (string)args.NewValue;
			 target.RaisePropertyChanged(nameof(Text));
		 })
		);

		public new string Text {
			get { return base.Text; }
			set { base.Text = value; }
		}
		private bool we_are_changing;
		protected override void OnTextChanged(EventArgs e) {
			we_are_changing = true;
			RaisePropertyChanged(nameof(Text));
			if (IsInitialized) {
				var binding = BindingOperations.GetBinding(this, TextProperty);
				if (Document != null && (binding?.Mode == BindingMode.TwoWay || binding?.Mode == BindingMode.OneWayToSource)) {
					SetValue(TextProperty, Text);
				}

			}
			base.OnTextChanged(e);
			we_are_changing = false;
		}

		public event PropertyChangedEventHandler PropertyChanged;
		public void RaisePropertyChanged(string info) {
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(info));
		}
		public OurTextEditor() {
			indentStrat = new CSharpIndentationStrategy(Options);
			TextArea.IndentationStrategy = indentStrat;
			using var xshd_stream = File.OpenRead(Path.Combine(new FileInfo(System.Reflection.Assembly.GetExecutingAssembly().Location).DirectoryName, "SyntaxCCustom.xshd"));
			using var xshd_reader = new XmlTextReader(xshd_stream);
			SyntaxHighlighting = HighlightingLoader.Load(xshd_reader, HighlightingManager.Instance);

			foldingManager = FoldingManager.Install(TextArea);
			foldingStrategy = new BraceFoldingStrategy();
			foldingStrategy.UpdateFoldings(foldingManager, Document);
			var btn = new Button { Content = "Indent", HorizontalAlignment = HorizontalAlignment.Right, VerticalAlignment = VerticalAlignment.Top };
			indent();

			//foldingStrategy.UpdateFoldings(foldingManager, textEditor.Document);// 
			PreviewKeyUp += (_, _) => DelayRefreshFold();
		}
		object latestDelay;
		private async void DelayRefreshFold() {
			var cur = latestDelay = new();
			await Task.Delay(TimeSpan.FromSeconds(2));
			if (cur != latestDelay)
				return;
			RefreshFold();
		}
		private void indent() {
			indentStrat.IndentLines(TextArea.Document, 0, TextArea.Document.LineCount - 1);
		}
		private CSharpIndentationStrategy indentStrat;
		private FoldingManager foldingManager;
		private BraceFoldingStrategy foldingStrategy;
		private object RefreshFold() {
			foldingStrategy.UpdateFoldings(foldingManager, Document);
			return null;
		}
	}
}
