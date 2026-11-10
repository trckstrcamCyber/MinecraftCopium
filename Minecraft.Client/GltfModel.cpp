#include "GltfModel.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <map>
#include <memory>
#include <algorithm>

namespace
{
    // ---------------------------------------------------------------------------------------------------------------
    // Tiny JSON reader (just enough for glTF)
    // ---------------------------------------------------------------------------------------------------------------
    struct JsonValue
    {
        enum Type { Null, Bool, Number, String, Array, Object } type = Null;
        double number = 0.0;
        bool boolean = false;
        std::string string;
        std::vector<JsonValue> array;
        std::vector<std::pair<std::string, JsonValue> > object;

        const JsonValue *get(const char *key) const
        {
            if (type != Object) return nullptr;
            for (size_t i = 0; i < object.size(); ++i) if (object[i].first == key) return &object[i].second;
            return nullptr;
        }
        double num(const char *key, double fallback) const
        {
            const JsonValue *v = get(key);
            return (v && v->type == Number) ? v->number : fallback;
        }
        std::string str(const char *key) const
        {
            const JsonValue *v = get(key);
            return (v && v->type == String) ? v->string : std::string();
        }
    };

    class JsonParser
    {
    public:
        JsonParser(const char *d, size_t n) : p(d), end(d + n), failed(false) {}
        bool parse(JsonValue &out) { skip(); value(out); skip(); return !failed; }
        std::string error;
    private:
        const char *p;
        const char *end;
        bool failed;

        void fail(const char *msg) { if (!failed) { failed = true; error = msg; } }
        void skip() { while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) ++p; }
        bool eat(char c) { skip(); if (p < end && *p == c) { ++p; return true; } return false; }

        void value(JsonValue &v)
        {
            if (failed) return;
            skip();
            if (p >= end) { fail("unexpected end of JSON"); return; }
            char c = *p;
            if (c == '{') object(v);
            else if (c == '[') array(v);
            else if (c == '"') { v.type = JsonValue::String; stringLiteral(v.string); }
            else if (c == 't' && end - p >= 4 && !strncmp(p, "true", 4)) { v.type = JsonValue::Bool; v.boolean = true; p += 4; }
            else if (c == 'f' && end - p >= 5 && !strncmp(p, "false", 5)) { v.type = JsonValue::Bool; v.boolean = false; p += 5; }
            else if (c == 'n' && end - p >= 4 && !strncmp(p, "null", 4)) { v.type = JsonValue::Null; p += 4; }
            else if (c == '-' || (c >= '0' && c <= '9'))
            {
                char *e = nullptr;
                v.type = JsonValue::Number;
                v.number = strtod(p, &e);
                if (e == p) { fail("bad number"); return; }
                p = e;
            }
            else fail("unexpected character in JSON");
        }

        void stringLiteral(std::string &out)
        {
            ++p; // opening quote
            while (p < end && *p != '"')
            {
                if (*p == '\\' && p + 1 < end)
                {
                    ++p;
                    switch (*p)
                    {
                    case 'n': out += '\n'; break;
                    case 't': out += '\t'; break;
                    case 'r': out += '\r'; break;
                    case 'b': out += '\b'; break;
                    case 'f': out += '\f'; break;
                    case 'u':
                        // keep ASCII range, replace anything else (names only need to be recognisable)
                        if (end - p >= 5)
                        {
                            unsigned int code = (unsigned int)strtoul(std::string(p + 1, 4).c_str(), nullptr, 16);
                            out += (code < 128) ? (char)code : '?';
                            p += 4;
                        }
                        break;
                    default: out += *p; break; // " \ /
                    }
                    ++p;
                }
                else out += *p++;
            }
            if (p >= end) { fail("unterminated string"); return; }
            ++p; // closing quote
        }

        void array(JsonValue &v)
        {
            v.type = JsonValue::Array;
            ++p;
            skip();
            if (p < end && *p == ']') { ++p; return; }
            while (!failed)
            {
                v.array.push_back(JsonValue());
                value(v.array.back());
                if (eat(',')) continue;
                if (eat(']')) return;
                fail("expected , or ] in array");
            }
        }

