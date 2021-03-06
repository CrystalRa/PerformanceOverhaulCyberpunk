#include "Overlay.h"

#include <atlcomcli.h>
#include <d3d12.h>
#include <Image.h>
#include <imgui.h>
#include <memory>
#include <Options.h>
#include <Pattern.h>
#include <kiero/kiero.h>
#include <spdlog/spdlog.h>

#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "reverse/Engine.h"
#include "reverse/Scripting.h"

static std::shared_ptr<Overlay> s_pOverlay;

void Overlay::Initialize(Image* apImage)
{
    if (!s_pOverlay)
    {
        s_pOverlay.reset(new (std::nothrow) Overlay);
        s_pOverlay->EarlyHooks(apImage);
    }
}

void Overlay::Shutdown()
{
    s_pOverlay = nullptr;
}

Overlay& Overlay::Get()
{
    return *s_pOverlay;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void Overlay::DrawImgui(IDXGISwapChain3* apSwapChain)
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(600.f, ImGui::GetFrameHeight() * 15.f + ImGui::GetFrameHeightWithSpacing()), ImGuiCond_FirstUseEver);

    ImGui::Begin("Cyber Engine Tweaks");

    if (Options::Get().GameImage.version == Image::MakeVersion(1, 4) ||
        Options::Get().GameImage.version == Image::MakeVersion(1, 5))
    {
        ImGui::Checkbox("Clear Input", &m_inputClear);
        ImGui::SameLine();
        if (ImGui::Button("Clear Output"))
        {
            std::lock_guard<std::recursive_mutex> _{ m_outputLock };
            m_outputLines.clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Scroll Output", &m_outputShouldScroll);

        static char command[512] = { 0 };

        {
            std::lock_guard<std::recursive_mutex> _{ m_outputLock };

            ImVec2 listboxSize = ImGui::GetContentRegionAvail();
            listboxSize.y -= ImGui::GetFrameHeightWithSpacing();
            const auto result = ImGui::ListBoxHeader("", listboxSize);
            for (auto& item : m_outputLines)
                if (ImGui::Selectable(item.c_str()))
                    std::strncpy(command, item.c_str(), sizeof(command) - 1);
            if (m_outputScroll)
            {
                if (m_outputShouldScroll)
                    ImGui::SetScrollHereY();
                m_outputScroll = false;
            }
            if (result)
                ImGui::ListBoxFooter();
        }

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        const auto execute = ImGui::InputText("", command, std::size(command), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SetItemDefaultFocus();
        if (execute)
        {
            std::string returnMessage;

            {
                std::lock_guard<std::recursive_mutex> _{ m_outputLock };
                m_outputLines.push_back(std::string(command));
                m_outputScroll = true;
            }

            if (!Scripting::Execute(command, returnMessage))
            {
                std::lock_guard<std::recursive_mutex> _{ m_outputLock };
                m_outputLines.push_back(std::string("Error: ") + returnMessage);
                m_outputScroll = true;
            }

            if (m_inputClear)
                std::memset(command, 0, sizeof(command));

            ImGui::SetKeyboardFocusHere();
        }
    }

    ImGui::End();

    ImGui::Render();
}

LRESULT APIENTRY Overlay::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_KEYDOWN && wParam == Options::Get().ConsoleKey)
    {
        s_pOverlay->Toggle();
        return 0;
    }

    switch (uMsg)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (wParam == Options::Get().ConsoleKey)
            return 0;
    case WM_CHAR:
        if (Options::Get().ConsoleChar && wParam == Options::Get().ConsoleChar)
            return 0;
    }

    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return 1;

    if (s_pOverlay->IsEnabled())
    {
        // ignore mouse & keyboard events
        if ((uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) ||
            (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST))
            return 0;

        // ignore specific messages
        switch (uMsg)
        {
        case WM_INPUT:
            return 0;
        }
    }

    return CallWindowProc(s_pOverlay->m_wndProc, hwnd, uMsg, wParam, lParam);
}

using TScriptCall = void*(uint8_t*, uint8_t**, REDString*, void*);

TScriptCall** GetScriptCallArray()
{
    static uint8_t* pLocation = FindSignature({ 0x4C, 0x8D, 0x15, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x89, 0x42, 0x38, 0x49, 0x8B, 0xF8, 0x48, 0x8B, 0x02, 0x4C, 0x8D, 0x44, 0x24, 0x20, 0xC7 }) + 3;
    static uintptr_t finalLocation = (uintptr_t)pLocation + 4 + *reinterpret_cast<uint32_t*>(pLocation);

    return reinterpret_cast<TScriptCall**>(finalLocation);
}


void* Overlay::Log(uintptr_t apThis, uint8_t** apStack)
{
    REDString result("");
    apStack[6] = nullptr;
    apStack[7] = nullptr;
    auto stack = *(*apStack)++;
    GetScriptCallArray()[stack](apStack[8], apStack, &result, nullptr);
    ++(*apStack);

    std::lock_guard<std::recursive_mutex> _{ Get().m_outputLock };
    Get().m_outputLines.emplace_back(result.ToString());
    Get().m_outputScroll = true;

    result.Destroy();

    return 0;
}

void Overlay::Toggle()
{
    struct Singleton
    {
        uint8_t pad0[0xC0];
        SomeStruct* pSomeStruct;
    };

    m_enabled = !m_enabled;

    while(true)
    {
        if (m_enabled && ShowCursor(TRUE) >= 0)
            break;
        if (!m_enabled && ShowCursor(FALSE) < 0)
            break;
    }

    ClipToCenter(CGameEngine::Get()->pSomeStruct);
}

bool Overlay::IsEnabled()
{
    return m_enabled;
}

Overlay::Overlay() = default;

Overlay::~Overlay() = default;

