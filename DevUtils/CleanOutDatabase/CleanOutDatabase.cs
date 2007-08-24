using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Data.OleDb;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace CleanOutDatabase
{
	public partial class CleanOutDatabase : Form
	{
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// 
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public CleanOutDatabase()
		{
			InitializeComponent();
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// 
		/// </summary>
		/// <param name="e"></param>
		/// ------------------------------------------------------------------------------------
		protected override void OnDragOver(DragEventArgs e)
		{
			e.Effect = DragDropEffects.Copy;
			base.OnDragOver(e);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// 
		/// </summary>
		/// <param name="e"></param>
		/// ------------------------------------------------------------------------------------
		protected override void OnDragDrop(DragEventArgs e)
		{
			base.OnDragDrop(e);

			string[] files = e.Data.GetData("FileDrop") as string[];
			if (files == null || files.Length == 0)
				return;

			string connectionString =
				@"Provider=Microsoft.Jet.OLEDB.4.0; Data Source=" + files[0];

			using (OleDbConnection connection = new OleDbConnection(connectionString))
			{
				connection.Open();
				Delete(connection, "Category");
				Delete(connection, "Document");
				Delete(connection, "PhonemicList");
				Delete(connection, "OrthoList");
				Delete(connection, "ToneList");
				Delete(connection, "GlossList");
				Delete(connection, "POSList");
				Delete(connection, "PhoneticList");
				Delete(connection, "CharList");
				Delete(connection, "Speaker");
				connection.Close();
				label1.Text = "Done";
			}
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// 
		/// </summary>
		/// <param name="connection"></param>
		/// <param name="table"></param>
		/// ------------------------------------------------------------------------------------
		private void Delete(OleDbConnection connection, string table)
		{
			string sql = "DELETE * FROM " + table;
			OleDbCommand cmd = new OleDbCommand(sql, connection);
			cmd.ExecuteNonQuery();
		}
	}
}