// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <iostream>
#include <vector>
#include<bitset>
#include <string> 
#include <iomanip>
#include <sstream>
#include <fstream>
#include <thread>
#include <chrono>
#include <future>
#include <algorithm>
#include "gui.h"
#include <thread>
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

HMODULE myhModule;
std::string str;
std::string str1;
std::vector<std::uint8_t> currentByte;
std::vector<std::byte> originalBytes;
std::vector<std::string> frameActions;
std::vector<std::string> frameTimes;
std::ifstream inData1;
std::ifstream inData2;

float preempt = 0;
static float sliderFloat = 0.f;


DWORD __stdcall EjectThread(LPVOID lpParameter) {
    Sleep(100);
    FreeLibraryAndExitThread(myhModule, 0);
}

DWORD GetAddressFromSignature(std::vector<int> signature, DWORD startaddress = 0, DWORD endaddress = 0) {
    
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    if (startaddress == 0) {
        startaddress = (DWORD)(si.lpMinimumApplicationAddress);
    }
    if (endaddress == 0) {
        endaddress = (DWORD)(si.lpMaximumApplicationAddress);
    }

    MEMORY_BASIC_INFORMATION mbi{ 0 };
    DWORD protectflags = (PAGE_GUARD | PAGE_NOCACHE | PAGE_NOACCESS);

    for (DWORD i = startaddress; i < endaddress - signature.size(); i++) {
        //std::cout << "scanning: " << std::hex << i << std::endl;
        if (VirtualQuery((LPCVOID)i, &mbi, sizeof(mbi))) {
            if (mbi.Protect & protectflags || !(mbi.State & MEM_COMMIT)) {
               // std::cout << "Bad Region! Region Base Address: " << mbi.BaseAddress << " | Region end address: " << std::hex << (int)((DWORD)mbi.BaseAddress + mbi.RegionSize) << std::endl;
                i += mbi.RegionSize;
                continue; // if bad adress then dont read from it
            }
            // std::cout << "Good Region! Region Base Address: " << mbi.BaseAddress << " | Region end address: " << std::hex << (int)((DWORD)mbi.BaseAddress + mbi.RegionSize) << std::endl;
            for (DWORD k = (DWORD)mbi.BaseAddress; k < (DWORD)mbi.BaseAddress + mbi.RegionSize - signature.size(); k++) {
                for (DWORD j = 0; j < signature.size(); j++) {
                    if (signature.at(j) != -1 && signature.at(j) != *(byte*)(k + j))
                        break;
                    if (j + 1 == signature.size())
                        return k;
                }
            }
            i = (DWORD)mbi.BaseAddress + mbi.RegionSize;
        }
    }
    return NULL;
}

void doLeftRim()
{
    keybd_event(0x5A, 0, KEYEVENTF_EXTENDEDKEY, 0);
   // std::cout << "doing left rim" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    keybd_event(0x5A, 0, KEYEVENTF_KEYUP, 0);
}
void doLeftInner()
{
    keybd_event(0x58, 0, KEYEVENTF_EXTENDEDKEY, 0);
   // std::cout << "doing left inner" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    keybd_event(0x58, 0, KEYEVENTF_KEYUP, 0);
}
void doRightInner()
{
    keybd_event(0x43, 0, KEYEVENTF_EXTENDEDKEY, 0);
// std::cout << "doing right inner" << std::endl;
std::this_thread::sleep_for(std::chrono::milliseconds(40));
keybd_event(0x43, 0, KEYEVENTF_KEYUP, 0);
}
void doRightRim()
{
    keybd_event(0x56, 0, KEYEVENTF_EXTENDEDKEY, 0);
    // std::cout << "doing right rim" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    keybd_event(0x56, 0, KEYEVENTF_KEYUP, 0);
}

void Patch(BYTE* dst, BYTE* src, unsigned int size)
{
    DWORD oldprotect;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
    memcpy(dst, src, size);
    VirtualProtect(dst, size, oldprotect, &oldprotect);
}

