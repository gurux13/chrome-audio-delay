using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Win32.TaskScheduler;
using CoreAudio;

namespace Configurator
{
    class SystemHelper
    {
        const string gx13Key = @"SOFTWARE\gurux13\ChromePatcher\devices";
        const string immKey = @"SOFTWARE\Microsoft\Windows\CurrentVersion\MMDevices\Audio\Render";
        const string startupKey = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Run";
        const string startupValueName = "gx13ChromePatcher";
        public class AudioEndpoint
        {
            public string Id { get; set; }
            public string Type { get; set; }
            public string Name { get; set; }
            public DEVICE_STATE State { get; internal set; }
        }
        public class AudioEndpointDelay
        {
            public string Id { get; set; }
            public UInt32 Delay { get; set; }
            public string Type { get; set; }
            public string Name { get; set; }
        }
        public List<AudioEndpoint> GetAudioEndpoints()
        {
            List<AudioEndpoint> rv = new List<AudioEndpoint>();
            var devices = Registry.LocalMachine.OpenSubKey(immKey);
            foreach (var skId in devices.GetSubKeyNames())
            {
                var skey = devices.OpenSubKey(skId + "\\Properties");
                var id = skey.GetValue("{9c119480-ddc2-4954-a150-5bd240d454ad},2");
                var type = skey.GetValue("{a45c254e-df1c-4efd-8020-67d146a850e0},2");
                var name = skey.GetValue("{b3f8fa53-0004-438e-9003-51a46e139bfc},6");
                if (id != null && id is string && type != null && type is string)
                {
                    var entry = new AudioEndpoint()
                    {
                        Id = id.ToString().Replace(@"SWD\MMDEVAPI\", ""),
                        Name = name?.ToString(),
                        Type = type.ToString()
                    };
                    rv.Add(entry);
                }
            }
            return rv;
        }
        public List<AudioEndpointDelay> GetAudioEndpointDelays()
        {
            List<AudioEndpointDelay> rv = new List<AudioEndpointDelay>();
            var devices = Registry.LocalMachine.OpenSubKey(gx13Key);
            foreach (var device in devices.GetValueNames())
            {
                var delay = devices.GetValue(device);
                rv.Add(new AudioEndpointDelay { Delay = (UInt32)(Int32) delay, Id = device });
            }
            return rv;
        }
        Guid devTypeProperty = Guid.Parse("{a45c254e-df1c-4efd-8020-67d146a850e0}");
        string GetEndpointType(MMDevice dev)
        {
            for (int i = 0; i < dev.Properties.Count; i++)
            {
                if (dev.Properties[i].Key.fmtid == devTypeProperty)
                {
                    return dev.Properties[i].Value as string;
                }
            }
            return null;
        }
        public List<AudioEndpoint> GetCurrentEndpoints()
        {
            MMDeviceEnumerator DevEnum = new MMDeviceEnumerator();
            var endpoints = DevEnum.EnumerateAudioEndPoints(EDataFlow.eRender, DEVICE_STATE.DEVICE_STATEMASK_ALL);
            var rv = endpoints.Where(x => x.State != DEVICE_STATE.DEVICE_STATE_NOTPRESENT).Select(x => new AudioEndpoint { Id = x.ID, Name = x.FriendlyName, Type = GetEndpointType(x), State = x.State });
            
            foreach (var endpoint in endpoints)
            {
                string s = "";
                for (int i = 0; i < endpoint.Properties.Count; i++)
                {
                    if (endpoint.Properties[i].Value is string)
                    {
                        s += i.ToString() + " -> " + endpoint.Properties[i].Value as string + "\n";
                    }
                }
                endpoint.Dispose();
            }
            return rv.ToList();
        }
        public void UpdateAudioEndpointDelay(AudioEndpointDelay delay)
        {
            var devices = Registry.LocalMachine.OpenSubKey(gx13Key, true);
            devices.SetValue(delay.Id, delay.Delay, RegistryValueKind.DWord);
        }
        string GetCurFolder()
        {
            return Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
        }
        string GetLoaderPath()
        {
            return Path.Combine(GetCurFolder(), "ManualInject.exe");
        }
        public bool IsAutoStart()
        {
            using (TaskService ts = new TaskService())
            {
                return ts.GetTask(startupValueName) != null;
            }
        }
        public void SetAutoStart(bool enabled)
        {
            using (TaskService ts = new TaskService())
            {
                ts.RootFolder.DeleteTask(startupValueName, false);
                if (enabled)
                {
                    TaskDefinition td = ts.NewTask();
                    td.RegistrationInfo.Description = "Runs the Chrome patcher for audio delay";

                    // Create a trigger that will fire the task at this time every other day
                    td.Triggers.Add(new LogonTrigger());

                    // Create an action that will launch Notepad whenever the trigger fires
                    td.Actions.Add(new ExecAction(GetLoaderPath(), null, GetCurFolder()));

                    td.Principal.RunLevel = TaskRunLevel.Highest;

                    td.Settings.AllowDemandStart = false;
                    td.Settings.DisallowStartIfOnBatteries = false;
                    td.Settings.StopIfGoingOnBatteries = false;
                    td.Settings.IdleSettings.StopOnIdleEnd = false;
                    td.Settings.ExecutionTimeLimit = TimeSpan.FromDays(365);
                    // Register the task in the root folder
                    ts.RootFolder.RegisterTaskDefinition(startupValueName, td);
                }
            }
        }
    }
}
