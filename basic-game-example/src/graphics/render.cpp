//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include "log_utils.hpp"
#include "math_utils.hpp"
#include "key_standards.hpp"
#include "export_glb.hpp"

#include "core/ee_core.hpp"
#include "import/ee_import_data.hpp"
#include "export/ee_export_data.hpp"
#include "core/kw_core.hpp"
#include "core/kw_input.hpp"
#include "graphics/kw_window.hpp"
#include "graphics/kw_window_global.hpp"
#include "graphics/kg_context.hpp"
#include "graphics/kg_hit_test.hpp"
#include "graphics/kg_viewport.hpp"
#include "graphics/kg_shader.hpp"
#include "graphics/kg_camera.hpp"
#include "graphics/kg_material.hpp"
#include "graphics/kg_texture.hpp"
#include "graphics/kg_mesh.hpp"
#include "widgets/kg_text.hpp"
#include "import/kg_import_font.hpp"
#include "import/kg_import_mesh.hpp"

#include "core/core.hpp"
#include "graphics/render.hpp"

using KalaHeaders::KalaLog::Log;
using KalaHeaders::KalaLog::LogType;

using KalaHeaders::KalaMath::epsilon;
using KalaHeaders::KalaMath::vec2;
using KalaHeaders::KalaMath::vec3;
using KalaHeaders::KalaMath::vec4;
using KalaHeaders::KalaMath::quat;
using KalaHeaders::KalaMath::Transform2D;
using KalaHeaders::KalaMath::Transform3D;
using KalaHeaders::KalaMath::PosTarget;
using KalaHeaders::KalaMath::RotTarget;
using KalaHeaders::KalaMath::SizeTarget;

using KalaHeaders::KalaKeyStandards::KeyboardButton;

using KalaHeaders::KalaExportGLB::ExportMeshData;
using KalaHeaders::KalaExportGLB::ExportGLBTextureData;
using KalaHeaders::KalaExportGLB::ExportMaterialData;
using KalaHeaders::KalaExportGLB::ExportNodeData;

using ElypsoEngine::Core::EngineCore;
using ElypsoEngine::Import::AssetData;
using ElypsoEngine::Import::ImportData;
using ElypsoEngine::Export::UserBlock;
using ElypsoEngine::Export::ExportData;
using KalaWindow::Core::KalaWindowCore;
using KalaWindow::Core::Input;
using KalaWindow::Graphics::FileType;
using KalaWindow::Graphics::Window_Global;
using KalaGraphics::Graphics::VSyncState;
using KalaGraphics::Graphics::GraphicsContext;
using KalaGraphics::Graphics::Viewport;
using KalaGraphics::Graphics::Shader;
using KalaGraphics::Graphics::Camera;
using KalaGraphics::Graphics::AlphaMode;
using KalaGraphics::Graphics::Material;
using KalaGraphics::Graphics::TextureFilterMode;
using KalaGraphics::Graphics::TextureWrapMode;
using KalaGraphics::Graphics::TextureData;
using KalaGraphics::Graphics::Texture;
using KalaGraphics::Graphics::Vertex;
using KalaGraphics::Graphics::AnchorPosition;
using KalaGraphics::Graphics::Mesh_Cube;
using KalaGraphics::Graphics::MeshData;
using KalaGraphics::Graphics::Mesh;
using KalaGraphics::Widgets::Text;
using KalaGraphics::Import::ImportFont;
using KalaGraphics::Import::ImportNodeData;
using KalaGraphics::Import::ImportPrimitiveData;
using KalaGraphics::Import::ImportMaterialData;
using KalaGraphics::Import::ImportMesh;

using GraphicsTemplate::Core::GraphicsTemplateCore;

using std::string;
using std::to_string;
using std::vector;
using std::filesystem::path;

static bool isInitialized{};

static bool isCameraMovable{};

static ImportFont* font{};

static Texture* vp1_Tex_root{};
static Texture* vp1_Tex_fallback{};
static Texture* vp1_Tex_fallback_groundTest{};

static vector<Mesh*> vp1_Mesh3D_importedMeshes{};
static Mesh* vp1_Mesh3D_groundTest{};