void PatchProper(BYTE* dst, BYTE src, unsigned int size)
{
    DWORD oldprotect;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
    *(BYTE*)dst = src;
    VirtualProtect(dst, size, oldprotect, &oldprotect);
}

std::vector<std::string>Split(std::string str, int splitLength)
{
    int NumSubstrings = str.length() / splitLength;
    std::vector<std::string> ret;

    for (auto i = 0; i < NumSubstrings; i++)
    {
        ret.push_back(str.substr(i * splitLength, splitLength));
    }
    return ret;
}

void renderMenu()
{
    gui::CreateHWindow("shiina");
    gui::CreateDevice();
    gui::CreateImGui();

    while (gui::exit)
    {
        gui::BeginRender();
        gui::Render();
        gui::EndRender();

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    gui::DestroyImGui();
    gui::DestroyDevice();
    gui::DestroyHWindow();
}
void patchARChangerBytes(DWORD base, int size)
{
    for (int i = 0; i < size; i++)
    {
        
        PatchProper(reinterpret_cast<std::uint8_t*>((int)base + i), currentByte[i], 1);
    }
}
void createByteArray()
{

}
void gui::Render() noexcept
{
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
    ImGui::Begin(
        "shiina private",
        &exit,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove
    );
    static bool checkBox = false;
    ImGui::SliderFloat("Approach Rate", &sliderFloat, 0.f, 10.f);
    if (ImGui::Button("Apply"))
    {
        preempt = 1200 - 750 * (sliderFloat - 5) / 5;
    }
    ImGui::End();
}

DWORD WINAPI Menu() {
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    while (1)
    {
        if (GetAsyncKeyState(VK_NUMPAD2))
        {
            std::cout << "injecting..." << std::endl;
            std::vector<int> emptySig = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
            DWORD emptySigEntry = GetAddressFromSignature(emptySig, 0x10000000, 0x50000000);
            if (emptySigEntry == NULL)
                emptySigEntry = GetAddressFromSignature(emptySig, 0x1F000000, 0x4A000000);
            if (emptySigEntry == NULL)
                emptySigEntry = GetAddressFromSignature(emptySig);

            //  *(float*)emptySigEntry = 450;


            std::vector<int> hookSig = { 0x8B, 0x40, 0x2C, 0xFF, 0x50, 0x08, 0xDD, 0x5D, 0xE0, 0xF2, 0x0F, 0x10, 0x45, 0xE0, 0xF2, 0x0F, 0x2C, 0xC0, 0x89, 0x46, 0x1C, };

            DWORD Entry1 = GetAddressFromSignature(hookSig, 0x01000000, 0x20000000);
            if (Entry1 == NULL)
                Entry1 = GetAddressFromSignature(hookSig, 0x1F000000, 0x4A000000);
            if (Entry1 == NULL)
                Entry1 = GetAddressFromSignature(hookSig);
        
            char addy[14];
            sprintf_s(addy, "\xD9\x05%x", emptySigEntry);
            std::cout << emptySigEntry << std::endl;
      




            Patch(reinterpret_cast<std::uint8_t*>((int)Entry1), (std::uint8_t*)addy, 6);
            // const std::uintptr_t addye = *reinterpret_cast<std::uintptr_t*>((int)Entry1 + 2);
             // *(float*)addye = 450;
        }
        
            std::cout << "injecting..." << std::endl;
            std::stringstream ss;
            std::vector<int> emptySig = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
            DWORD emptySigEntry = GetAddressFromSignature(emptySig, 0x11111111, 0x90000000);
            if (emptySigEntry == NULL)
                emptySigEntry = GetAddressFromSignature(emptySig, 0x1F000000, 0x4A000000);
            if (emptySigEntry == NULL)
                emptySigEntry = GetAddressFromSignature(emptySig);

            //  *(float*)emptySigEntry = 450;
          

            std::vector<int> hookSig = { 0x8B, 0x40, 0x2C, 0xFF, 0x50, 0x08, 0xDD, 0x5D, 0xE0, 0xF2, 0x0F, 0x10, 0x45, 0xE0, 0xF2, 0x0F, 0x2C, 0xC0, 0x89, 0x46, 0x1C, };

            DWORD Entry1 = GetAddressFromSignature(hookSig, 0x01000000, 0x20000000);
            if (Entry1 == NULL)
                Entry1 = GetAddressFromSignature(hookSig, 0x1F000000, 0x4A000000);
            if (Entry1 == NULL)
                Entry1 = GetAddressFromSignature(hookSig);
           
            /*
            char addy[14];
            sprintf_s(addy, "\xD9\x05%x", emptySigEntry);
            std::cout << emptySigEntry << std::endl;
            std::cout << addy << " wow" << std::endl;
            */
            ss << std::hex << emptySigEntry;
            std::string address;
            address = ss.str();
            std::string temp;
            
            temp = address[1];
            temp = temp + address[0];
            int i_hex = std::stoi(temp, nullptr, 16);
            temp = address[3];
            temp = temp + address[2];
            int i_hex2 = std::stoi(temp, nullptr, 16);
            temp = address[5];
            temp = temp + address[4];
            int i_hex3 = std::stoi(temp, nullptr, 16);
            temp = address[7];
            temp = temp + address[6];
            int i_hex4 = std::stoi(temp, nullptr, 16);


            byte notlol = 0xD9;
            byte verynotlol = 0x05;
            byte lol = (byte)i_hex;
            byte lol1 = (byte)i_hex2;
            byte lol2 = (byte)i_hex3;
            byte lol3 = (byte)i_hex4;
            PatchProper(reinterpret_cast<std::uint8_t*>((int)Entry1), notlol, 1);
            PatchProper(reinterpret_cast<std::uint8_t*>((int)Entry1 + 1), verynotlol, 1);
            PatchProper(reinterpret_cast<std::uint8_t*>((int)Entry1 + 2), lol3, 1);
            PatchProper(reinterpret_cast<std::uint8_t*>((int)Entry1 + 3), lol2, 1);
            PatchProper(reinterpret_cast<std::uint8_t*>((int)Entry1 + 4), lol1, 1);
            PatchProper(reinterpret_cast<std::uint8_t*>((int)Entry1 + 5), lol, 1);
            const std::uintptr_t addye = *reinterpret_cast<std::uintptr_t*>((int)Entry1 + 2);
          
            
           
              
            std::thread(renderMenu).detach();
            fclose(fp);
            FreeConsole();

            preempt = 450;
            while (2)
            {
                *(float*)addye = preempt;
                preempt = 1200 - 750 * (sliderFloat - 5) / 5;
            }
           // Patch(reinterpret_cast<std::uint8_t*>((int)Entry1), (std::uint8_t*)i_hex, 1);
            //Patch(reinterpret_cast<std::uint8_t*>((int)Entry1 + 3), (std::uint8_t*)i_hex2, 1);
            //Patch(reinterpret_cast<std::uint8_t*>((int)Entry1 + 4), (std::uint8_t*)i_hex3, 1);
            //Patch(reinterpret_cast<std::uint8_t*>((int)Entry1 + 5), (std::uint8_t*)i_hex4, 1);
            
            

            /*
            Patch(reinterpret_cast<std::uint8_t*>((int)Entry1), (std::uint8_t*)addy, 2);
            for (int i = 0; i < 4; i++)
            {
                int e = i + 2;
                
                Patch(reinterpret_cast<std::uint8_t*>((int)Entry1 + e), (std::uint8_t*)addy, 1);
            }
            */
        

    }
    fclose(fp);
    FreeConsole();
    CreateThread(0, 0, EjectThread, 0, 0, 0);
    int i = 0;

    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        myhModule = hModule;
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Menu, NULL, 0, NULL);
        
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

