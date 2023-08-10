#include <iostream>
#include <vector>
// WINDOWS API
#include <Windows.h>
#include <TlHelp32.h>

#ifdef UNICODE
#define mycout std::wcout
#elif
#define mycout std::cout
#endif // UNICODE

class SmartHandle
{
public:
   SmartHandle(HANDLE handle)
   {
      _handle = handle;
   }

   ~SmartHandle()
   {
      if (_handle)
         CloseHandle(_handle);
   }
   // приведение к типу bool
   operator bool()
   {
      return _handle != NULL;
   }
   // приведение к типу HANDLE
   operator HANDLE()
   {
      return _handle;
   }
   // getter для HANDLE
   HANDLE handle()
   {
      return _handle;
   }

private:
   HANDLE _handle = NULL;
};

struct ProcessInfo
{
   PROCESSENTRY32 pe; // процессы
   std::vector<THREADENTRY32> threads; // потоки
};


int main(int argc, char* argv[])
{
   SmartHandle processSnap(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
   SmartHandle threadSnap(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0));
   if (!processSnap || !threadSnap)
      return 1;

   std::vector<ProcessInfo> processInfos;

   // для потоков
   std::vector<THREADENTRY32> threads;

   THREADENTRY32 te{ sizeof(THREADENTRY32) };
   if (Thread32First(threadSnap, &te) == FALSE)
      return 2;

   do
   {
      threads.push_back(te);
   } while (Thread32Next(threadSnap, &te));

   // для процессов
   PROCESSENTRY32 pe{ sizeof(PROCESSENTRY32) };
   if (Process32First(processSnap, &pe) == FALSE)
      return 2;

   do
   {
      std::vector<THREADENTRY32> subThreads;
      for (const auto& thread : threads)
      {
         if (thread.th32OwnerProcessID == pe.th32ProcessID)
            subThreads.push_back(thread);
      }
      processInfos.push_back(ProcessInfo{ pe, subThreads });
   } while (Process32Next(processSnap, &pe));

   // вывод процессов и потоков в консоль
   for (const auto& processInfo : processInfos)
   {
      mycout << processInfo.pe.szExeFile << std::endl;
      for (const auto& thread : processInfo.threads)
      {
         mycout << "    " << thread.th32ThreadID << std::endl;
      }
   }

}