static Text* vp1_Text_typeTest{};
static Mesh* vp1_Mesh2D_typeTest{};

static Mesh* vp1_Mesh2D_typeTestBackground{};

static path lastImportedMeshPath{};
static vector<Mesh*> lastImportedMeshPtrs{};

namespace GraphicsTemplate::Graphics
{
    bool Render::IsInitialized() { return isInitialized; }
    void Render::Initialize()
    {
        if (!GraphicsTemplateCore::IsInitialized())
        {
            Log::Print(
                "Failed to initialize renderer because core has not yet been initialized!",
                "BGE_RENDER",
                LogType::LOG_WARNING);

            return;
        }
        if (isInitialized)
        {
            Log::Print(
                "Failed to initialize renderer because it has already been initialized!",
                "BGE_RENDER",
                LogType::LOG_WARNING);

            return;
        }

        //sync before creating kg objects
        EngineCore::SyncID();

        const path& exePath = KalaWindowCore::GetExePath();

        Viewport* viewport = GraphicsTemplateCore::GetViewport();
        Shader* primary3DShader = GraphicsTemplateCore::Get3DShader();
        Shader* primary2DShader = GraphicsTemplateCore::Get2DShader();

        //
        // CREATE TEXTURES
        //

        Texture* fallbackTex{};
        string err = Texture::GetRegistry().GetContent(primary3DShader->GetFallbackTextureID(), fallbackTex);
        if (!err.empty())
        {
            KalaWindowCore::ForceClose(
                "Game render error",
                "Failed to get fallback texture from shader '" + to_string(primary3DShader->GetID()) + "'! Reason: " + err);
        }

        vp1_Tex_fallback_groundTest = CreateTexture(
            primary3DShader,
            {
                .pixelData = vector<u8>(fallbackTex->GetPixelData()),
                .filterMode = TextureFilterMode::FILTER_NEAREST,
                .wrapMode = TextureWrapMode::WRAP_REPEAT,
                .size = 16
            });

        //
        // CREATE GROUND
        //

        vp1_Mesh3D_groundTest = CreateMesh(
            primary3DShader,
            Mesh::GenerateMeshData(Mesh_Cube{.edgeCount = 4}));

        Material* groundMeshMat{};
        err = Material::GetRegistry().GetContent(vp1_Mesh3D_groundTest->GetMaterialID(), groundMeshMat);
        if (!err.empty())
        {
            KalaWindowCore::ForceClose(
                "Game render error",
                "Failed to initialize ground mesh because its material was invalid! Reason: " + err);
        }

        groundMeshMat->SetBaseColorTextureID(vp1_Tex_fallback_groundTest->GetID());

        vector<Vertex> vertices = vp1_Mesh3D_groundTest->GetVertices();
        vector<u32> indices = vp1_Mesh3D_groundTest->GetIndices();

        vec3 newSize = { 10.0f, epsilon, 10.0f }; 

        for (Vertex& v : vertices)
        {
            v.uv.x *= newSize.x;
            v.uv.y *= newSize.z;
        }

        vp1_Mesh3D_groundTest->SetMeshData(
        {
            .vertices = std::move(vertices),
            .indices = std::move(indices)
        });
        Transform3D& groundTransform = scast<Transform3D&>(vp1_Mesh3D_groundTest->GetTransform());
        groundTransform.setsize(newSize);
        groundTransform.update();

        //
        // SELECT AND INITIALIZE FONT
        //

        //initialize from selected font path

        /*
        vector<path> files = Window_Global::GetFiles(
            FileType::FILE_CUSTOM,
            {
                ".ttf",
                ".otf"
                });

        if (files.empty())
        {
            KalaWindowCore::ForceClose(
                "Game render error",
                "Failed to import font because no font was selected!");
        }

        string fontName = files.front().stem().string();
        font = ImportFont::Initialize(
            path(files.front()),
            64);
        */

        //initialize from known font path

        path fontName = path("LeagueGothic") / "LeagueGothic-Regular.otf";
        path fontPath = exePath.parent_path() / "files" / "fonts" / fontName;

        font = ImportFont::Initialize(
            path(fontPath),
            32);

        if (!font)
        {
            KalaWindowCore::ForceClose(
                "Game render error",
                "Failed to import font '" + fontName.string() + "'!");
        }

        /*
        Test_Print_Glyph_Atlas_To_Texture(
            font,
            vp1_Tex_font,
            vp1_Mesh2D_font,
            vp1_Mesh2D_fontBackground);
        */

        //
        // CREATE TEXT WIDGET
        //

        vp1_Text_typeTest = Text::Initialize(
            font->GetID(),
            viewport->GetID());
        
        //vp1_Text_typeTest->SetEditState(true);
        vp1_Text_typeTest->SetMaxCharacters(50);

        err = Mesh::GetRegistry().GetContent(vp1_Text_typeTest->GetMeshID(), vp1_Mesh2D_typeTest);
        if (!err.empty())
        {
            KalaWindowCore::ForceClose(
                "Game render error",
                "Failed to initialize text because its mesh '" 
                + to_string(vp1_Text_typeTest->GetMeshID()) + "' was invalid! Reason: " + err);
        }

        vp1_Mesh2D_typeTest->SetIgnoreHoverState(false);

        vp1_Mesh2D_typeTest->SetViewportAnchorPosition(AnchorPosition::P_TOP_LEFT);
        vp1_Mesh2D_typeTest->SetSelfAnchorPosition(AnchorPosition::P_TOP_LEFT);

        Material* typeTestMeshMat{};
        err = Material::GetRegistry().GetContent(vp1_Mesh2D_typeTest->GetMaterialID(), typeTestMeshMat);
        if (!err.empty())
        {
            KalaWindowCore::ForceClose(
                "Game render error",
                "Failed to initialize text because its mesh material '" 
                + to_string(vp1_Mesh2D_typeTest->GetMaterialID()) + "' was invalid! Reason: " + err);
        }
        typeTestMeshMat->SetBaseColor( vec4{0.0f, 0.0f, 0.0f, 1.0f} );
        typeTestMeshMat->SetAlphaMode(AlphaMode::A_BLEND);

        vp1_Mesh2D_typeTestBackground = Mesh::Initialize(primary2DShader->GetID());

        vp1_Mesh2D_typeTestBackground->SetViewportAnchorPosition(AnchorPosition::P_TOP_LEFT);
        vp1_Mesh2D_typeTestBackground->SetSelfAnchorPosition(AnchorPosition::P_TOP_LEFT);

        Material* typeTestBackgroundMat{};
        err = Material::GetRegistry().GetContent(vp1_Mesh2D_typeTestBackground->GetMaterialID(), typeTestBackgroundMat);
        if (!err.empty())
        {
            KalaWindowCore::ForceClose(
                "Game render error",
                "Failed to initialize text because its background mesh material '" 
                + to_string(vp1_Mesh2D_typeTestBackground->GetMaterialID()) + "' was invalid! Reason: " + err);
        }
        typeTestBackgroundMat->SetBaseColor( vec4{0.5f, 0.5f, 0.8f, 0.3f} );
        typeTestBackgroundMat->SetAlphaMode(AlphaMode::A_BLEND);

        //sync after kg objects are done with initialization
        EngineCore::SyncID();

        isInitialized = true;
    }