        void object(JsonValue &v)
        {
            v.type = JsonValue::Object;
            ++p;
            skip();
            if (p < end && *p == '}') { ++p; return; }
            while (!failed)
            {
                skip();
                if (p >= end || *p != '"') { fail("expected string key"); return; }
                std::string key;
                stringLiteral(key);
                if (!eat(':')) { fail("expected :"); return; }
                v.object.push_back(std::make_pair(key, JsonValue()));
                value(v.object.back().second);
                if (eat(',')) continue;
                if (eat('}')) return;
                fail("expected , or } in object");
            }
        }
    };

    // ---------------------------------------------------------------------------------------------------------------
    bool base64Decode(const char *s, size_t n, std::vector<unsigned char> &out)
    {
        static signed char table[256];
        static bool init = false;
        if (!init)
        {
            memset(table, -1, sizeof(table));
            const char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            for (int i = 0; i < 64; ++i) table[(unsigned char)alphabet[i]] = (signed char)i;
            init = true;
        }
        out.clear();
        out.reserve(n / 4 * 3);
        unsigned int acc = 0;
        int bits = 0;
        for (size_t i = 0; i < n; ++i)
        {
            unsigned char c = (unsigned char)s[i];
            if (c == '=' || c == '\n' || c == '\r' || c == ' ') continue;
            signed char v = table[c];
            if (v < 0) return false;
            acc = (acc << 6) | (unsigned int)v;
            bits += 6;
            if (bits >= 8)
            {
                bits -= 8;
                out.push_back((unsigned char)((acc >> bits) & 0xFF));
            }
        }
        return true;
    }

    struct BufferData { std::vector<unsigned char> bytes; };

    // Reads accessor `index` into floats (converted from whatever component type it uses). `components` = 1..4.
    bool readAccessor(const JsonValue &root, const std::vector<BufferData> &buffers, int index, std::vector<double> &out, int &components)
    {
        const JsonValue *accessors = root.get("accessors");
        const JsonValue *views = root.get("bufferViews");
        if (!accessors || !views || index < 0 || index >= (int)accessors->array.size()) return false;
        const JsonValue &a = accessors->array[index];
        int viewIndex = (int)a.num("bufferView", -1);
        if (viewIndex < 0 || viewIndex >= (int)views->array.size()) return false;
        const JsonValue &view = views->array[viewIndex];
        int bufferIndex = (int)view.num("buffer", 0);
        if (bufferIndex < 0 || bufferIndex >= (int)buffers.size()) return false;
        const std::vector<unsigned char> &bytes = buffers[bufferIndex].bytes;

        std::string type = a.str("type");
        components = (type == "SCALAR") ? 1 : (type == "VEC2") ? 2 : (type == "VEC3") ? 3 : (type == "VEC4") ? 4 : 0;
        if (components == 0) return false;
        int componentType = (int)a.num("componentType", 5126);
        int compSize = (componentType == 5126 || componentType == 5125) ? 4 : (componentType == 5123 || componentType == 5122) ? 2 : 1;
        size_t count = (size_t)a.num("count", 0);
        size_t offset = (size_t)view.num("byteOffset", 0) + (size_t)a.num("byteOffset", 0);
        size_t stride = (size_t)view.num("byteStride", 0);
        if (stride == 0) stride = (size_t)(compSize * components);

        out.resize(count * (size_t)components);
        for (size_t i = 0; i < count; ++i)
        {
            size_t at = offset + i * stride;
            if (at + (size_t)(compSize * components) > bytes.size()) return false;
            for (int c = 0; c < components; ++c)
            {
                const unsigned char *src = &bytes[at + (size_t)(c * compSize)];
                double v = 0.0;
                if (componentType == 5126) { float f; memcpy(&f, src, 4); v = f; }
                else if (componentType == 5125) { unsigned int u; memcpy(&u, src, 4); v = u; }
                else if (componentType == 5123) { unsigned short u; memcpy(&u, src, 2); v = u; }
                else if (componentType == 5122) { short u; memcpy(&u, src, 2); v = u; }
                else if (componentType == 5121) { v = *src; }
                else if (componentType == 5120) { v = (signed char)*src; }
                out[i * (size_t)components + (size_t)c] = v;
            }
        }
        return true;
    }

    void quatToMatrix(const float q[4], float s[3], float t[3], float m[16])
    {
        float x = q[0], y = q[1], z = q[2], w = q[3];
        float n = sqrtf(x * x + y * y + z * z + w * w);
        if (n > 0.0f) { x /= n; y /= n; z /= n; w /= n; } else { x = y = z = 0.0f; w = 1.0f; }
        m[0] = (1 - 2 * (y * y + z * z)) * s[0]; m[1] = (2 * (x * y - z * w)) * s[1];     m[2] = (2 * (x * z + y * w)) * s[2];     m[3] = t[0];
        m[4] = (2 * (x * y + z * w)) * s[0];     m[5] = (1 - 2 * (x * x + z * z)) * s[1]; m[6] = (2 * (y * z - x * w)) * s[2];     m[7] = t[1];
        m[8] = (2 * (x * z - y * w)) * s[0];     m[9] = (2 * (y * z + x * w)) * s[1];     m[10] = (1 - 2 * (x * x + y * y)) * s[2]; m[11] = t[2];
        m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
    }

    void multiply(const float a[16], const float b[16], float out[16])
    {
        float r[16];
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                r[i * 4 + j] = a[i * 4 + 0] * b[0 * 4 + j] + a[i * 4 + 1] * b[1 * 4 + j] + a[i * 4 + 2] * b[2 * 4 + j] + a[i * 4 + 3] * b[3 * 4 + j];
        memcpy(out, r, sizeof(r));
    }

    std::string lowerCase(std::string s)
    {
        for (size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
        return s;
    }
}

