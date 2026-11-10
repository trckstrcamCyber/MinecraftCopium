#pragma once
// neoLegacy - minimal glTF 2.0 loader for Blockbench exports.
//
// Supports exactly what Blockbench's glTF exporter writes:
//   * one scene of nodes with translation / rotation / scale (no skins, no matrices)
//   * meshes made of TRIANGLES with POSITION / NORMAL / TEXCOORD_0 and an optional index buffer
//   * buffers embedded as base64 data: URIs
//   * animations that drive node translation / rotation / scale (LINEAR, STEP, or CUBICSPLINE read as linear)
//
// This file deliberately depends on nothing but the C++ standard library so it can be tested on its own.
#include <string>
#include <vector>
#include <cstddef>

class GltfModel
{
public:
    struct Vertex { float x, y, z, nx, ny, nz, u, v; };

    struct Primitive
    {
        int material = 0;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;   // always a triangle list; generated if the file had none
    };

    struct Node
    {
        std::string name;
        int parent = -1;
        std::vector<int> children;
        float t[3] = {0, 0, 0};
        float r[4] = {0, 0, 0, 1};
        float s[3] = {1, 1, 1};
        int mesh = -1;
    };

    // path: 0 = translation (3 floats), 1 = rotation (4 floats, xyzw), 2 = scale (3 floats)
    struct Channel
    {
        int node = -1;
        int path = 0;
        bool step = false;
        std::vector<float> times;
        std::vector<float> values;   // times.size() * (3 or 4)
    };

    struct Clip
    {
        std::string name;
        float duration = 0.0f;
        std::vector<Channel> channels;
    };

    // Local TRS of every node (what a clip overrides).
    struct Pose
    {
        std::vector<float> t;   // 3 per node
        std::vector<float> r;   // 4 per node
        std::vector<float> s;   // 3 per node
    };

    bool loadFromMemory(const char *data, size_t size, std::string &error);
    bool loadFromFile(const std::string &path, std::string &error);

    bool isLoaded() const { return loaded; }
    int  findClip(const std::string &name) const;       // -1 if missing
    int  findNode(const std::string &name) const;       // first node with that name, -1 if missing
    std::string materialTextureName(int material) const; // lower-case name of the material's base colour texture

    void resetPose(Pose &pose) const;                                         // every node back to its rest TRS
    void applyClip(Pose &pose, int clip, float timeSeconds) const;            // override the channels the clip animates
    void computeWorld(const Pose &pose, std::vector<float> &world16) const;   // 16 floats per node, row-major, m[row*4+col]

    std::vector<Node> nodes;
    std::vector<int> rootNodes;
    std::vector<std::vector<Primitive> > meshes;
    std::vector<Clip> clips;

private:
    bool loaded = false;
    std::vector<std::string> materialTextures;
};