    void Render::Update()
    {
        if (!isInitialized) return;

        static Input* input = GraphicsTemplateCore::GetInput();
        static GraphicsContext* gctx = GraphicsTemplateCore::GetGraphicsContext();
        static Shader* primary3DShader = GraphicsTemplateCore::Get3DShader();
        static Camera* camera = GraphicsTemplateCore::GetCamera();

        ToggleCamera(input);

        MoveCamera(
            input,
            camera,
            EngineCore::GetDeltaTime());

        string err = ImportMeshes(
            input, 
            primary3DShader);
        if (!err.empty())
        {
            Log::Print(
                "Failed to import meshes! Reason: " + err,
                "GAME_RENDER",
                LogType::LOG_WARNING);
        }

        ToggleVSync(gctx, input);

        static string lastFPS{};
        string fps = GraphicsTemplateCore::GetFPS(0.5f)[0];

        if (lastFPS != ""
            && fps == "")
        {
            lastFPS = fps;
            vp1_Text_typeTest->SetText("");
        }
        if (lastFPS != fps)
        {
            lastFPS = fps;
            vp1_Text_typeTest->SetText(fps, true);
        }

        Transform2D& typeTestTransform = scast<Transform2D&>(vp1_Mesh2D_typeTest->GetTransform());
        Transform2D& typeTestBackgroundTransform = scast<Transform2D&>(vp1_Mesh2D_typeTestBackground->GetTransform());

        const vec2 textSize = typeTestTransform.getsize(SizeTarget::SIZE_LOCAL);
        const vec2 textBackgroundSize = typeTestBackgroundTransform.getsize(SizeTarget::SIZE_LOCAL);

        if (textBackgroundSize != textSize)
        {
            typeTestBackgroundTransform.setsize(textSize);
            typeTestBackgroundTransform.update();
        }

        /*
        Test_Print_Glyph_To_Texture(
            input,
            font,
            vp1_Tex_font,
            vp1_Mesh2D_font,
            vp1_Mesh2D_fontBackground);
        */
    }

