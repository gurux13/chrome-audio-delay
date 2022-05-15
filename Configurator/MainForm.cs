using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Configurator
{
    public partial class MainForm : Form
    {
        SystemHelper registry = new SystemHelper();
        public MainForm()
        {
            InitializeComponent();
        }

        void SetDelay(SystemHelper.AudioEndpoint audioEndpoint, int delay)
        {
            registry.UpdateAudioEndpointDelay(new SystemHelper.AudioEndpointDelay { Id = audioEndpoint.Id, Delay = (UInt32)delay });
        }

        Process GetInjectorProcess()
        {
            return Process.GetProcesses().FirstOrDefault(x => x.ProcessName == "ManualInject");
        }

        bool hadRunningLastRefresh = false;

        private void PopulateDgv(List<SystemHelper.AudioEndpoint> endpoints, List<SystemHelper.AudioEndpointDelay> selectedEndpoints)
        {
            dgvAllItems.Rows.Clear();
            HashSet<string> selectedEndpointsSet = selectedEndpoints.Select(x => x.Id).ToHashSet();
            foreach (var endpoint in endpoints)
            {
                if (selectedEndpointsSet.Contains(endpoint.Id))
                {
                    continue;
                }
                dgvAllItems.Rows.Add(new string[] { endpoint.Name, endpoint.Type });
                var row = dgvAllItems.Rows[dgvAllItems.Rows.Count - 1];
                if (endpoint.State == CoreAudio.DEVICE_STATE.DEVICE_STATE_ACTIVE)
                {
                    foreach (DataGridViewCell cell in row.Cells)
                    {
                        cell.Style.BackColor = Color.LightGreen;
                        cell.Style.SelectionBackColor = Color.Green;
                    }
                }

            }
        }


        private void btnRefresh_Click(object sender, EventArgs e)
        {
            bool isRunning = GetInjectorProcess() != null;
            hadRunningLastRefresh = isRunning;
            lblIsRunning.Text = isRunning ? "Running" : "Not Running";
            lblIsRunning.ForeColor = isRunning ? Color.Green : Color.Red;
            btnStartStop.Text = isRunning ? "Stop now" : "Start now";
            cbStartWithWindows.Checked = registry.IsAutoStart();
            var endpoints = registry.GetCurrentEndpoints();
            var delays = registry.GetAudioEndpointDelays();
            PopulateDgv(endpoints, delays);

            pnlDevices.Controls.Clear();
            int curY = 0;
            foreach (var endpoint in endpoints)
            {
                var label = new Label();
                label.Text = (endpoint.Name ?? "") + " (" + endpoint.Type + ")";
                if (endpoint.State == CoreAudio.DEVICE_STATE.DEVICE_STATE_ACTIVE)
                {
                    label.Font = new Font(label.Font, FontStyle.Bold);
                }
                label.Top = curY;
                label.Width = pnlDevices.Width;
                label.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
                pnlDevices.Controls.Add(label);
                curY = label.Bottom + 5;
                var lblValue = new Label();
                lblValue.Top = curY;
                lblValue.TextAlign = ContentAlignment.MiddleRight;
                lblValue.Text = "0ms";
                lblValue.Width = 60;
                lblValue.Left = pnlDevices.Width - 5 - lblValue.Width;
                lblValue.Anchor = AnchorStyles.Right | AnchorStyles.Top;
                pnlDevices.Controls.Add(lblValue);
                var trackBar = new TrackBar();
                trackBar.Minimum = 0;
                trackBar.Maximum = 500;
                var delay = delays.FirstOrDefault(x => x.Id == endpoint.Id);
                trackBar.ValueChanged += (sender, e) =>
                {
                    lblValue.Text = trackBar.Value + "ms";
                    SetDelay(endpoint, trackBar.Value);
                };
                if (delay != null)
                {
                    trackBar.Value = Math.Max(0, Math.Min(500, (int)delay.Delay));

                }
                trackBar.Width = pnlDevices.Width - lblValue.Width;
                trackBar.Top = curY;
                trackBar.TickFrequency = 100;
                trackBar.TickStyle = TickStyle.Both;
                trackBar.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;

                pnlDevices.Controls.Add(trackBar);
                curY = trackBar.Bottom + 5;
            }
            var newHeight = Math.Min(600, curY);
            this.Height = newHeight + pnlDevices.Top + (this.Bottom - pnlDevices.Bottom);
        }

        private void cbStartWithWindows_CheckedChanged(object sender, EventArgs e)
        {
            registry.SetAutoStart(cbStartWithWindows.Checked);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            btnRefresh_Click(sender, e);
        }

        private async void btnStartStop_Click(object sender, EventArgs e)
        {
            var injectorProcess = GetInjectorProcess();
            if (hadRunningLastRefresh != (injectorProcess != null))
            {
                btnRefresh_Click(sender, e);
                return;
            }
            if (injectorProcess != null)
            {
                injectorProcess.Kill();
                await injectorProcess.WaitForExitAsync();
            }
            else
            {
                Process.Start(".\\ManualInject.exe");
            }
            btnRefresh_Click(sender, e);
        }

        private void MainForm_Resize(object sender, EventArgs e)
        {
            const int padding = 10;
            pnlAllEndpoints.Width = pnlDevices.Width = (this.ClientSize.Width - btnMoveEndpoint.Width - 4 * padding) / 2;
            pnlAllEndpoints.Left = padding;
            pnlDevices.Left = pnlAllEndpoints.Right + 2 * padding + btnMoveEndpoint.Width;
            btnMoveEndpoint.Left = (pnlAllEndpoints.Right + pnlDevices.Left - btnMoveEndpoint.Width) / 2;
            btnMoveEndpoint.Top = (pnlAllEndpoints.Top + pnlAllEndpoints.Bottom - btnMoveEndpoint.Height) / 2;
            lblLeftLabel.Left = pnlAllEndpoints.Left + (pnlAllEndpoints.Width - lblLeftLabel.Width) / 2;
            lblRightLabel.Left = pnlDevices.Left + (pnlDevices.Width - lblRightLabel.Width) / 2;

        }
    }
}
