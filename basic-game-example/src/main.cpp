//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <filesystem>

#include "log_utils.hpp"

#include "core/ee_core.hpp"
#include "export/ee_export_data.hpp"
#include "core/kw_core.hpp"

#include "core/core.hpp"
#include "graphics/render.hpp"

using KalaHeaders::KalaCore::ContainsValue;

using KalaHeaders::KalaLog::Log;
using KalaHeaders::KalaLog::LogType;

using ElypsoEngine::Core::AppConfig;
using ElypsoEngine::Export::SUPPORTED_EXTENSIONS;
using ElypsoEngine::Export::UserBlock;
using ElypsoEngine::Export::ExportData;

using KalaWindow::Core::KalaWindowCore;

using GraphicsTemplate::Core::GraphicsTemplateCore;
using GraphicsTemplate::Graphics::Render;

using std::vector;
using std::filesystem::path;
using std::filesystem::recursive_directory_iterator;

extern const AppConfig ElypsoEngine::Core::appConfig = 
{
    .title = "basic game example",
    .pos = 600
};

void ElypsoEngine::Core::Init()
{
    //print all user system info for now always at the very top

    Log::Print(KalaWindowCore::GetCPUInfoString());
    Log::Print(" ");
    Log::Print(KalaWindowCore::GetGPUInfoString());
    Log::Print(" ");
    Log::Print(KalaWindowCore::GetRAMInfoString(true));
    Log::Print(" ");
    Log::Print(KalaWindowCore::GetOSInfoString());
    Log::Print(" ");

    GraphicsTemplateCore::Initialize();
    Render::Initialize();

    /*
    vector<UserBlock> assetBlocks{};

    for (const auto& p : recursive_directory_iterator(KalaWindowCore::GetExePath().parent_path()))
    {
        path thisPath = path(p);

        if (!is_directory(thisPath)
            && ContainsValue(SUPPORTED_EXTENSIONS, thisPath.extension()))
        {
            assetBlocks.push_back({ thisPath.stem(), thisPath });
            Log::Print("@@@@@ path: " + thisPath.string());
        }
    }

    ExportData::ExportAssetData(
        KalaWindowCore::GetExePath().parent_path() / "out.edat",
        assetBlocks);
    */
}

void ElypsoEngine::Core::EarlyUpdate()
{

}

void ElypsoEngine::Core::FixedUpdate()
{

}

void ElypsoEngine::Core::Update()
{
    Render::Update();
}

void ElypsoEngine::Core::LateUpdate()
{
    
}