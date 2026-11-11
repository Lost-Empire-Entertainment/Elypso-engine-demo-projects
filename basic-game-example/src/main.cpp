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

using KalaHeaders::KalaLog::Log;
using KalaHeaders::KalaLog::LogType;

using ElypsoEngine::Core::AppConfig;
using ElypsoEngine::Export::ExportData;

using KalaWindow::Core::KalaWindowCore;

using GraphicsTemplate::Core::GraphicsTemplateCore;
using GraphicsTemplate::Graphics::Render;

using std::filesystem::path;

extern const AppConfig ElypsoEngine::Core::appConfig = 
{
    .title = "basic game example",
    .pos = 600
};

void ElypsoEngine::Core::Init()
{
    //print all user system info for now always at the very top

    Log::Print(
        KalaWindowCore::GetCPUInfoString() + "\n" +
        KalaWindowCore::GetGPUInfoString() + "\n" +
        KalaWindowCore::GetRAMInfoString(true) + "\n" +
        KalaWindowCore::GetOSInfoString() + "\n");

    GraphicsTemplateCore::Initialize();
    Render::Initialize();
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

void ElypsoEngine::Core::Shutdown()
{
    /*
    path lastImportedModel = Render::GetLastImportedModel();

    if (!lastImportedModel.empty())
    {
        ExportData::ExportAssetData(
            KalaWindowCore::GetExePath().parent_path() / "out.edat", 
            {{ lastImportedModel.stem(), lastImportedModel }});
    }
    */
}