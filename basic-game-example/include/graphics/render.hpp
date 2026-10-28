//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <vector>

#include "core_utils.hpp"

#include "core/kw_input.hpp"
#include "graphics/kg_context.hpp"
#include "graphics/kg_shader.hpp"
#include "graphics/kg_camera.hpp"
#include "graphics/kg_texture.hpp"
#include "graphics/kg_mesh.hpp"

namespace GraphicsTemplate::Graphics
{
    using KalaWindow::Core::Input;
    using KalaGraphics::Graphics::GraphicsContext;
    using KalaGraphics::Graphics::Shader;
    using KalaGraphics::Graphics::Camera;
    using KalaGraphics::Graphics::TextureData;
    using KalaGraphics::Graphics::Texture;
    using KalaGraphics::Graphics::MeshData;
    using KalaGraphics::Graphics::Mesh;

    using std::vector;

    class LIB_API Render
    {
    public:
        KNODISCARD
        static bool IsInitialized();
        static void Initialize();

        static void Update();

        KNODISCARD
        static Texture* GetRootTexture();
        static void SetRootTexture(Texture* newValue);
        
        KNODISCARD
        static Texture* GetFallbackTexture();
        static void SetFallbackTexture(Texture* newValue);

        KNODISCARD
        static Texture* GetGroundTexture();
        static void SetGroundTexture(Texture* newValue);

        static void ToggleVSync(
            GraphicsContext* gctx,
            Input* input);

        static void ToggleCamera(Input* input);

        static void MoveCamera(
            Input* input,
            Camera* cam,
            f32 deltaTime);

        static void ImportMeshes(
            Input* input,
            Shader* shader);

        KNODISCARD
        static Texture* CreateTexture(
            Shader* shader,
            TextureData&& textureData);

        KNODISCARD
        static Mesh* CreateMesh(
            Shader* shader,
            MeshData&& meshData);
    };
}