    Texture* Render::GetRootTexture() { return vp1_Tex_root; }
    void Render::SetRootTexture(Texture* newValue) { vp1_Tex_root = newValue; }

    Texture* Render::GetFallbackTexture() { return vp1_Tex_fallback; }
    void Render::SetFallbackTexture(Texture* newValue) { vp1_Tex_fallback = newValue; }

    Texture* Render::GetGroundTexture() { return vp1_Tex_fallback_groundTest; }
    void Render::SetGroundTexture(Texture* newValue) { vp1_Tex_fallback_groundTest = newValue; }

    void Render::ToggleVSync(
        GraphicsContext* gctx,
        Input* input)
    {
        //if (input->IsComboPressed(combo_vsync_disable))
        if (input->IsKeyPressed(KeyboardButton::K_1))
        {
            Log::Print("@@@@@ set vsync state to off");
            gctx->SetVSyncState(VSyncState::VSYNC_OFF);
        }
        //else if (input->IsComboPressed(combo_vsync_adaptive))
        else if (input->IsKeyPressed(KeyboardButton::K_2))
        {
            Log::Print("@@@@@ set vsync state to adaptive");
            gctx->SetVSyncState(VSyncState::VSYNC_ON_ADAPTIVE);
        }
        //else if (input->IsComboPressed(combo_vsync_triple_buffered))
        if (input->IsKeyPressed(KeyboardButton::K_3))
        {
            Log::Print("@@@@@ set vsync state to triple buffered");
            gctx->SetVSyncState(VSyncState::VSYNC_ON_TRIPLE_BUFFERED);
        }
    }

    void Render::ToggleCamera(Input* input)
    {
        //if (input->IsComboPressed(combo_toggle_cam_move))
        if (input->IsKeyPressed(KeyboardButton::K_ESC))
        {
            isCameraMovable = !isCameraMovable;

            if (!isCameraMovable)
            {
                input->SetMouseLockState(false);
                input->SetMouseVisibility(true);
            }
            else
            {
                input->SetMouseLockState(true);
                input->SetMouseVisibility(false);
            }

            Log::Print("@@@@@ set cam move state to: " 
                + string(isCameraMovable ? "on" : "off"));
        }
    }

