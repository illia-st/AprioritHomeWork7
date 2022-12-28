#include <iostream>
#include <functional>

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <memory>
#include "CreateFileGuard.h"
#include "MySmartPointers.h"


const char* DEFAULT_PORT = "12345";

void FunctionDeleterForSocket(SOCKET * fd){
    if(*fd == INVALID_SOCKET) return;
    closesocket(*fd);
    delete fd;
    std::cout << "Socket is deleted in FunctionDeleterForSocket" << std::endl;
}
std::function<void(SOCKET * fd)> FunctorDeleter = [](SOCKET * fd){
    if(*fd == INVALID_SOCKET) return;
    closesocket(*fd);
    delete fd;
    std::cout << "Socket is deleted in FunctorDeleter" << std::endl;
};
struct Socket_deleter{
    void operator() (SOCKET* const fd){
        if(*fd == INVALID_SOCKET) return;
        closesocket(*fd);
        delete fd;
        std::cout << "Socket is deleted in struct Socket_deleter" << std::endl;
    }
};
void DeletersExample() {
    // Socket deleters example
    WSADATA wsaData;
    int iResult;

    struct addrinfo *result = nullptr;
    struct addrinfo hints;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return;
    }
    std::unique_ptr<SOCKET, void(*)(SOCKET*)> Socket1(
            std::make_unique<SOCKET>(socket(result->ai_family, result->ai_socktype, result->ai_protocol)).get(),
            FunctionDeleterForSocket );
    std::unique_ptr<SOCKET, std::function<void(SOCKET * fd)>> Socket2(
            std::make_unique<SOCKET>(socket(result->ai_family, result->ai_socktype, result->ai_protocol)).get(),
            FunctorDeleter );
    std::unique_ptr<SOCKET, Socket_deleter> Socket3(
            std::make_unique<SOCKET>(socket(result->ai_family, result->ai_socktype, result->ai_protocol)).get(),
            Socket_deleter{ });

    WSACleanup();
}

void GuardExample(){
    try{
        CreateFileGuard handler(
                "C:\\NewFile.txt",
                GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_NEW,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );
        std::string text_to_write1 {"Advice of the day: Don't shot in your leg. Use guards.\n"};
        std::string text_to_write2 {"Have a nice day :)\n"};
        DWORD bytesWritten;
        WriteFile(
                handler.GetHandle(),
                text_to_write1.c_str(),
                text_to_write1.length(),
                &bytesWritten,
                nullptr
        );
        bytesWritten = 0;
        WriteFile(
                handler,
                text_to_write2.c_str(),
                text_to_write2.length(),
                &bytesWritten,
                nullptr
                );

    } catch (const std::runtime_error& ex) {
        std::cerr << ex.what() << std::endl;
    }catch(const std::exception& ex){
        std::cerr << ex.what() << std::endl;
    }
}

void SmartPointersExample(){
    {
        MySharedPtr<int> ptr1(new int(15));
        MySharedPtr<int> ptr2(ptr1);
        std::cout << *ptr2 << std::endl;
        *ptr1 = 10;
        std::cout << *ptr2 << std::endl;
        MySharedPtr<int> ptr3(std::move(ptr2));
        if(ptr2.get() == nullptr){
            std::cout << "ptr2 is now moved" << std::endl;
            std::cout << "ptr2 Use_count() > " << ptr2.use_count() << std::endl;
            std::cout << "ptr3 Use_count() > " << ptr3.use_count() << std::endl;
        }
        MyWeakPtr<int> weak1(ptr3);
        std::cout << "weak1 Use_count() > " << weak1.use_count() << std::endl;
        MySharedPtr<int> ptr4(weak1.lock());
        std::cout << "ptr4 Use_count() > " << ptr4.use_count() << std::endl;
    }
    std::cout << "==========================================================" << std::endl;
    {
        MySharedPtr<int> ptr1(new int(14));
        {
            MyWeakPtr<int> weak1(ptr1);
            std::cout << "weak1 Use_count() > " << weak1.use_count() << std::endl;
        }
        std::cout << "ptr1 Use_count() > " << ptr1.use_count() << std::endl;
    }
    std::cout << "==========================================================" << std::endl;
    {
        MyWeakPtr<int> weak1;
        {
            MySharedPtr<int> ptr1(new int(10));
            weak1 = ptr1;
            std::cout << "weak1 Use_count() > " << weak1.use_count() << std::endl;
        }
        std::cout << "weak1 Use_count() > " << weak1.use_count() << std::endl;
    }
}

int main(){
    std::cout << "Guard example" << std::endl;
    GuardExample();
    std::cout << "|||||||||||||||||||||||||||||||||||||||||||||||||||||" << std::endl;
    std::cout << "Socket deleters example" << std::endl;
    DeletersExample();
    std::cout << "|||||||||||||||||||||||||||||||||||||||||||||||||||||" << std::endl;
    std::cout << "Smart pointers example" << std::endl;
    SmartPointersExample();

    return 0;
}