bool GltfModel::loadFromFile(const std::string &path, std::string &error)
{
    FILE *f = fopen(path.c_str(), "rb");
    if (!f) { error = "cannot open " + path; return false; }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) { fclose(f); error = "empty file " + path; return false; }
    std::vector<char> data((size_t)size);
    size_t got = fread(&data[0], 1, (size_t)size, f);
    fclose(f);
    if (got != (size_t)size) { error = "short read " + path; return false; }
    return loadFromMemory(&data[0], data.size(), error);
}

bool GltfModel::loadFromMemory(const char *data, size_t size, std::string &error)
{
    loaded = false;
    nodes.clear(); rootNodes.clear(); meshes.clear(); clips.clear(); materialTextures.clear();

    JsonValue root;
    JsonParser parser(data, size);
    if (!parser.parse(root) || root.type != JsonValue::Object) { error = "JSON error: " + parser.error; return false; }

    // ---- buffers -------------------------------------------------------------------------------------------------
    std::vector<BufferData> buffers;
    const JsonValue *jb = root.get("buffers");
    if (!jb) { error = "no buffers"; return false; }
    for (size_t i = 0; i < jb->array.size(); ++i)
    {
        std::string uri = jb->array[i].str("uri");
        size_t comma = uri.find(',');
        if (uri.compare(0, 5, "data:") != 0 || comma == std::string::npos) { error = "buffer is not an embedded data: URI"; return false; }
        BufferData b;
        if (!base64Decode(uri.c_str() + comma + 1, uri.size() - comma - 1, b.bytes)) { error = "bad base64 in buffer"; return false; }
        buffers.push_back(b);
    }

    // ---- materials -> texture names ---------------------------------------------------------------------------------
    const JsonValue *jm = root.get("materials");
    const JsonValue *jt = root.get("textures");
    const JsonValue *ji = root.get("images");
    if (jm)
    {
        for (size_t i = 0; i < jm->array.size(); ++i)
        {
            std::string name;
            const JsonValue *pbr = jm->array[i].get("pbrMetallicRoughness");
            const JsonValue *bct = pbr ? pbr->get("baseColorTexture") : nullptr;
            if (bct && jt)
            {
                int ti = (int)bct->num("index", -1);
                if (ti >= 0 && ti < (int)jt->array.size())
                {
                    name = jt->array[ti].str("name");
                    if (name.empty() && ji)
                    {
                        int si = (int)jt->array[ti].num("source", -1);
                        if (si >= 0 && si < (int)ji->array.size()) name = ji->array[si].str("name");
                    }
                }
            }
            if (name.empty()) name = jm->array[i].str("name");
            materialTextures.push_back(lowerCase(name));
        }
    }

    // ---- meshes ---------------------------------------------------------------------------------------------------
    const JsonValue *jmesh = root.get("meshes");
    if (jmesh)
    {
        for (size_t mi = 0; mi < jmesh->array.size(); ++mi)
        {
            std::vector<Primitive> prims;
            const JsonValue *jp = jmesh->array[mi].get("primitives");
            if (jp)
            {
                for (size_t pi = 0; pi < jp->array.size(); ++pi)
                {
                    const JsonValue &prim = jp->array[pi];
                    const JsonValue *attrs = prim.get("attributes");
                    if (!attrs) continue;
                    if ((int)prim.num("mode", 4) != 4) continue;   // triangles only
                    std::vector<double> pos, nor, uv; int pc = 0, nc = 0, uc = 0;
                    if (!readAccessor(root, buffers, (int)attrs->num("POSITION", -1), pos, pc) || pc != 3) continue;
                    bool hasN = readAccessor(root, buffers, (int)attrs->num("NORMAL", -1), nor, nc) && nc == 3;
                    bool hasUV = readAccessor(root, buffers, (int)attrs->num("TEXCOORD_0", -1), uv, uc) && uc == 2;

                    Primitive out;
                    out.material = (int)prim.num("material", 0);
                    size_t vc = pos.size() / 3;
                    out.vertices.resize(vc);
                    for (size_t v = 0; v < vc; ++v)
                    {
                        Vertex &vx = out.vertices[v];
                        vx.x = (float)pos[v * 3]; vx.y = (float)pos[v * 3 + 1]; vx.z = (float)pos[v * 3 + 2];
                        vx.nx = hasN ? (float)nor[v * 3] : 0.0f; vx.ny = hasN ? (float)nor[v * 3 + 1] : 1.0f; vx.nz = hasN ? (float)nor[v * 3 + 2] : 0.0f;
                        vx.u = hasUV ? (float)uv[v * 2] : 0.0f; vx.v = hasUV ? (float)uv[v * 2 + 1] : 0.0f;
                    }
                    std::vector<double> idx; int ic = 0;
                    if (prim.get("indices") && readAccessor(root, buffers, (int)prim.num("indices", -1), idx, ic))
                    {
                        for (size_t k = 0; k + 2 < idx.size(); k += 3)
                        {
                            if (idx[k] >= vc || idx[k + 1] >= vc || idx[k + 2] >= vc) continue;
                            out.indices.push_back((unsigned int)idx[k]); out.indices.push_back((unsigned int)idx[k + 1]); out.indices.push_back((unsigned int)idx[k + 2]);
                        }
                    }
                    else
                    {
                        for (size_t k = 0; k + 2 < vc; k += 3) { out.indices.push_back((unsigned int)k); out.indices.push_back((unsigned int)k + 1); out.indices.push_back((unsigned int)k + 2); }
                    }
                    prims.push_back(out);
                }
            }
            meshes.push_back(prims);
        }
    }

    // ---- nodes ----------------------------------------------------------------------------------------------------
    const JsonValue *jn = root.get("nodes");
    if (!jn || jn->array.empty()) { error = "no nodes"; return false; }
    nodes.resize(jn->array.size());
    for (size_t i = 0; i < jn->array.size(); ++i)
    {
        const JsonValue &n = jn->array[i];
        Node &node = nodes[i];
        node.name = n.str("name");
        node.mesh = (int)n.num("mesh", -1);
        const JsonValue *t = n.get("translation"); if (t && t->array.size() == 3) for (int k = 0; k < 3; ++k) node.t[k] = (float)t->array[k].number;
        const JsonValue *r = n.get("rotation");    if (r && r->array.size() == 4) for (int k = 0; k < 4; ++k) node.r[k] = (float)r->array[k].number;
        const JsonValue *s = n.get("scale");       if (s && s->array.size() == 3) for (int k = 0; k < 3; ++k) node.s[k] = (float)s->array[k].number;
        const JsonValue *c = n.get("children");
        if (c) for (size_t k = 0; k < c->array.size(); ++k) node.children.push_back((int)c->array[k].number);
    }
    for (size_t i = 0; i < nodes.size(); ++i)
        for (size_t k = 0; k < nodes[i].children.size(); ++k)
        {
            int child = nodes[i].children[k];
            if (child >= 0 && child < (int)nodes.size()) nodes[child].parent = (int)i;
        }
    const JsonValue *scenes = root.get("scenes");
    int sceneIndex = (int)root.num("scene", 0);
    if (scenes && sceneIndex >= 0 && sceneIndex < (int)scenes->array.size())
    {
        const JsonValue *sn = scenes->array[sceneIndex].get("nodes");
        if (sn) for (size_t k = 0; k < sn->array.size(); ++k) rootNodes.push_back((int)sn->array[k].number);
    }
    if (rootNodes.empty())
        for (size_t i = 0; i < nodes.size(); ++i) if (nodes[i].parent < 0) rootNodes.push_back((int)i);

    // ---- animations ------------------------------------------------------------------------------------------------
    const JsonValue *ja = root.get("animations");
    if (ja)
    {
        for (size_t ai = 0; ai < ja->array.size(); ++ai)
        {
            const JsonValue &an = ja->array[ai];
            Clip clip;
            clip.name = an.str("name");
            const JsonValue *samplers = an.get("samplers");
            const JsonValue *chans = an.get("channels");
            if (!samplers || !chans) { clips.push_back(clip); continue; }
            for (size_t ci = 0; ci < chans->array.size(); ++ci)
            {
                const JsonValue &ch = chans->array[ci];
                const JsonValue *target = ch.get("target");
                if (!target) continue;
                std::string path = target->str("path");
                int pathId = (path == "translation") ? 0 : (path == "rotation") ? 1 : (path == "scale") ? 2 : -1;
                int nodeIndex = (int)target->num("node", -1);
                int samplerIndex = (int)ch.num("sampler", -1);
                if (pathId < 0 || nodeIndex < 0 || nodeIndex >= (int)nodes.size() || samplerIndex < 0 || samplerIndex >= (int)samplers->array.size()) continue;
                const JsonValue &sampler = samplers->array[samplerIndex];
                std::vector<double> times, values; int tc = 0, vcmp = 0;
                if (!readAccessor(root, buffers, (int)sampler.num("input", -1), times, tc) || tc != 1) continue;
                if (!readAccessor(root, buffers, (int)sampler.num("output", -1), values, vcmp)) continue;
                int width = (pathId == 1) ? 4 : 3;
                if (vcmp != width) continue;
                std::string interp = sampler.str("interpolation");

                Channel out;
                out.node = nodeIndex;
                out.path = pathId;
                out.step = (interp == "STEP");
                size_t keys = times.size();
                bool cubic = (interp == "CUBICSPLINE");
                if (cubic && values.size() >= keys * 3 * (size_t)width) { /* [in-tangent, value, out-tangent] per key: keep the value */ }
                for (size_t k = 0; k < keys; ++k)
                {
                    out.times.push_back((float)times[k]);
                    size_t base = cubic ? (k * 3 + 1) * (size_t)width : k * (size_t)width;
                    for (int c = 0; c < width; ++c) out.values.push_back(base + (size_t)c < values.size() ? (float)values[base + (size_t)c] : 0.0f);
                }
                if (keys > 0 && out.times[keys - 1] > clip.duration) clip.duration = out.times[keys - 1];
                clip.channels.push_back(out);
            }
            clips.push_back(clip);
        }
    }

    loaded = true;
    return true;
}

