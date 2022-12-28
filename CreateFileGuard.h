#pragma once

#include <Windows.h>

class CreateFileGuard{
public:
    explicit CreateFileGuard(
            LPCSTR lpFileName,
            // pointer to name of the file
            DWORD dwDesiredAccess,
            // access (read-write) mode
            DWORD dwShareMode,
            // share mode
            LPSECURITY_ATTRIBUTES lpSecurityAttributes,
            // pointer to security attributes
            DWORD dwCreationDistribution,
            // how to create
            DWORD dwFlagsAndAttributes,
            // file attributes
            HANDLE hTemplateFile): hFile(CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
                                                    dwCreationDistribution, dwFlagsAndAttributes, hTemplateFile))
    {
        if(hFile == INVALID_HANDLE_VALUE){
            throw std::runtime_error("Failed to open/create file");
        }
    }
    // as HANDLE is void*
    // we don't need operator HANDLE& ()
    operator HANDLE () const{
        return hFile;
    }
    // also you can use GetHandle ;)
    HANDLE GetHandle() const{
        return hFile;
    }
    ~CreateFileGuard(){
        CloseHandle(hFile);
    }
private:
    HANDLE hFile;
};