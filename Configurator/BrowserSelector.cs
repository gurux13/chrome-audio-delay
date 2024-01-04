using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Configurator
{
    public partial class BrowserSelector : Form
    {
        SystemHelper systemHelper = new SystemHelper();
        Dictionary<string, string> knownBrowsers = new Dictionary<string, string>()
        {
            {"Chrome", "chrome.exe" },
            {"Edge", "msedge.exe" },
            {"Yandex.Browser", @"%LOCALAPPDATA%\Yandex\YandexBrowser\Application" },
        };
        public BrowserSelector()
        {
            InitializeComponent();
        }

        

        private void btnAddManually_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "Executables|*.exe";
            if (ofd.ShowDialog() == DialogResult.OK)
            {
                systemHelper.AddExecutable(ofd.FileName);
            }
            RecalculateEnabled();
        }

        void RecalculateEnabled()
        {
            dgvEnabled.Rows.Clear();
            var enabledBrowsers = systemHelper.GetExecutables();
            foreach (var browser in enabledBrowsers)
            {
                dgvEnabled.Rows.Add(browser);
            }
        }

        void RecalculateFound()
        {
            dgvFound.Rows.Clear();
            var allEnabled = new HashSet<string>(systemHelper.GetExecutables());
            foreach (var browser in knownBrowsers)
            {
                var path = systemHelper.ResolveFileName(browser.Value);
                if (path != null && !allEnabled.Contains(path))
                {
                    dgvFound.Rows.Add(new string[] { browser.Key, path });
                }
            }
        }

        private void BrowserSelector_Load(object sender, EventArgs e)
        {
            RecalculateEnabled();
            RecalculateFound();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            foreach (var entry in dgvEnabled.SelectedCells)
            {
                systemHelper.DeleteExecutable((entry as DataGridViewTextBoxCell).Value as string);
            }
            RecalculateEnabled();
            RecalculateFound();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dgvFound.SelectedRows)
            {
                systemHelper.AddExecutable(row.Cells[1].Value as string);
            }
            RecalculateEnabled();
            RecalculateFound();
        }

        private void button4_Click(object sender, EventArgs e)
        {
            Close();
        }
    }
}
