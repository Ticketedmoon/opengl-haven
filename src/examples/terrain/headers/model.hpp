#pragma once

#include "mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../../../../../headers/stb_image.h"

class Model 
{
    public:
        Model(char* path);
        void Draw(Shader &shader);	

    public:
        std::vector<Mesh> meshes;
        std::vector<Texture> textures_loaded;

    private:
        // model data
        std::string directory;

        void loadModel(std::string path);
        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName);
        unsigned int TextureFromFile(const char* path, const std::string& directory);
};