int GltfModel::findClip(const std::string &name) const
{
    for (size_t i = 0; i < clips.size(); ++i) if (clips[i].name == name) return (int)i;
    return -1;
}

int GltfModel::findNode(const std::string &name) const
{
    for (size_t i = 0; i < nodes.size(); ++i) if (nodes[i].name == name) return (int)i;
    return -1;
}

std::string GltfModel::materialTextureName(int material) const
{
    return (material >= 0 && material < (int)materialTextures.size()) ? materialTextures[material] : std::string();
}

void GltfModel::resetPose(Pose &pose) const
{
    pose.t.resize(nodes.size() * 3); pose.r.resize(nodes.size() * 4); pose.s.resize(nodes.size() * 3);
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        for (int k = 0; k < 3; ++k) { pose.t[i * 3 + k] = nodes[i].t[k]; pose.s[i * 3 + k] = nodes[i].s[k]; }
        for (int k = 0; k < 4; ++k) pose.r[i * 4 + k] = nodes[i].r[k];
    }
}

void GltfModel::applyClip(Pose &pose, int clipIndex, float time) const
{
    if (clipIndex < 0 || clipIndex >= (int)clips.size()) return;
    const Clip &clip = clips[clipIndex];
    for (size_t ci = 0; ci < clip.channels.size(); ++ci)
    {
        const Channel &ch = clip.channels[ci];
        size_t keys = ch.times.size();
        if (keys == 0) continue;
        int width = (ch.path == 1) ? 4 : 3;
        float value[4] = {0, 0, 0, 1};

        if (keys == 1 || time <= ch.times[0])
        {
            for (int c = 0; c < width; ++c) value[c] = ch.values[c];
        }
        else if (time >= ch.times[keys - 1])
        {
            for (int c = 0; c < width; ++c) value[c] = ch.values[(keys - 1) * (size_t)width + (size_t)c];
        }
        else
        {
            size_t k = 0;
            while (k + 2 < keys && ch.times[k + 1] <= time) ++k;     // times[k] <= time < times[k+1]
            const float *a = &ch.values[k * (size_t)width];
            const float *b = &ch.values[(k + 1) * (size_t)width];
            float span = ch.times[k + 1] - ch.times[k];
            float f = span > 0.0f ? (time - ch.times[k]) / span : 0.0f;
            if (ch.step) f = 0.0f;
            if (ch.path == 1)
            {
                float dot = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
                float bb[4] = {b[0], b[1], b[2], b[3]};
                if (dot < 0.0f) { dot = -dot; for (int c = 0; c < 4; ++c) bb[c] = -bb[c]; }
                float wa, wb;
                if (dot > 0.9995f) { wa = 1.0f - f; wb = f; }
                else
                {
                    float theta = acosf(dot), sinTheta = sinf(theta);
                    wa = sinf((1.0f - f) * theta) / sinTheta;
                    wb = sinf(f * theta) / sinTheta;
                }
                for (int c = 0; c < 4; ++c) value[c] = a[c] * wa + bb[c] * wb;
                float n = sqrtf(value[0] * value[0] + value[1] * value[1] + value[2] * value[2] + value[3] * value[3]);
                if (n > 0.0f) for (int c = 0; c < 4; ++c) value[c] /= n;
            }
            else
            {
                for (int c = 0; c < 3; ++c) value[c] = a[c] + (b[c] - a[c]) * f;
            }
        }

        size_t n = (size_t)ch.node;
        if (ch.path == 0)      { for (int c = 0; c < 3; ++c) pose.t[n * 3 + c] = value[c]; }
        else if (ch.path == 1) { for (int c = 0; c < 4; ++c) pose.r[n * 4 + c] = value[c]; }
        else                   { for (int c = 0; c < 3; ++c) pose.s[n * 3 + c] = value[c]; }
    }
}

