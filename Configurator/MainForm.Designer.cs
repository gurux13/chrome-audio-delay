
namespace Configurator
{
    partial class MainForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.cbStartWithWindows = new System.Windows.Forms.CheckBox();
            this.pnlDevices = new System.Windows.Forms.Panel();
            this.btnRefresh = new System.Windows.Forms.Button();
            this.btnStartStop = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.lblIsRunning = new System.Windows.Forms.Label();
            this.pnlAllEndpoints = new System.Windows.Forms.Panel();
            this.dgvAllItems = new System.Windows.Forms.DataGridView();
            this.colName = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.colType = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.btnMoveEndpoint = new System.Windows.Forms.Button();
            this.lblLeftLabel = new System.Windows.Forms.Label();
            this.lblRightLabel = new System.Windows.Forms.Label();
            this.btnExecutables = new System.Windows.Forms.Button();
            this.tmrBlinkSelect = new System.Windows.Forms.Timer(this.components);
            this.pnlAllEndpoints.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dgvAllItems)).BeginInit();
            this.SuspendLayout();
            // 
            // cbStartWithWindows
            // 
            this.cbStartWithWindows.AutoSize = true;
            this.cbStartWithWindows.Location = new System.Drawing.Point(13, 13);
            this.cbStartWithWindows.Name = "cbStartWithWindows";
            this.cbStartWithWindows.Size = new System.Drawing.Size(213, 24);
            this.cbStartWithWindows.TabIndex = 0;
            this.cbStartWithWindows.Text = "Start injector with Windows";
            this.cbStartWithWindows.UseVisualStyleBackColor = true;
            this.cbStartWithWindows.CheckedChanged += new System.EventHandler(this.cbStartWithWindows_CheckedChanged);
            // 
            // pnlDevices
            // 
            this.pnlDevices.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.pnlDevices.AutoScroll = true;
            this.pnlDevices.BackColor = System.Drawing.SystemColors.ControlLightLight;
            this.pnlDevices.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.pnlDevices.Location = new System.Drawing.Point(441, 97);
            this.pnlDevices.Name = "pnlDevices";
            this.pnlDevices.Size = new System.Drawing.Size(375, 324);
            this.pnlDevices.TabIndex = 1;
            // 
            // btnRefresh
            // 
            this.btnRefresh.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnRefresh.Image = ((System.Drawing.Image)(resources.GetObject("btnRefresh.Image")));
            this.btnRefresh.Location = new System.Drawing.Point(769, 1);
            this.btnRefresh.Name = "btnRefresh";
            this.btnRefresh.Size = new System.Drawing.Size(47, 47);
            this.btnRefresh.TabIndex = 0;
            this.btnRefresh.UseVisualStyleBackColor = true;
            this.btnRefresh.Click += new System.EventHandler(this.btnRefresh_Click);
            // 
            // btnStartStop
            // 
            this.btnStartStop.Location = new System.Drawing.Point(249, 13);
            this.btnStartStop.Name = "btnStartStop";
            this.btnStartStop.Size = new System.Drawing.Size(114, 47);
            this.btnStartStop.TabIndex = 2;
            this.btnStartStop.Text = "Start now";
            this.btnStartStop.UseVisualStyleBackColor = true;
            this.btnStartStop.Click += new System.EventHandler(this.btnStartStop_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 40);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(76, 20);
            this.label1.TabIndex = 3;
            this.label1.Text = "Injector is:";
            // 
            // lblIsRunning
            // 
            this.lblIsRunning.AutoSize = true;
            this.lblIsRunning.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point);
            this.lblIsRunning.Location = new System.Drawing.Point(94, 40);
            this.lblIsRunning.Name = "lblIsRunning";
            this.lblIsRunning.Size = new System.Drawing.Size(99, 20);
            this.lblIsRunning.TabIndex = 4;
            this.lblIsRunning.Text = "Not Running";
            // 
            // pnlAllEndpoints
            // 
            this.pnlAllEndpoints.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.pnlAllEndpoints.AutoScroll = true;
            this.pnlAllEndpoints.BackColor = System.Drawing.SystemColors.ControlLightLight;
            this.pnlAllEndpoints.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.pnlAllEndpoints.Controls.Add(this.dgvAllItems);
            this.pnlAllEndpoints.Location = new System.Drawing.Point(13, 97);
            this.pnlAllEndpoints.Name = "pnlAllEndpoints";
            this.pnlAllEndpoints.Size = new System.Drawing.Size(375, 324);
            this.pnlAllEndpoints.TabIndex = 2;
            // 
            // dgvAllItems
            // 
            this.dgvAllItems.AllowUserToAddRows = false;
            this.dgvAllItems.AllowUserToDeleteRows = false;
            this.dgvAllItems.AllowUserToResizeColumns = false;
            this.dgvAllItems.AllowUserToResizeRows = false;
            this.dgvAllItems.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
            this.dgvAllItems.BackgroundColor = System.Drawing.SystemColors.ControlLightLight;
            this.dgvAllItems.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.dgvAllItems.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgvAllItems.ColumnHeadersVisible = false;
            this.dgvAllItems.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.colName,
            this.colType});
            this.dgvAllItems.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dgvAllItems.EditMode = System.Windows.Forms.DataGridViewEditMode.EditProgrammatically;
            this.dgvAllItems.Location = new System.Drawing.Point(0, 0);
            this.dgvAllItems.MultiSelect = false;
            this.dgvAllItems.Name = "dgvAllItems";
            this.dgvAllItems.ReadOnly = true;
            this.dgvAllItems.RowHeadersVisible = false;
            this.dgvAllItems.RowHeadersWidth = 51;
            this.dgvAllItems.RowTemplate.Height = 29;
            this.dgvAllItems.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dgvAllItems.Size = new System.Drawing.Size(371, 320);
            this.dgvAllItems.TabIndex = 0;
            // 
            // colName
            // 
            this.colName.HeaderText = "Name";
            this.colName.MinimumWidth = 6;
            this.colName.Name = "colName";
            this.colName.ReadOnly = true;
            // 
            // colType
            // 
            this.colType.FillWeight = 50F;
            this.colType.HeaderText = "Type";
            this.colType.MinimumWidth = 6;
            this.colType.Name = "colType";
            this.colType.ReadOnly = true;
            // 
            // btnMoveEndpoint
            // 
            this.btnMoveEndpoint.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.btnMoveEndpoint.Location = new System.Drawing.Point(394, 237);
            this.btnMoveEndpoint.Name = "btnMoveEndpoint";
            this.btnMoveEndpoint.Size = new System.Drawing.Size(41, 41);
            this.btnMoveEndpoint.TabIndex = 5;
            this.btnMoveEndpoint.Text = "->";
            this.btnMoveEndpoint.UseVisualStyleBackColor = true;
            this.btnMoveEndpoint.Click += new System.EventHandler(this.btnMoveEndpoint_Click);
            // 
            // lblLeftLabel
            // 
            this.lblLeftLabel.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.lblLeftLabel.AutoSize = true;
            this.lblLeftLabel.Location = new System.Drawing.Point(151, 74);
            this.lblLeftLabel.Name = "lblLeftLabel";
            this.lblLeftLabel.Size = new System.Drawing.Size(111, 20);
            this.lblLeftLabel.TabIndex = 6;
            this.lblLeftLabel.Text = "System Devices";
            // 
            // lblRightLabel
            // 
            this.lblRightLabel.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.lblRightLabel.AutoSize = true;
            this.lblRightLabel.Location = new System.Drawing.Point(580, 74);
            this.lblRightLabel.Name = "lblRightLabel";
            this.lblRightLabel.Size = new System.Drawing.Size(138, 20);
            this.lblRightLabel.TabIndex = 7;
            this.lblRightLabel.Text = "Overridden Devices";
            // 
            // btnExecutables
            // 
            this.btnExecutables.Location = new System.Drawing.Point(509, 13);
            this.btnExecutables.Name = "btnExecutables";
            this.btnExecutables.Size = new System.Drawing.Size(189, 47);
            this.btnExecutables.TabIndex = 8;
            this.btnExecutables.Text = "Choose Browsers";
            this.btnExecutables.UseVisualStyleBackColor = true;
            this.btnExecutables.Click += new System.EventHandler(this.btnExecutables_Click);
            // 
            // tmrBlinkSelect
            // 
            this.tmrBlinkSelect.Tick += new System.EventHandler(this.tmrBlinkSelect_Tick);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(828, 433);
            this.Controls.Add(this.btnExecutables);
            this.Controls.Add(this.lblRightLabel);
            this.Controls.Add(this.lblLeftLabel);
            this.Controls.Add(this.btnMoveEndpoint);
            this.Controls.Add(this.pnlAllEndpoints);
            this.Controls.Add(this.lblIsRunning);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.btnStartStop);
            this.Controls.Add(this.pnlDevices);
            this.Controls.Add(this.btnRefresh);
            this.Controls.Add(this.cbStartWithWindows);
            this.Name = "MainForm";
            this.Text = "Chrome Audio Delay";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.Resize += new System.EventHandler(this.MainForm_Resize);
            this.pnlAllEndpoints.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dgvAllItems)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.CheckBox cbStartWithWindows;
        private System.Windows.Forms.Panel pnlDevices;
        private System.Windows.Forms.Button btnRefresh;
        private System.Windows.Forms.Button btnStartStop;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label lblIsRunning;
        private System.Windows.Forms.Panel pnlAllEndpoints;
        private System.Windows.Forms.Button btnMoveEndpoint;
        private System.Windows.Forms.DataGridView dgvAllItems;
        private System.Windows.Forms.DataGridViewTextBoxColumn colName;
        private System.Windows.Forms.DataGridViewTextBoxColumn colType;
        private System.Windows.Forms.Label lblLeftLabel;
        private System.Windows.Forms.Label lblRightLabel;
        private System.Windows.Forms.Button btnExecutables;
        private System.Windows.Forms.Timer tmrBlinkSelect;
    }
}