    void Render::MoveCamera(
        Input* input,
        Camera* cam,
        f32 deltaTime)
    {
        static f32 sprintMultiplier = 2.0f;

        if (!isCameraMovable) return;

        vec2 mouse = input->GetRawMouseDelta();
        vec2 kb{};
        f32 vertical{};

        //forwards
        if (input->IsKeyHeld(KeyboardButton::K_W)) kb.y += 1.0f;
        //backwards
        if (input->IsKeyHeld(KeyboardButton::K_S)) kb.y -= 1.0f;
        //left
        if (input->IsKeyHeld(KeyboardButton::K_A)) kb.x -= 1.0f;
        //right
        if (input->IsKeyHeld(KeyboardButton::K_D)) kb.x += 1.0f;

        //up
        if (input->IsKeyHeld(KeyboardButton::K_E)) vertical += 1.0f;
        //down
        if (input->IsKeyHeld(KeyboardButton::K_Q)) vertical -= 1.0f;

        //sprint
        if (input->IsKeyHeld(KeyboardButton::K_LEFT_SHIFT)) deltaTime *= sprintMultiplier;

        /*
        string moveStr = "\nmouse - x: " 
            + to_string(mouse.x) + ", y: "
            + to_string(mouse.y) + "\nkb - x: "
            + to_string(kb.x) + ", y: "
            + to_string(kb.y);

        Transform3D& ct = cam->GetTransform();
        vec3 euler = toeuler3(ct.rot_world);

        string transformStr = "\npos - x: "
            + to_string(ct.pos_world.x) + ", y: "
            + to_string(ct.pos_world.y) + ", z: "
            + to_string(ct.pos_world.z) + ", \nrot - x: "
            + to_string(euler.x) + ", y: "
            + to_string(euler.y) + ", z: "
            + to_string(euler.z) + ", \nsize - x: "
            + to_string(ct.size_world.x) + ", y: "
            + to_string(ct.size_world.y) + ", z: "
            + to_string(ct.size_world.z);

        Log::Print(
            moveStr + transformStr,
            "GAME_CAM_MOVE_TEST",
            LogType::LOG_VERBOSE);
        */

        cam->Move(
            mouse,
            kb,
            vertical,
            deltaTime);
    }

    path Render::GetLastImportedMeshPath() { return lastImportedMeshPath; }
    vector<Mesh*> Render::GetLastImportedMeshPointers() { return lastImportedMeshPtrs; }

    string Render::ImportMeshes(
        Input* input,
        Shader* shader)
    {
        if (!input) return "Input was invalid!";
        if (!shader) return "Shader was invalid!";

        if (input->IsKeyPressed(KeyboardButton::K_SPACE))
        {
            vector<path> files = Window_Global::GetFiles(
                FileType::FILE_CUSTOM,
                {
                    ".glb",
                    ".gltf",
                    ".edat"
                    });

            if (files.empty()) return "No assets were selected!";

            ImportMesh* importMesh{};

            if (files.front().extension() == ".edat")
            {
                string err = ImportData::ImportAssetData(path(files.front()));
                if (!err.empty()) return err;

                vector<AssetData> assetData = ImportData::GetAssetData();
                if (assetData.empty()) return "GetAssetData returned empty vector!";

                vector<AssetData> streamedAssets{};
                err = ImportData::StreamAssets(
                    { assetData.front().assetName },
                    streamedAssets);

                if (!err.empty()) return err;

                importMesh = ImportMesh::Initialize(std::move(streamedAssets.front().assetData));
            }
            else importMesh = ImportMesh::Initialize(path(files.front()));

            if (!importMesh) return "ImportMesh couldn't be initialized!";

            vector<u32> meshIDs{};

            if (!lastImportedMeshPtrs.empty())
            {
                for (Mesh* m : lastImportedMeshPtrs) m->Destroy();
                lastImportedMeshPtrs.clear();
            }

            for (const ImportNodeData& nodeData : importMesh->GetMeshData())
            {
                for (const ImportPrimitiveData& primitiveData : nodeData.primitiveData)
                {
                    Mesh* primitive = Mesh::Initialize(shader->GetID());

                    if (!primitive)
                    {
                        KalaWindowCore::ForceClose(
                            "Game render error",
                            "Failed to import model '" + files.front().string() 
                            + "' because one of its primitives failed to initialize!");
                    }

                    Material* mat{};
                    string err = Material::GetRegistry().GetContent(primitive->GetMaterialID(), mat);
                    if (!err.empty())
                    {
                        KalaWindowCore::ForceClose(
                            "Game render error",
                            "Failed to initialize mesh because its material was invalid! Reason: " + err);
                    }

                    primitive->SetMeshData(
                    {
                        .vertices = vector<Vertex>(primitiveData.meshData.vertices),
                        .indices = vector<u32>(primitiveData.meshData.indices)
                    });

                    Transform3D& mt = primitive->GetTransform();
                    mt.setpos(nodeData.transform.getpos(PosTarget::POS_LOCAL));
                    mt.setrotquat(nodeData.transform.getrotquat(RotTarget::ROT_LOCAL));
                    mt.setsize(nodeData.transform.getsize(SizeTarget::SIZE_LOCAL));
                    mt.update();

                    primitive->FlipFaceDirection();

                    const ImportMaterialData& matData = primitiveData.matData;

                    mat->SetBaseColor(vec4(matData.baseColor));

                    if (matData.alphaMode != AlphaMode::A_OPAQUE)
                    {
                        mat->SetAlphaMode(matData.alphaMode);

                        if (matData.alphaMode == AlphaMode::A_MASK)
                        {
                            mat->SetAlphaCutoff(matData.alphaCutoff);
                        }
                    }

                    if (!matData.textureData.pixelData.empty())
                    {
                        Texture* meshTex = Texture::Initialize(shader->GetID());

                        meshTex->SetPixelData(vector<u8>(matData.textureData.pixelData));
                        meshTex->SetSize(matData.textureData.size);
                        meshTex->SetPixelFormat(matData.textureData.pixelFormat);

                        mat->SetBaseColorTextureID(meshTex->GetID());
                    }

                    lastImportedMeshPtrs.push_back(primitive);

                    meshIDs.push_back(primitive->GetID());
                }
            }

            if (files.front().extension() != ".edat")
            {
                lastImportedMeshPath = files.front();
            }

            static int exportIndex{};

            //temporarily exporting meshes right after import succeeds
            string err = ExportMeshes(
                lastImportedMeshPath.parent_path() 
                / string(lastImportedMeshPath.stem().string() + "_" + to_string(++exportIndex) + ".edat"),
                lastImportedMeshPtrs);

            if (!err.empty())
            {
                Log::Print(
                    "Failed to export meshes! Reason: " + err,
                    "GAME_RENDER",
                    LogType::LOG_WARNING);
            }
        }

        return {};
    }

