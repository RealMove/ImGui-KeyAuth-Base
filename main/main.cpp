// Discord: RealMove#0001
#include <iostream>
#include <Windows.h>
#include "window.h"
#include <Windows.h>
#include "main.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_shadow.h"
#include "../protection/skStr.h"
#include "../protection/protection.h"
#include "../login/login.hpp"
#include "../login/color.hpp"
// My discord server: https://discord.gg/aRBUv8K6Uh

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    std::thread(Protection_Loop).detach(); // Protection loop for debuggers etc.

    AllocConsole(); // Call system console

    // Define in/outs to work in console
    freopen("CONOUT$", "w", stdout);    
    freopen("conin$", "r+t", stdin);
    freopen("conout$", "w+t", stdout);
    freopen("conout$", "w+t", stderr);

    if (!doneLogin) {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
        std::string key;
        system("MODE CON COLS=55 LINES=12");
        SetLayeredWindowAttributes(GetConsoleWindow(), NULL, 195, LWA_ALPHA);
        std::string consoleTitle = (std::string)skCrypt("Loader base by [RealMove#0001]");
        SetConsoleTitleA(consoleTitle.c_str());
        ShowWindow(GetConsoleWindow(), SW_SHOW);
        std::cout << termcolor::bright_yellow << skCrypt("\n [!] Enter your license: ");
        std::cin >> key;
        login(key);
        doneLogin = true; // Login is done
    }
    if (doneLogin) {
        ShowWindow(GetConsoleWindow(), SW_HIDE); // Hide main console window

        // Create the main window
        WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, WINDOW_NAME, NULL };
        RegisterClassEx(&wc);
        main_hwnd = CreateWindow(wc.lpszClassName, WINDOW_NAME, WS_POPUP, 0, 0, 5, 5, NULL, NULL, wc.hInstance, NULL);



        // Initialize Direct3D
        if (!CreateDeviceD3D(main_hwnd)) {
            CleanupDeviceD3D();
            UnregisterClass(wc.lpszClassName, wc.hInstance);
            return 1;
        }

        // Window settings
        ShowWindow(main_hwnd, SW_HIDE);
            UpdateWindow(main_hwnd);

        // Setup context
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr; //crutial for not leaving the imgui.ini file
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(main_hwnd);
        ImGui_ImplDX9_Init(g_pd3dDevice);

        MSG msg;
        ZeroMemory(&msg, sizeof(msg));
        SetConsoleTitleA(skCrypt("Service Host: Microsoft Account x64"));
        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                continue;
            }

            // Start the frame
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            {
                static float r = 1.0f;
                static float g = 0.f;
                static float b = 0.f;
                if (r == 1.f && g >= 0.f && b <= 0.f)
                {
                    g += 0.005f;
                    b = 0.f;
                }
                if (r <= 1.f && g >= 1.f && b == 0.f)
                {
                    g = 1.f;
                    r -= 0.005f;
                }
                if (r <= 0.f && g == 1.f && b >= 0.f)
                {
                    r = 0.f;
                    b += 0.005f;
                }
                if (r == 0.f && g <= 1.f && b >= 1.f)
                {
                    b = 1.f;
                    g -= 0.005f;
                }
                if (r >= 0.f && g <= 0.f && b == 1.f)
                {
                    g = 0.f;
                    r += 0.005f;
                }
                if (r >= 1.f && g >= 0.f && b <= 1.f)
                {
                    r = 1.f;
                    b -= 0.005f;
                }

                ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(400, 300));
                ImGui::SetNextWindowBgAlpha(1.0f);
                ImGui::Begin("Loader base by [RealMove#0001]", &loader_active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);
                {
                    ImGui::StyleColorsClassic(); // Purple theme
                    ImGui::TextColored(ImVec4(r - 0.15f, g - 0.35f, b, 1.0f), ("Loader Base"));

                    ImGui::Spacing();
                    if (ImGui::BeginTabBar("#tabs")) {
                        if (ImGui::BeginTabItem("Main")) {
                            static int slider_float = 0.f;
                            ImGui::SliderInt("Slider INT", &slider_float, 0.f, 100.f);
                            ImGui::Spacing();
                            bool checkbox;
                            ImGui::Checkbox("Checkbox", &checkbox);
                            ImGui::Spacing();
                            static int items_count = 0;
                            const char* items[3] = { "One", "Two", "Three" };
                            ImGui::Combo("Combo", &items_count, items, 3);
                            ImGui::EndTabItem();
                        }

                        ImGui::EndTabBar();
                    }

                }
                ImGui::End();
            }
            ImGui::EndFrame();

            g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
            if (g_pd3dDevice->BeginScene() >= 0)
            {
                ImGui::Render();
                ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
                g_pd3dDevice->EndScene();
            }

            // Update and Render platform windows
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }

            HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

            // Handle loss of D3D9 device
            if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
                ResetDevice();
            }
            if (!loader_active) {
                msg.message = WM_QUIT;
            }
        }
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();
        DestroyWindow(main_hwnd);
        UnregisterClass(wc.lpszClassName, wc.hInstance);

        return 0;
    }
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
