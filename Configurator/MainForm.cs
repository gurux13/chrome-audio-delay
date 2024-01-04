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
        SystemHelper systemHelper = new SystemHelper();
        public MainForm()
        {
            InitializeComponent();
        }

        void SetDelay(SystemHelper.AudioEndpointDelay delay)
        {
            systemHelper.UpdateEndpointDelay(delay);
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
                row.Tag = endpoint;
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

        private void PopulateOverrides(List<SystemHelper.AudioEndpoint> endpoints, List<SystemHelper.AudioEndpointDelay> selectedEndpoints)
        {
            const int padding = 10;
            Dictionary<string, SystemHelper.AudioEndpoint> endpointsDict = endpoints.ToDictionary(x => x.Id);
            pnlDevices.SuspendLayout();
            pnlDevices.Controls.Clear();
            int curY = 0;
            foreach (var endpoint in selectedEndpoints)
            {
                endpointsDict.TryGetValue(endpoint.Id, out var sysEndpoint);
                var button = new Button();
                button.Text = "X";
                button.Left = 0;
                button.Top = curY;
                button.Width = button.Height = 50;
                button.Click += (s, e) =>
                {
                    systemHelper.RemoveEndpointDelay(endpoint);
                    btnRefresh_Click(s, e);
                };
                pnlDevices.Controls.Add(button);
                var label = new Label();
                label.Text = (endpoint.Name ?? "") + " (" + endpoint.Type + ")";
                if (sysEndpoint?.State == CoreAudio.DEVICE_STATE.DEVICE_STATE_ACTIVE)
                {
                    label.Font = new Font(label.Font, FontStyle.Bold);
                }
                label.Left = button.Right + padding;
                label.Top = curY;
                label.Width = pnlDevices.Width;
                label.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
                pnlDevices.Controls.Add(label);
                curY = label.Bottom + padding;
                var lblValue = new Label();
                lblValue.Top = curY;
                lblValue.TextAlign = ContentAlignment.MiddleRight;
                lblValue.Text = "0ms";
                lblValue.Width = 60;
                lblValue.Left = pnlDevices.Width - padding - lblValue.Width -(pnlDevices.VerticalScroll.Visible ? 60 : 0);
                lblValue.Anchor = AnchorStyles.Right | AnchorStyles.Top;
                pnlDevices.Controls.Add(lblValue);
                var trackBar = new TrackBar();
                trackBar.Left = label.Left;
                trackBar.Minimum = 0;
                trackBar.Maximum = 500;
                trackBar.ValueChanged += (sender, e) =>
                {
                    lblValue.Text = trackBar.Value + "ms";
                    endpoint.Delay = (uint)trackBar.Value;
                    SetDelay(endpoint);
                };
                trackBar.Value = Math.Max(0, Math.Min(500, (int)endpoint.Delay));
                trackBar.Width = lblValue.Left - trackBar.Left - padding;
                trackBar.Top = curY;
                trackBar.TickFrequency = 100;
                trackBar.TickStyle = TickStyle.Both;
                trackBar.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;

                button.Top = (trackBar.Bottom + label.Top - button.Height) / 2;

                pnlDevices.Controls.Add(trackBar);
                curY = trackBar.Bottom + 5;
            }
            pnlDevices.ResumeLayout();
        }

        private void btnRefresh_Click(object sender, EventArgs e)
        {
            this.SuspendLayout();
            bool isRunning = GetInjectorProcess() != null;
            hadRunningLastRefresh = isRunning;
            lblIsRunning.Text = isRunning ? "Running" : "Not Running";
            lblIsRunning.ForeColor = isRunning ? Color.Green : Color.Red;
            btnStartStop.Text = isRunning ? "Stop now" : "Start now";
            cbStartWithWindows.Checked = systemHelper.IsAutoStart();
            var endpoints = systemHelper.GetCurrentEndpoints();
            var delays = systemHelper.GetAudioEndpointDelays();
            PopulateDgv(endpoints, delays);
            PopulateOverrides(endpoints, delays);
            this.ResumeLayout(true);
        }

        private void cbStartWithWindows_CheckedChanged(object sender, EventArgs e)
        {
            systemHelper.SetAutoStart(cbStartWithWindows.Checked);
        }

        void RecalculateBrowserCount()
        {
            var count = systemHelper.GetExecutables().Count;
            btnExecutables.Text = $"Select Browsers ({count})";
            if (btnOriginalFont == null)
            {
                btnOriginalFont = btnExecutables.Font;
                btnBoldFont = new Font(btnOriginalFont, FontStyle.Bold);
            }
            btnExecutables.Font = count == 0 ? btnBoldFont : btnOriginalFont;
        }
        private void Form1_Load(object sender, EventArgs e)
        {
            systemHelper.Initialize();
            btnRefresh_Click(sender, e);
            RecalculateBrowserCount();
        }

        void BlinkSelectBrowsers()
        {
            tmrRunsLeft = 5;
            tmrBlinkSelect.Enabled = true;
        }

        private async void btnStartStop_Click(object sender, EventArgs e)
        {
            
            var injectorProcess = GetInjectorProcess();
            if (injectorProcess == null)
            {
                if (systemHelper.GetExecutables().Count == 0)
                {
                    BlinkSelectBrowsers();
                    return;
                }
            }
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

        private void btnMoveEndpoint_Click(object sender, EventArgs e)
        {
            if (dgvAllItems.SelectedRows.Count != 1)
            {
                return;
            }
            var endpoint = dgvAllItems.SelectedRows[0].Tag as SystemHelper.AudioEndpoint;
            var delay = new SystemHelper.AudioEndpointDelay
            {
                Delay = 0,
                Id = endpoint.Id,
                Name = endpoint.Name,
                Type = endpoint.Type,
            };
            systemHelper.UpdateEndpointDelay(delay);
            btnRefresh_Click(sender, e);
        }

        private void btnExecutables_Click(object sender, EventArgs e)
        {
            (new BrowserSelector()).ShowDialog();
            RecalculateBrowserCount();
        }
        Color? originalBtnColor = null;
        Font? btnBoldFont = null;
        Font? btnOriginalFont = null;
        int tmrRunsLeft = 0;
        private void tmrBlinkSelect_Tick(object sender, EventArgs e)
        {
            
            if (originalBtnColor == null)
            {
                originalBtnColor = btnExecutables.ForeColor;
            }
            Color originalBtnColorNotNull = (Color)originalBtnColor;
            if (tmrRunsLeft <= 0)
            {
                tmrBlinkSelect.Enabled = false;
                btnExecutables.ForeColor = originalBtnColorNotNull;
                return;
            }
            btnExecutables.ForeColor = btnExecutables.ForeColor == originalBtnColorNotNull ? Color.Red : originalBtnColorNotNull;
            --tmrRunsLeft;
        }
    }
}
