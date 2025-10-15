#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

bool TerminateProcessByPID(DWORD dwProcessId, UINT uExitCode = 0)
{
    bool bResult = false;
    
    // 打开进程
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
    
    if (hProcess != NULL)
    {
        // 尝试终止父进程
        if (TerminateProcess(hProcess, uExitCode))
        {
            std::cout << "Terminated process with PID: " << dwProcessId << std::endl;
            bResult = true;
        }
        else
        {
            std::cout << "Faild to terminate process. Erroe code: " << GetLastError() << std::endl;
        }
        
        CloseHandle(hProcess);
    }
    else
    {
        std::cout << "Cannot open process with PID: " << dwProcessId 
                  << ",Error code: " << GetLastError() << std::endl;
    }
    
    return bResult;
}

//强制终止所有子进程
bool ForceTerminateProcessTree(DWORD dwProcessId, UINT uExitCode = 0)
{
    bool bResult = true;
    
    // 创建进程快照
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        std::cout << "Faild to create process snapshot. Error code: " << GetLastError() << std::endl;
        return false;
    }
    
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    
    // 枚举所有进程
    if (Process32First(hSnapshot, &pe))
    {
        do
        {
            // 如果这个进程的父进程ID是我们目标进程ID
            if (pe.th32ParentProcessID == dwProcessId)
            {
                // 递归终止子进程
                ForceTerminateProcessTree(pe.th32ProcessID, uExitCode);
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    
    CloseHandle(hSnapshot);
    
    // 最后终止目标进程本身
    bResult = TerminateProcessByPID(dwProcessId, uExitCode);
    
    return bResult;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: taskfuck <PID>" << std::endl;
        return 1;
    }
    
    DWORD pid = atoi(argv[1]);
    
    if (pid == 0)
    {
        std::cout << "Invalid PID" << std::endl;
        return 1;
    }

    if (!ForceTerminateProcessTree(pid))
    {
        std::cout << "Cannot terminate process with PID: " << pid << std::endl;
        return 1;
    }
 
    return 0;
}