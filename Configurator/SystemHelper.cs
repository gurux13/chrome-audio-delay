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
        public List<AudioEndpointDelay> GetAudioEndpointDelays()
        {
            List<AudioEndpointDelay> rv = new List<AudioEndpointDelay>();
            using (var devices = Registry.LocalMachine.OpenSubKey(gx13Key))
            {
                foreach (var deviceId in devices.GetSubKeyNames())
                {
                    using (var key = devices.OpenSubKey(deviceId))
                    {
                        var delay = (Int32)key.GetValue("Delay");
                        var name = (string)key.GetValue("Name");
                        var type = (string)key.GetValue("Type");
                        rv.Add(new AudioEndpointDelay { Delay = (UInt32)(Int32)delay, Id = deviceId, Name = name, Type = type });
                    }
                }
            }
            return rv;
        }
        public void UpdateEndpointDelay(AudioEndpointDelay delay)
        {
            using (var key = Registry.LocalMachine.CreateSubKey(gx13Key + "\\" + delay.Id))
            {
                key.SetValue("Delay", (Int32)delay.Delay);
                key.SetValue("Name", delay.Name);
                key.SetValue("Type", delay.Type);
            }
        }
        public void RemoveEndpointDelay(AudioEndpointDelay delay)
        {
            Registry.LocalMachine.DeleteSubKey(gx13Key + "\\" + delay.Id);
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
                endpoint.Dispose();
            }
            return rv.ToList();
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