void GltfModel::computeWorld(const Pose &pose, std::vector<float> &world) const
{
    world.assign(nodes.size() * 16, 0.0f);
    std::vector<char> done(nodes.size(), 0);
    // Iterative: repeat until every node has its parent's matrix (node order in the file is not guaranteed parent-first).
    std::vector<int> stack(rootNodes.begin(), rootNodes.end());
    while (!stack.empty())
    {
        int i = stack.back(); stack.pop_back();
        if (i < 0 || i >= (int)nodes.size() || done[i]) continue;
        float local[16];
        float t[3] = {pose.t[(size_t)i * 3], pose.t[(size_t)i * 3 + 1], pose.t[(size_t)i * 3 + 2]};
        float s[3] = {pose.s[(size_t)i * 3], pose.s[(size_t)i * 3 + 1], pose.s[(size_t)i * 3 + 2]};
        float r[4] = {pose.r[(size_t)i * 4], pose.r[(size_t)i * 4 + 1], pose.r[(size_t)i * 4 + 2], pose.r[(size_t)i * 4 + 3]};
        quatToMatrix(r, s, t, local);
        int p = nodes[i].parent;
        if (p >= 0) multiply(&world[(size_t)p * 16], local, &world[(size_t)i * 16]);
        else memcpy(&world[(size_t)i * 16], local, sizeof(local));
        done[i] = 1;
        for (size_t k = 0; k < nodes[i].children.size(); ++k) stack.push_back(nodes[i].children[k]);
    }
}
