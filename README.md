# Remote Assistance Volatile Environment LPE

| Exploit Information |                                   |
|:------------------- |:--------------------------------- |
| Publish Date        | 28.06.2017                        |
| Fixed               | Windows 10 RS3 (16299)            |
| Target              | Microsoft Windows                 |
| exploit-db          | N/A                               |
| CVE                 | N/A                               |
| Versions            | Windows 7-10, x86 and x64         |

## Description

In msra.exe (Windows Remote Assistance), there is a DLL loading vulnerability
that can be exploited by injecting an environment variable.

A load attempt to `%SYSTEMROOT%\System32\mswsock.dll` will be performed from
this auto-elevated process.

Redirecting `%SYSTEMROOT%` can be achieved through Volatile Environment. For
this, we set `HKEY_CURRENT_USER\Volatile Environment\SYSTEMROOT` to a new
directory, which we then populate with our hijacked payload DLL, along with
*.clb files from `C:\Windows\Registration` as they are loaded from our new
directory as well.

Then, as we execute msra.exe, it will load our payload DLL and also the COM+
components. We need to copy those, too, because the process will otherwise
crash. Note, that we need to specify the runas verb, as the executable's
requestedExecutionLevel is asInvoker.

Our DLL is now executed with high IL. In this example, Payload.exe will be
started, which is an exemplary payload file displaying a MessageBox.

## Expected Result

When everything worked correctly, Payload.exe should be executed, displaying basic information including integrity level.

![](http://bytecode77.localhost/images/sites/hacking/exploits/uac-bypass/remote-assistance-privilege-escalation/result.png)

## Downloads

Compiled binaries with example payload:

[![](https://bytecode77.com/images/shared/fileicons/zip.png) RemoteAssistanceVolatileEnvironmentLPE rev1 Binaries.zip](https://bytecode77.com/downloads/hacking/exploits/uac-bypass/RemoteAssistanceVolatileEnvironmentLPE%20rev1%20Binaries.zip)