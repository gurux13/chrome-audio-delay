namespace Configurator
{
    partial class BrowserSelector
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.dgvEnabled = new System.Windows.Forms.DataGridView();
            this.dgvEName = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dgvFound = new System.Windows.Forms.DataGridView();
            this.dgvFName = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dgvFPath = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.button3 = new System.Windows.Forms.Button();
            this.button4 = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.dgvEnabled)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dgvFound)).BeginInit();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(113, 20);
            this.label1.TabIndex = 0;
            this.label1.Text = "Found browsers";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 241);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(126, 20);
            this.label2.TabIndex = 2;
            this.label2.Text = "Enabled browsers";
            // 
            // dgvEnabled
            // 
            this.dgvEnabled.AllowUserToAddRows = false;
            this.dgvEnabled.AllowUserToDeleteRows = false;
            this.dgvEnabled.AllowUserToResizeColumns = false;
            this.dgvEnabled.AllowUserToResizeRows = false;
            this.dgvEnabled.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dgvEnabled.BackgroundColor = System.Drawing.SystemColors.Control;
            this.dgvEnabled.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgvEnabled.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.dgvEName});
            this.dgvEnabled.EditMode = System.Windows.Forms.DataGridViewEditMode.EditProgrammatically;
            this.dgvEnabled.Location = new System.Drawing.Point(12, 269);
            this.dgvEnabled.Name = "dgvEnabled";
            this.dgvEnabled.RowHeadersVisible = false;
            this.dgvEnabled.RowHeadersWidth = 51;
            this.dgvEnabled.RowTemplate.Height = 29;
            this.dgvEnabled.Size = new System.Drawing.Size(849, 200);
            this.dgvEnabled.TabIndex = 3;
            // 
            // dgvEName
            // 
            this.dgvEName.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.dgvEName.HeaderText = "Path";
            this.dgvEName.MinimumWidth = 6;
            this.dgvEName.Name = "dgvEName";
            this.dgvEName.ReadOnly = true;
            this.dgvEName.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            // 
            // dgvFound
            // 
            this.dgvFound.AllowUserToAddRows = false;
            this.dgvFound.AllowUserToDeleteRows = false;
            this.dgvFound.AllowUserToResizeColumns = false;
            this.dgvFound.AllowUserToResizeRows = false;
            this.dgvFound.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dgvFound.BackgroundColor = System.Drawing.SystemColors.Control;
            this.dgvFound.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgvFound.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.dgvFName,
            this.dgvFPath});
            this.dgvFound.EditMode = System.Windows.Forms.DataGridViewEditMode.EditProgrammatically;
            this.dgvFound.Location = new System.Drawing.Point(12, 32);
            this.dgvFound.Name = "dgvFound";
            this.dgvFound.RowHeadersVisible = false;
            this.dgvFound.RowHeadersWidth = 51;
            this.dgvFound.RowTemplate.Height = 29;
            this.dgvFound.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dgvFound.Size = new System.Drawing.Size(849, 200);
            this.dgvFound.TabIndex = 4;
            // 
            // dgvFName
            // 
            this.dgvFName.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
            this.dgvFName.HeaderText = "Name";
            this.dgvFName.MinimumWidth = 6;
            this.dgvFName.Name = "dgvFName";
            this.dgvFName.Width = 78;
            // 
            // dgvFPath
            // 
            this.dgvFPath.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.dgvFPath.HeaderText = "Path";
            this.dgvFPath.MinimumWidth = 6;
            this.dgvFPath.Name = "dgvFPath";
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(144, 237);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(29, 29);
            this.button1.TabIndex = 5;
            this.button1.Text = "↓";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(179, 237);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(29, 29);
            this.button2.TabIndex = 6;
            this.button2.Text = "X";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // button3
            // 
            this.button3.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.button3.Location = new System.Drawing.Point(742, 237);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(119, 29);
            this.button3.TabIndex = 7;
            this.button3.Text = "Add manually";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.btnAddManually_Click);
            // 
            // button4
            // 
            this.button4.Location = new System.Drawing.Point(12, 475);
            this.button4.Name = "button4";
            this.button4.Size = new System.Drawing.Size(94, 29);
            this.button4.TabIndex = 8;
            this.button4.Text = "Done";
            this.button4.UseVisualStyleBackColor = true;
            this.button4.Click += new System.EventHandler(this.button4_Click);
            // 
            // BrowserSelector
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(873, 511);
            this.Controls.Add(this.button4);
            this.Controls.Add(this.button3);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.dgvFound);
            this.Controls.Add(this.dgvEnabled);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Name = "BrowserSelector";
            this.Text = "Select browsers";
            this.Load += new System.EventHandler(this.BrowserSelector_Load);
            ((System.ComponentModel.ISupportInitialize)(this.dgvEnabled)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dgvFound)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.DataGridView dgvEnabled;
        private System.Windows.Forms.DataGridView dgvFound;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.DataGridViewTextBoxColumn dgvFName;
        private System.Windows.Forms.DataGridViewTextBoxColumn dgvFPath;
        private System.Windows.Forms.DataGridViewTextBoxColumn dgvEName;
        private System.Windows.Forms.Button button4;
    }
}