    string Render::ExportMeshes(
        const path& targetPath,
        const vector<Mesh*>& meshes)
    {
        if (exists(targetPath))
        {
            return "Failed to export meshes because target path already exists!";
        }
        if (!targetPath.has_extension()
            || (targetPath.extension() != ".glb"
            && targetPath.extension() != ".edat"))
        {
            return "Failed to export meshes because target path had invalid or no extension!";
        }
        if (meshes.empty())
        {
            return "Failed to export meshes because no meshes were selected!";
        }

        for (const Mesh* m : meshes)
        {
            if (!m) return "Failed to export meshes because one or more of the meshes were invalid!";
        }

        if (targetPath.extension() == ".glb")
        {
            vector<ExportNodeData> nodeData{};

            u32 index{};
            for (Mesh* m : meshes)
            {
                ExportNodeData node{};
                node.nodeName = "model_" + to_string(index++);

                const Transform3D& t = scast<Transform3D&>(m->GetTransform());
                const vec3 pos = t.getpos(PosTarget::POS_LOCAL);
                const quat rot = t.getrotquat(RotTarget::ROT_LOCAL);
                const vec3 size = t.getsize(SizeTarget::SIZE_LOCAL);
                node.transform = 
                    {
                        .position = { pos.x, pos.y, pos.z },
                        .rotation = { rot.x, rot.y, rot.z, rot.w },
                        .size = { size.x, size.y, size.z }
                    };

                const vector<Vertex>& vertices = m->GetVertices();
                vector<u32> indices = m->GetIndices();

                vector<KalaHeaders::KalaExportGLB::Vertex> exportVertices(vertices.size());
                for (size_t i = 0; i < vertices.size(); i++)
                {
                    const Vertex& v = vertices[i];
                    KalaHeaders::KalaExportGLB::Vertex& ev = exportVertices[i];

                    ev.pos[0] = v.pos.x;
                    ev.pos[1] = v.pos.y;
                    ev.pos[2] = v.pos.z;

                    ev.normal[0] = v.normal.x;
                    ev.normal[1] = v.normal.y;
                    ev.normal[2] = v.normal.z;

                    ev.uv[0] = v.uv.x;
                    ev.uv[1] = v.uv.y;

                    ev.color[0] = v.color.x;
                    ev.color[1] = v.color.y;
                    ev.color[2] = v.color.z;
                    ev.color[3] = v.color.w;
                }

                node.meshData.vertices = std::move(exportVertices);
                node.meshData.indices = std::move(indices);

                Material* mat{};
                string err = Material::GetRegistry().GetContent(m->GetMaterialID(), mat);
                if (!err.empty())
                {
                    KalaWindowCore::ForceClose(
                        "Game render error",
                        "Failed to get mesh '" + to_string(m->GetID()) 
                        + "' material because it was invalid! Reason: " + err);
                }

                Texture* tex{};
                err = Texture::GetRegistry().GetContent(mat->GetBaseColorTextureID(), tex);
                if (!err.empty())
                {
                    KalaWindowCore::ForceClose(
                        "Game render error",
                        "Failed to get mesh '" + to_string(m->GetID()) 
                        + "' base color texture because it was invalid! Reason: " + err);
                }

                const vec4& baseColor = mat->GetBaseColor();
                node.matData.baseColor[0] = baseColor.x;
                node.matData.baseColor[1] = baseColor.y;
                node.matData.baseColor[2] = baseColor.z;
                node.matData.baseColor[3] = baseColor.w;

                node.matData.alphaMode = scast<KalaHeaders::KalaExportGLB::AlphaMode>(scast<u8>(mat->GetAlphaMode()));
                node.matData.alphaCutoff = mat->GetAlphaCutoff();

                node.matData.textureData.pngImageData = tex->GetPixelData();

                nodeData.push_back(std::move(node));
            }

            string err = KalaHeaders::KalaExportGLB::ExportMeshes(
                std::move(nodeData),
                targetPath);
            if (!err.empty()) return err;
        }
        else
        {
            path outPath{};
            size_t relevantMeshCount{};

            for (const Mesh* m : meshes)
            {
                for (const Mesh* lastMesh : lastImportedMeshPtrs)
                {
                    if (m->GetID() == lastMesh->GetID())
                    {
                        outPath = lastImportedMeshPath;
                        relevantMeshCount++;
                        break;
                    }
                }
            }

            if (outPath.empty())
            {
                return 
                    "Failed to export meshes because none of the given "
                    "meshes matched the last imported meshes "
                    "so their original mesh path could not be assigned!";
            }
            if (relevantMeshCount != meshes.size())
            {
                return 
                    "Failed to export meshes because one of the meshes "
                    "did not match any of the last imported meshes "
                    "so their original mesh path could not be assigned!";
            }

            string err = ExportData::ExportAssetData(
                targetPath,
                {{ 
                    .blockName = "mesh_" + lastImportedMeshPath.stem().string(),
                    .blockPath = lastImportedMeshPath
                }});
            if (!err.empty()) return err;
        }

        return "";
    }

    Texture* Render::CreateTexture(
        Shader* shader,
        TextureData&& textureData)
    {
        //sync ids before generating texture
        EngineCore::SyncID();

        Texture* texture = Texture::Initialize(
            shader->GetID(),
            std::move(textureData));
        if (!texture)
        {
            KalaWindowCore::ForceClose(
                "Game render error",
                "Failed to initialize test texture!");
        }

        return texture;
    }

    Mesh* Render::CreateMesh(
        Shader* shader,
        MeshData&& meshData)
    {
        //sync ids before generating mesh
        EngineCore::SyncID();

        Mesh* mesh = Mesh::Initialize(shader->GetID());
        if (!mesh)
        {
            KalaWindowCore::ForceClose(
                "Game render error",
                "Failed to initialize test mesh!");
        }

        Material* mat{};
        string err = Material::GetRegistry().GetContent(mesh->GetMaterialID(), mat);
        if (!err.empty())
        {
            KalaWindowCore::ForceClose(
                "Game render error",
                "Failed to initialize test mesh because its material was invalid! Reason: " + err);
        }

        if (!mesh->Is2D()) mesh->SetMeshData(std::move(meshData));

        return mesh;
    }
}