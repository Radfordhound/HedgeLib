#include "hedgelib/hl_scene.h"
#include "hedgelib/io/hl_file.h"

#ifdef HL_USE_FBX_SDK
#include <fbxsdk.h>
#endif

#include <robin_hood.h>

namespace hl
{
node_attribute::~node_attribute() {}

const node* node::find_child(const char* name, bool recursive) const
{
    for (const node* child : m_children)
    {
        if (child->name == name)
        {
            return child;
        }

        if (recursive)
        {
            const node* r = child->find_child(name, recursive);
            if (r) return r;
        }
    }

    return nullptr;
}

const node* node::find_child(const std::string& name, bool recursive) const
{
    // NOTE: Implemented separately to take advantage of different std::string == operator.

    for (const node* child : m_children)
    {
        if (child->name == name)
        {
            return child;
        }

        if (recursive)
        {
            const node* r = child->find_child(name, recursive);
            if (r) return r;
        }
    }

    return nullptr;
}

bool node::has_attributes_of_type(node_attribute_type type) const
{
    for (auto& attribute : attributes)
    {
        if (attribute->type() == type)
        {
            return true;
        }
    }

    return false;
}

node* scene::in_create_root_node()
{
    m_nodes.emplace_back(new node(*this, "RootNode"));
    return m_nodes.back().get();
}

const material* scene::find_material(const char* name) const
{
    for (auto& matPtr : m_materials)
    {
        if (matPtr->name == name)
        {
            return matPtr.get();
        }
    }

    return nullptr;
}

const material* scene::find_material(const std::string& name) const
{
    for (auto& matPtr : m_materials)
    {
        if (matPtr->name == name)
        {
            return matPtr.get();
        }
    }

    return nullptr;
}

material& scene::add_material(const char* name)
{
    std::unique_ptr<material> newMat(new material(*this, name));
    material* matPtr = newMat.get();
    m_materials.emplace_back(std::move(newMat));
    return *matPtr;
}

material& scene::add_material(const std::string& name)
{
    std::unique_ptr<material> newMat(new material(*this, name));
    material* matPtr = newMat.get();
    m_materials.emplace_back(std::move(newMat));
    return *matPtr;
}

material& scene::add_material(std::string&& name)
{
    std::unique_ptr<material> newMat(new material(*this, std::move(name)));
    material* matPtr = newMat.get();
    m_materials.emplace_back(std::move(newMat));
    return *matPtr;
}

const texture* scene::find_texture(const char* name) const
{
    for (auto& texPtr : m_textures)
    {
        if (texPtr->name == name)
        {
            return texPtr.get();
        }
    }

    return nullptr;
}

const texture* scene::find_texture(const std::string& name) const
{
    for (auto& texPtr : m_textures)
    {
        if (texPtr->name == name)
        {
            return texPtr.get();
        }
    }

    return nullptr;
}

texture& scene::add_texture(const char* name, const char* utf8FilePath)
{
    std::unique_ptr<texture> newTex(new texture(name, utf8FilePath));
    texture* texPtr = newTex.get();
    m_textures.emplace_back(std::move(newTex));
    return *texPtr;
}

texture& scene::add_texture(const std::string& name, const char* utf8FilePath)
{
    std::unique_ptr<texture> newTex(new texture(name, utf8FilePath));
    texture* texPtr = newTex.get();
    m_textures.emplace_back(std::move(newTex));
    return *texPtr;
}

texture& scene::add_texture(std::string&& name, const char* utf8FilePath)
{
    std::unique_ptr<texture> newTex(new texture(std::move(name), utf8FilePath));
    texture* texPtr = newTex.get();
    m_textures.emplace_back(std::move(newTex));
    return *texPtr;
}

texture& scene::add_texture(const char* name, const std::string& utf8FilePath)
{
    std::unique_ptr<texture> newTex(new texture(name, utf8FilePath));
    texture* texPtr = newTex.get();
    m_textures.emplace_back(std::move(newTex));
    return *texPtr;
}

texture& scene::add_texture(const std::string& name, const std::string& utf8FilePath)
{
    std::unique_ptr<texture> newTex(new texture(name, utf8FilePath));
    texture* texPtr = newTex.get();
    m_textures.emplace_back(std::move(newTex));
    return *texPtr;
}

texture& scene::add_texture(std::string&& name, const std::string& utf8FilePath)
{
    std::unique_ptr<texture> newTex(new texture(std::move(name), utf8FilePath));
    texture* texPtr = newTex.get();
    m_textures.emplace_back(std::move(newTex));
    return *texPtr;
}

texture& scene::add_texture(const char* name, std::string&& utf8FilePath)
{
    std::unique_ptr<texture> newTex(new texture(name, std::move(utf8FilePath)));
    texture* texPtr = newTex.get();
    m_textures.emplace_back(std::move(newTex));
    return *texPtr;
}

texture& scene::add_texture(const std::string& name, std::string&& utf8FilePath)
{
    std::unique_ptr<texture> newTex(new texture(name, std::move(utf8FilePath)));
    texture* texPtr = newTex.get();
    m_textures.emplace_back(std::move(newTex));
    return *texPtr;
}

texture& scene::add_texture(std::string&& name, std::string&& utf8FilePath)
{
    std::unique_ptr<texture> newTex(new texture(
        std::move(name), std::move(utf8FilePath)));

    texture* texPtr = newTex.get();
    m_textures.emplace_back(std::move(newTex));
    return *texPtr;
}

#ifdef HL_USE_FBX_SDK
/**
 * @brief Smart pointer class that automatically creates and destroys FBX SDK objects.
*/
template<typename T>
class in_fbx_unique_ptr
{
    T* m_ptr = nullptr;

public:
    using pointer = T*;
    using element_type = T;

    inline const T* get() const noexcept
    {
        return m_ptr;
    }

    inline T* get() noexcept
    {
        return m_ptr;
    }

    T* release() noexcept
    {
        T* ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

    void reset(pointer ptr = pointer())
    {
        if (m_ptr)
        {
            m_ptr->Destroy();
        }

        m_ptr = ptr;
    }

    void swap(in_fbx_unique_ptr<T>& other) noexcept
    {
        std::swap(m_ptr, other.m_ptr);
    }

    in_fbx_unique_ptr<T>& operator=(T* ptr)
    {
        reset(ptr);
        return *this;
    }

    in_fbx_unique_ptr<T>& operator=(std::nullptr_t)
    {
        reset(nullptr);
        return *this;
    }

    in_fbx_unique_ptr<T>& operator=(const in_fbx_unique_ptr<T>& other) = delete;

    in_fbx_unique_ptr<T>& operator=(in_fbx_unique_ptr<T>&& other)
    {
        reset(other.m_ptr);
        other.m_ptr = nullptr;
        return *this;
    }

    inline const T* operator->() const noexcept
    {
        return m_ptr;
    }

    inline T* operator->() noexcept
    {
        return m_ptr;
    }

    inline const T& operator*() const noexcept
    {
        return *m_ptr;
    }

    inline T& operator*() noexcept
    {
        return *m_ptr;
    }

    inline bool operator<(const in_fbx_unique_ptr<T>& other) const noexcept
    {
        return (m_ptr < other.m_ptr);
    }

    inline bool operator>(const in_fbx_unique_ptr<T>& other) const noexcept
    {
        return (m_ptr > other.m_ptr);
    }

    inline bool operator<=(const in_fbx_unique_ptr<T>& other) const noexcept
    {
        return (m_ptr <= other.m_ptr);
    }

    inline bool operator>=(const in_fbx_unique_ptr<T>& other) const noexcept
    {
        return (m_ptr >= other.m_ptr);
    }

    inline bool operator==(const in_fbx_unique_ptr<T>& other) const noexcept
    {
        return (m_ptr == other.m_ptr);
    }

    inline bool operator!=(const in_fbx_unique_ptr<T>& other) const noexcept
    {
        return (m_ptr != other.m_ptr);
    }

    inline bool operator<(const T* ptr) const noexcept
    {
        return (m_ptr < ptr);
    }

    inline bool operator>(const T* ptr) const noexcept
    {
        return (m_ptr > ptr);
    }

    inline bool operator<=(const T* ptr) const noexcept
    {
        return (m_ptr <= ptr);
    }

    inline bool operator>=(const T* ptr) const noexcept
    {
        return (m_ptr >= ptr);
    }

    inline bool operator==(const T* ptr) const noexcept
    {
        return (m_ptr == ptr);
    }

    inline bool operator!=(const T* ptr) const noexcept
    {
        return (m_ptr != ptr);
    }

    inline explicit operator bool() const noexcept
    {
        return (m_ptr != nullptr);
    }

    in_fbx_unique_ptr() noexcept : m_ptr(nullptr) {}

    template<class... args_t>
    in_fbx_unique_ptr(args_t&&... args) :
        m_ptr(T::Create(std::forward<args_t>(args)...)) {}

    in_fbx_unique_ptr(std::nullptr_t) noexcept {}
    in_fbx_unique_ptr(T* ptr) noexcept : m_ptr(ptr) {}

    in_fbx_unique_ptr(const in_fbx_unique_ptr<T>& other) = delete;

    in_fbx_unique_ptr(in_fbx_unique_ptr<T>&& other) noexcept :
        m_ptr(other.m_ptr)
    {
        other.m_ptr = nullptr;
    }

    ~in_fbx_unique_ptr()
    {
        if (m_ptr)
        {
            m_ptr->Destroy();
        }
    }
};

static in_fbx_unique_ptr<FbxManager> in_fbx_manager = nullptr;
static int in_fbx_reader_id = -1;
static int in_fbx_binary_writer_id = -1;
static int in_fbx_ascii_writer_id = -1;

static void in_fbx_initialize_manager()
{
    // Initialize manager.
    in_fbx_manager = FbxManager::Create();
    if (!in_fbx_manager)
    {
        throw std::runtime_error("Failed to create FBX manager");
    }

    // Create and setup IOSettings.
    in_fbx_manager->SetIOSettings(FbxIOSettings::Create(
        in_fbx_manager.get(), IOSROOT));

    // Get FBX reader/writer ids.
    FbxIOPluginRegistry* fbxPluginReg = in_fbx_manager->GetIOPluginRegistry();
    in_fbx_reader_id = fbxPluginReg->FindReaderIDByDescription("FBX (*.fbx)");
    in_fbx_binary_writer_id = fbxPluginReg->FindWriterIDByDescription("FBX binary (*.fbx)");
    in_fbx_ascii_writer_id = fbxPluginReg->FindWriterIDByDescription("FBX ascii (*.fbx)");
}

static seek_mode in_fbx_get_seek_mode(const FbxFile::ESeekPos seekPos) noexcept
{
    switch (seekPos)
    {
    default:
    case FbxFile::eBegin:
        return seek_mode::beg;

    case FbxFile::eCurrent:
        return seek_mode::cur;

    case FbxFile::eEnd:
        return seek_mode::end;
    }
}

class in_fbx_stream_wrapper : public FbxStream
{
    stream* m_stream = nullptr;
    bool isOpen = false;

public:
    EState GetState() override
    {
        return (isOpen) ? eOpen : eClosed;
    }

    bool Open(void* pStreamData) override
    {
        if (!isOpen)
        {
            isOpen = true;
        }
        else
        {
            m_stream->jump_to(0);
        }

        return true;
    }

    bool Close() override
    {
        if (isOpen)
        {
            isOpen = false;
        }
        
        return true;
    }

    bool Flush() override
    {
        if (!isOpen) return false;

        m_stream->flush();
        return true;
    }

    int Write(const void* pData, int pSize) override
    {
        return (!isOpen) ? 0 : static_cast<int>(m_stream->write(pSize, pData));
    }

    int Read(void* pData, int pSize) const override
    {
        return (!isOpen) ? 0 : static_cast<int>(m_stream->read(pSize, pData));
    }

    int GetReaderID() const override
    {
        return -1;
    }

    int GetWriterID() const override
    {
        return -1;
    }

    void Seek(const FbxInt64& pOffset, const FbxFile::ESeekPos& pSeekPos) override
    {
        if (!isOpen) return;

        m_stream->seek(in_fbx_get_seek_mode(pSeekPos), pOffset);
    }

    long GetPosition() const override
    {
        return (!isOpen) ? 0 : static_cast<long>(m_stream->tell());
    }

    void SetPosition(long pPosition) override
    {
        if (!isOpen) return;
        
        m_stream->jump_to(pPosition);
    }

    int GetError() const override
    {
        // TODO: Should we catch exceptions, set a "didError" boolean flag, and return the value of that flag from here instead?
        return 0;
    }

    void ClearError() override
    {
        // TODO: Should we reset the aforementioned "didError" boolean flag here instead?
    }

    in_fbx_stream_wrapper(stream& stream) noexcept :
        m_stream(&stream) {}
};

static node& in_fbx_import_mesh(const char* nodeName,
    const FbxMesh& fbxMesh, node& parentNode)
{
    // Create node.
    node& meshNode = parentNode.add_child(nodeName);
    
    // Create mesh attribute and import data if necessary.
    if (fbxMesh.GetControlPointsCount())
    {
        // TODO: Split up mesh into separate meshes based on materials used.

        // Create mesh attribute.
        std::unique_ptr<mesh> meshAttr(new mesh(fbxMesh.GetName()));

        // Copy vertices.
        const int fbxControlPointCount = fbxMesh.GetControlPointsCount();
        const FbxVector4* fbxControlPoints = fbxMesh.GetControlPoints();

        meshAttr->vertices.reserve(fbxControlPointCount);

        for (int i = 0; i < fbxControlPointCount; ++i)
        {
            const FbxVector4& fbxControlPoint = fbxControlPoints[i];
            meshAttr->vertices.emplace_back(
                static_cast<float>(fbxControlPoint.mData[0]),
                static_cast<float>(fbxControlPoint.mData[1]),
                static_cast<float>(fbxControlPoint.mData[2]),
                static_cast<float>(fbxControlPoint.mData[3]));
        }

        // TODO

        // Add mesh attribute to node.
        meshNode.attributes.emplace_back(std::move(meshAttr));
    }

    return meshNode;
}

static node* in_fbx_import_node(const FbxNode& fbxNode, node& parentNode)
{
    const FbxNodeAttribute* fbxNodeAttr = fbxNode.GetNodeAttribute();
    if (!fbxNodeAttr) return nullptr;

    switch (fbxNodeAttr->GetAttributeType())
    {
    default:
    case FbxNodeAttribute::eUnknown:
    case FbxNodeAttribute::eMarker:
        return nullptr;

    case FbxNodeAttribute::eNull:
        // TODO: Create an object instead and import transform if necessary.
        return &parentNode.add_child(fbxNode.GetName());

    // TODO
    //case FbxNodeAttribute::eSkeleton:

    case FbxNodeAttribute::eMesh:
        return &in_fbx_import_mesh(fbxNode.GetName(),
            *static_cast<const FbxMesh*>(fbxNodeAttr),
            parentNode);

    // TODO
    /*case FbxNodeAttribute::eNurbs:
    case FbxNodeAttribute::ePatch:
    case FbxNodeAttribute::eCamera:
    case FbxNodeAttribute::eCameraStereo:
    case FbxNodeAttribute::eCameraSwitcher:
    case FbxNodeAttribute::eLight:
    case FbxNodeAttribute::eOpticalReference:
    case FbxNodeAttribute::eOpticalMarker:
    case FbxNodeAttribute::eNurbsCurve:
    case FbxNodeAttribute::eTrimNurbsSurface:
    case FbxNodeAttribute::eBoundary:
    case FbxNodeAttribute::eNurbsSurface:
    case FbxNodeAttribute::eShape:
    case FbxNodeAttribute::eLODGroup:
    case FbxNodeAttribute::eSubDiv:
    case FbxNodeAttribute::eCachedEffect:
    case FbxNodeAttribute::eLine:*/
    }
}

static void in_fbx_import_children(const FbxNode& fbxParentNode, node& parentNode)
{
    for (int i = 0; i < fbxParentNode.GetChildCount(); ++i)
    {
        // Import child node.
        const FbxNode& fbxChildNode = *fbxParentNode.GetChild(i);
        node* childNode = in_fbx_import_node(fbxChildNode, parentNode);

        if (!childNode) continue;

        // Recurse through children.
        in_fbx_import_children(fbxChildNode, *childNode);
    }
}

static void in_fbx_import(FbxImporter& fbxImporter, scene& scene)
{
    // Create FBX scene.
    in_fbx_unique_ptr<FbxScene> fbxScene(in_fbx_manager.get(), "Imported FBX Scene");
    if (!fbxScene)
    {
        throw std::runtime_error("Failed to create FBX scene");
    }

    // Import scene from .fbx file.
    if (!fbxImporter.Import(fbxScene.get()))
    {
        throw std::runtime_error(fbxImporter.GetStatus().GetErrorString());
    }

    in_fbx_import_children(*fbxScene->GetRootNode(), scene.root_node());
}

void scene::import_fbx(stream& stream)
{
    // Initialize FBX manager if necessary.
    if (!in_fbx_manager)
    {
        in_fbx_initialize_manager();
    }

    // Create FBX importer.
    in_fbx_stream_wrapper fbxStream(stream);
    in_fbx_unique_ptr<FbxImporter> fbxImporter(in_fbx_manager.get(), "");

    if (!fbxImporter->Initialize(&fbxStream, nullptr,
        in_fbx_reader_id, in_fbx_manager->GetIOSettings()))
    {
        throw std::runtime_error(fbxImporter->GetStatus().GetErrorString());
    }
    
    // Import scene from .fbx file.
    in_fbx_import(*fbxImporter.get(), *this);
}

void scene::import_fbx(const nchar* filePath) // TODO: Just call the stream& variant unless this is actually slower
{
    // Initialize FBX manager if necessary.
    if (!in_fbx_manager)
    {
        in_fbx_initialize_manager();
    }

    // Get file path as UTF-8 string.
#ifdef HL_IN_WIN32_UNICODE
    std::string filePathUTF8 = text::conv<text::native_to_utf8>(filePath);
    const char* filePathUTF8Ptr = filePathUTF8.c_str();
#else
    const char* filePathUTF8Ptr = filePath;
#endif

    // Create FBX importer.
    in_fbx_unique_ptr<FbxImporter> fbxImporter(in_fbx_manager.get(), "");

    if (!fbxImporter->Initialize(filePathUTF8Ptr,
        in_fbx_reader_id, in_fbx_manager->GetIOSettings()))
    {
        throw std::runtime_error(fbxImporter->GetStatus().GetErrorString());
    }

    // Import scene from .fbx file.
    in_fbx_import(*fbxImporter.get(), *this);
}

static FbxTexture* in_fbx_add_texture(const texture& tex,
    robin_hood::unordered_map<const texture*, FbxTexture*>& fbxTextures)
{
    in_fbx_unique_ptr<FbxFileTexture> newFbxTex(
        in_fbx_manager.get(), tex.name.c_str());

    newFbxTex->SetFileName(tex.utf8FilePath.c_str());
    newFbxTex->SetTextureUse(FbxTexture::eStandard);
    newFbxTex->SetMappingType(FbxTexture::eUV);
    newFbxTex->SetMaterialUse(FbxFileTexture::eModelMaterial);
    newFbxTex->SetSwapUV(false);
    newFbxTex->SetTranslation(0.0, 0.0);
    newFbxTex->SetScale(1.0, 1.0);
    newFbxTex->SetRotation(0.0, 0.0);
    newFbxTex->UVSet.Set("DiffuseUV");

    // TODO

    fbxTextures.emplace(&tex, newFbxTex.get());
    return newFbxTex.release();
}

static FbxTexture* in_fbx_get_or_add_texture(const texture& tex,
    robin_hood::unordered_map<const texture*, FbxTexture*>& fbxTextures)
{
    auto it = fbxTextures.find(&tex);
    return (it != fbxTextures.end()) ? it->second :
        in_fbx_add_texture(tex, fbxTextures);
}

static FbxSurfaceMaterial* in_fbx_add_material(const material& mat,
    robin_hood::unordered_map<const material*, FbxSurfaceMaterial*>& fbxMats,
    robin_hood::unordered_map<const texture*, FbxTexture*>& fbxTextures)
{
    in_fbx_unique_ptr<FbxSurfacePhong> newFbxMat(
        in_fbx_manager.get(), mat.name.c_str());

    newFbxMat->ShadingModel.Set("Phong");

    newFbxMat->Diffuse.Set(FbxDouble3(mat.diffuseColor.x,
        mat.diffuseColor.y, mat.diffuseColor.z));

    // TODO

    // Connect diffuse texture, if any.
    if (mat.diffuseTex)
    {
        newFbxMat->Diffuse.ConnectSrcObject(in_fbx_get_or_add_texture(
            *mat.diffuseTex, fbxTextures));
    }

    // Connect specular texture, if any.
    if (mat.specularTex)
    {
        newFbxMat->Specular.ConnectSrcObject(in_fbx_get_or_add_texture(
            *mat.specularTex, fbxTextures));
    }

    // Connect ambient texture, if any.
    if (mat.ambientTex)
    {
        newFbxMat->Ambient.ConnectSrcObject(in_fbx_get_or_add_texture(
            *mat.ambientTex, fbxTextures));
    }

    // Connect normal texture, if any.
    if (mat.normalTex)
    {
        newFbxMat->NormalMap.ConnectSrcObject(in_fbx_get_or_add_texture(
            *mat.normalTex, fbxTextures));
    }

    // Connect reflection texture, if any.
    if (mat.reflectionTex)
    {
        newFbxMat->Reflection.ConnectSrcObject(in_fbx_get_or_add_texture(
            *mat.reflectionTex, fbxTextures));
    }

    // TODO: Connect other textures.

    fbxMats.emplace(&mat, newFbxMat.get());
    return newFbxMat.release();
}

static FbxSurfaceMaterial* in_fbx_get_or_add_material(const material& mat,
    robin_hood::unordered_map<const material*, FbxSurfaceMaterial*>& fbxMats,
    robin_hood::unordered_map<const texture*, FbxTexture*>& fbxTextures)
{
    auto it = fbxMats.find(&mat);

    return (it != fbxMats.end()) ? it->second :
        in_fbx_add_material(mat, fbxMats, fbxTextures);
}

template<typename T>
T* in_fbx_add_layer_element(FbxMesh& fbxMesh,
    FbxLayerElement::EMappingMode mappingMode = FbxLayerElement::eByControlPoint,
    FbxLayerElement::EReferenceMode refMode = FbxLayerElement::eDirect)
{
    T* fbxLayerElem = T::Create(&fbxMesh, "");
    fbxLayerElem->SetMappingMode(mappingMode);
    fbxLayerElem->SetReferenceMode(refMode);
    return fbxLayerElem;
}

static void in_fbx_export_mesh(const node& node, FbxNode& fbxNode,
    robin_hood::unordered_map<const material*, FbxSurfaceMaterial*>& fbxMats,
    robin_hood::unordered_map<const texture*, FbxTexture*>& fbxTextures)
{
    // Create FBX mesh.
    in_fbx_unique_ptr<FbxMesh> fbxMesh(in_fbx_manager.get(), node.name.c_str());

    // Compute total vertex count.
    int totalVertexCount = 0;
    for (auto& attributePtr : node.attributes)
    {
        // Skip attributes which are not meshes.
        if (attributePtr->type() != node_attribute_type::mesh)
            continue;

        // Add to total count.
        const hl::mesh& mesh = static_cast<const hl::mesh&>(*attributePtr);
        totalVertexCount += static_cast<int>(mesh.vertices.size());
    }

    // Setup FBX control points.
    fbxMesh->InitControlPoints(totalVertexCount);

    // Setup FBX layer.
    int fbxLayerIndex = fbxMesh->CreateLayer();
    if (fbxLayerIndex == -1)
    {
        throw std::runtime_error("Failed to create FBX layer");
    }

    FbxLayer* fbxLayer = fbxMesh->GetLayer(fbxLayerIndex);

    // Setup FBX layer elements.
    auto fbxElemNormal = in_fbx_add_layer_element<FbxLayerElementNormal>(*fbxMesh);
    fbxLayer->SetNormals(fbxElemNormal);

    auto fbxElemBinormal = in_fbx_add_layer_element<FbxLayerElementBinormal>(*fbxMesh);
    fbxLayer->SetBinormals(fbxElemBinormal);

    auto fbxElemTangent = in_fbx_add_layer_element<FbxLayerElementTangent>(*fbxMesh);
    fbxLayer->SetTangents(fbxElemTangent);

    auto fbxElemUV0 = in_fbx_add_layer_element<FbxLayerElementUV>(*fbxMesh);
    fbxLayer->SetUVs(fbxElemUV0, FbxLayerElement::eTextureDiffuse);

    auto fbxElemVtxColor = in_fbx_add_layer_element<FbxLayerElementVertexColor>(*fbxMesh);
    fbxLayer->SetVertexColors(fbxElemVtxColor);

    // Setup FBX layer element direct arrays.
    FbxVector4* fbxControlPoints = fbxMesh->GetControlPoints();
    FbxLayerElementArrayTemplate<FbxVector4>& fbxNormals = fbxElemNormal->GetDirectArray();
    FbxLayerElementArrayTemplate<FbxVector4>& fbxBinormals = fbxElemBinormal->GetDirectArray();
    FbxLayerElementArrayTemplate<FbxVector4>& fbxTangents = fbxElemTangent->GetDirectArray();
    FbxLayerElementArrayTemplate<FbxVector2>& fbxUV0 = fbxElemUV0->GetDirectArray();
    FbxLayerElementArrayTemplate<FbxColor>& fbxColors = fbxElemVtxColor->GetDirectArray();

    // Convert mesh data and add to FBX mesh.
    std::size_t curFbxVtxIndex = 0;
    for (auto& attributePtr : node.attributes)
    {
        // Skip attributes which are not meshes.
        if (attributePtr->type() != node_attribute_type::mesh)
            continue;

        // Generate FBX control points.
        const hl::mesh& mesh = static_cast<const hl::mesh&>(*attributePtr);
        for (std::size_t i = 0; i < mesh.vertices.size(); ++i)
        {
            const vec4& curVtx = mesh.vertices[i];
            fbxControlPoints[curFbxVtxIndex + i] = FbxVector4(
                curVtx.x, curVtx.y, curVtx.z, curVtx.w);
        }

        // Generate FBX normals.
        for (std::size_t i = 0; i < mesh.normals.size(); ++i)
        {
            const vec4& curVal = mesh.normals[i];
            fbxNormals.Add(FbxVector4(curVal.x, curVal.y, curVal.z, curVal.w));
        }

        // Generate FBX binormals.
        for (std::size_t i = 0; i < mesh.binormals.size(); ++i)
        {
            const vec4& curVal = mesh.binormals[i];
            fbxBinormals.Add(FbxVector4(curVal.x, curVal.y, curVal.z, curVal.w));
        }

        // Generate FBX tangents.
        for (std::size_t i = 0; i < mesh.tangents.size(); ++i)
        {
            const vec4& curVal = mesh.tangents[i];
            fbxTangents.Add(FbxVector4(curVal.x, curVal.y, curVal.z, curVal.w));
        }

        // Generate UV coordinates.
        for (std::size_t i = 0; i < mesh.uvs[0].size(); ++i)
        {
            const vec2& curVal = mesh.uvs[0][i];
            fbxUV0.Add(FbxVector2(curVal.x, 1.0 - curVal.y));
        }

        // TODO: Other UV channels.

        // Generate FBX vertex colors.
        for (std::size_t i = 0; i < mesh.colors.size(); ++i)
        {
            const vec4& curVal = mesh.colors[i];
            fbxColors.Add(FbxColor(curVal.x, curVal.y, curVal.z, curVal.w));
        }

        // Ensure face count is a multiple of 3.
        if ((mesh.faces.size() % 3U) != 0)
        {
            throw std::runtime_error("Invalid data; "
                "mesh face count must be a multiple of 3");
        }

        // Generate FBX polygons.
        for (std::size_t i = 0; i < mesh.faces.size(); i += 3)
        {
            fbxMesh->BeginPolygon(-1, -1, -1, false);

            // TODO: We treat the faces as counter-clockwise here. Should we always do this?
            fbxMesh->AddPolygon(static_cast<int>(curFbxVtxIndex + mesh.faces[i + 2]));
            fbxMesh->AddPolygon(static_cast<int>(curFbxVtxIndex + mesh.faces[i + 1]));
            fbxMesh->AddPolygon(static_cast<int>(curFbxVtxIndex + mesh.faces[i]));

            fbxMesh->EndPolygon();
        }

        curFbxVtxIndex += mesh.vertices.size();
    }

    // Assign materials to mesh.
    // NOTE: It seems we have to do this here *AFTER* the above loop has fully completed
    // or we run into issues where the FBX SDK screws up the material indices.
    auto fbxElemMaterial = in_fbx_add_layer_element<FbxLayerElementMaterial>(*fbxMesh,
        FbxLayerElement::eByPolygon, FbxLayerElement::eIndexToDirect);

    fbxLayer->SetMaterials(fbxElemMaterial);

    FbxLayerElementArrayTemplate<int>& fbxMatIndices = fbxElemMaterial->GetIndexArray();

    for (auto& attributePtr : node.attributes)
    {
        // Skip attributes which are not meshes.
        if (attributePtr->type() != node_attribute_type::mesh)
            continue;

        // Setup material if this mesh has a material assigned to it.
        const hl::mesh& mesh = static_cast<const hl::mesh&>(*attributePtr);
        if (mesh.material)
        {
            // Get FBX material from node, or from FBX material tree (creating it if necessary).
            int fbxMatIndex = fbxNode.GetMaterialIndex(mesh.material->name.c_str());
            if (fbxMatIndex == -1)
            {
                FbxSurfaceMaterial* fbxMat = in_fbx_get_or_add_material(
                    *mesh.material, fbxMats, fbxTextures);

                fbxMatIndex = fbxNode.AddMaterial(fbxMat);

                if (fbxMatIndex == -1)
                {
                    throw std::runtime_error("Failed to add material to FBX mesh");
                }
            }

            // Add material indices to mesh.
            for (std::size_t i = 0; i < mesh.faces.size(); i += 3)
            {
                fbxMatIndices.Add(fbxMatIndex);
            }
        }
    }

    fbxNode.SetShadingMode(FbxNode::eFullShading);
    fbxNode.SetNodeAttribute(fbxMesh.release());
}

static FbxSkeleton* in_fbx_export_bone(const bone& bone, bool& addedRootBone)
{
    // Generate FBX skeleton.
    in_fbx_unique_ptr<FbxSkeleton> fbxSkeleton(in_fbx_manager.get(), bone.name.c_str());

    // Set FBX skeleton type and update the value of the addedRootBone flag.
    // NOTE: addedRootBone is passed by-reference; its value persists between
    // multiple in_fbx_export_bone calls.
    fbxSkeleton->SetSkeletonType((addedRootBone) ?
        FbxSkeleton::eLimbNode : FbxSkeleton::eRoot);

    addedRootBone = true;

    return fbxSkeleton.release();
}

static FbxNode* in_fbx_export_node(const node& node, bool& addedRootBone,
    robin_hood::unordered_map<const material*, FbxSurfaceMaterial*>& fbxMats,
    robin_hood::unordered_map<const texture*, FbxTexture*>& fbxTextures)
{
    // Create FBX Node.
    in_fbx_unique_ptr<FbxNode> fbxNode(in_fbx_manager.get(), node.name.c_str());

    // Set local translation.
    fbxNode->LclTranslation.Set(FbxDouble3(node.pos.x, node.pos.y, node.pos.z));

    // Set local rotation.
    FbxVector4 fbxRot;
    fbxRot.SetXYZ(FbxQuaternion(node.rot.x, node.rot.y, node.rot.z, node.rot.w));
    fbxNode->LclRotation.Set(fbxRot);

    // Set local scaling.
    fbxNode->LclScaling.Set(FbxDouble3(node.scale.x, node.scale.y, node.scale.z));

    // Convert special node types.
    switch (node.type())
    {
    case node_type::bone:
        fbxNode->SetNodeAttribute(in_fbx_export_bone(static_cast<
            const bone&>(node), addedRootBone));
        break;
    }

    // Convert attributes.
    if (node.has_attributes_of_type(node_attribute_type::mesh))
    {
        in_fbx_export_mesh(node, *fbxNode, fbxMats, fbxTextures);
    }

    return fbxNode.release();
}

static void in_fbx_export_children(const node& parentNode, FbxNode& fbxParentNode,
    robin_hood::unordered_map<const material*, FbxSurfaceMaterial*>& fbxMats,
    robin_hood::unordered_map<const texture*, FbxTexture*>& fbxTextures,
    bool addedRootBone = false)
{
    for (auto& childNodePtr : parentNode.children())
    {
        // Generate FBX child node.
        const node& childNode = *childNodePtr;
        in_fbx_unique_ptr<FbxNode> fbxChildNode(in_fbx_export_node(
            childNode, addedRootBone, fbxMats, fbxTextures));

        if (!fbxChildNode) continue;

        // Recurse through children.
        in_fbx_export_children(childNode, *fbxChildNode,
            fbxMats, fbxTextures, addedRootBone);

        // Add generated FBX child node to FBX scene.
        if (!fbxParentNode.AddChild(fbxChildNode.get()))
        {
            throw std::runtime_error("Could not add child node to FBX scene");
        }

        fbxChildNode.release();
    }
}

struct in_fbx_bone_data
{
    in_fbx_unique_ptr<FbxCluster> fbxCluster;
    FbxNode* fbxBonePtr = nullptr;

    void set_transform(const FbxAMatrix& fbxAffineMtx)
    {
        fbxCluster->SetTransformMatrix(fbxAffineMtx);
    }

    void link_to_fbx_bone(FbxNode& fbxBone)
    {
        fbxBonePtr = &fbxBone;
        fbxCluster->SetLink(&fbxBone);
        fbxCluster->SetLinkMode(FbxCluster::eTotalOne);
        fbxCluster->SetTransformLinkMatrix(fbxBone.EvaluateGlobalTransform());
    }

    in_fbx_bone_data(FbxManager* fbxMgr) :
        fbxCluster(fbxMgr, "") {}
};

static void in_fbx_link_mesh_to_bones(FbxScene& fbxScene,
    const node& node, int fbxNodeIndex)
{
    // Generate FBX bone data.
    robin_hood::unordered_map<const bone*, in_fbx_bone_data> fbxBoneData;
    FbxNode* fbxMeshNode = fbxScene.GetNode(fbxNodeIndex);
    const FbxAMatrix& fbxMeshMtx = fbxMeshNode->EvaluateGlobalTransform();
    std::size_t curFbxVtxIndex = 0;

    for (auto& attributePtr : node.attributes)
    {
        // Skip non-mesh node attributes.
        if (attributePtr->type() != node_attribute_type::mesh)
            continue;

        // Ensure we have at least one bone to link.
        const hl::mesh& mesh = static_cast<const hl::mesh&>(*attributePtr);
        if (mesh.boneRefs.empty()) return;

        // Ensure we have the same amount of bone references and bone weights.
        if (mesh.boneRefs.size() != mesh.boneWeights.size())
        {
            throw std::runtime_error("Invalid data; "
                "the amount of bone references and bone weights must be equal");
        }

        // Generate FBX bone data.
        for (std::size_t i = 0; i < mesh.boneRefs.size(); ++i)
        {
            const bone_ref& boneRefs = mesh.boneRefs[i];
            const vec4& boneWeight = mesh.boneWeights[i];

            for (int i2 = 0; i2 < 4; ++i2)
            {
                if (!boneRefs[i2]) continue;

                // Find existing FBX bone data for this bone, or add new bone data if none was found.
                auto p = fbxBoneData.emplace(boneRefs[i2], in_fbx_manager.get());
                
                if (p.second) // If a new FBX bone data was just added...
                {
                    // Set the newly-added FBX bone data's transform.
                    p.first->second.set_transform(fbxMeshMtx);

                    // Link the newly-added FBX bone data to the corresponding FBX bone.
                    FbxNode* fbxBoneNode = fbxScene.FindNodeByName(FbxString(
                        boneRefs[i2]->name.c_str(), boneRefs[i2]->name.size()));

                    if (!fbxBoneNode)
                    {
                        // TODO: Log warning that we couldn't find a referenced bone with the given name.
                        continue;
                    }

                    p.first->second.link_to_fbx_bone(*fbxBoneNode);
                }

                // Add bone weight to FBX cluster.
                p.first->second.fbxCluster->AddControlPointIndex(
                    static_cast<int>(curFbxVtxIndex + i), boneWeight[i2]);
            }
        }

        curFbxVtxIndex += mesh.vertices.size();
    }

    // Generate FBX skin.
    in_fbx_unique_ptr<FbxSkin> fbxSkin(in_fbx_manager.get(), "");
    for (auto& p : fbxBoneData)
    {
        if (!fbxSkin->AddCluster(p.second.fbxCluster.get()))
        {
            throw std::runtime_error("Failed to add cluster to FBX skin");
        }

        // Apparently calling FbxSkin::Destroy also deletes the memory for
        // all of the fbx clusters that have been added to it; so we have to
        // release the unique pointer to the cluster now to avoid causing a segfault
        p.second.fbxCluster.release();
    }

    // Add FBX skin to FBX mesh.
    FbxMesh* fbxMesh = fbxMeshNode->GetMesh();
    fbxMesh->AddDeformer(fbxSkin.get());

    // Release the FBX skin pointer so it doesn't get
    // deleted, thus removing it from the final export.
    fbxSkin.release();
}

static void in_fbx_link_children(FbxScene& fbxScene, const node& node, int& fbxNodeIndex)
{
    for (auto& childNodePtr : node.children())
    {
        // Link FBX child node if necessary.
        const hl::node& childNode = *childNodePtr;
        ++fbxNodeIndex;

        if (childNode.has_attributes_of_type(node_attribute_type::mesh))
        {
            in_fbx_link_mesh_to_bones(fbxScene, childNode, fbxNodeIndex);
        }

        // Recurse through children.
        in_fbx_link_children(fbxScene, childNode, fbxNodeIndex);
    }
}

static void in_fbx_export(const scene& scene, FbxExporter& fbxExporter)
{
    // Create FBX scene.
    in_fbx_unique_ptr<FbxScene> fbxScene(in_fbx_manager.get(), scene.name.c_str());
    if (!fbxScene)
    {
        throw std::runtime_error("Failed to create FBX scene");
    }

    FbxGlobalSettings& fbxGlobalSettings = fbxScene->GetGlobalSettings();
    //fbxGlobalSettings.SetAxisSystem(FbxAxisSystem::Max);
    fbxGlobalSettings.SetSystemUnit(FbxSystemUnit::m);

    // Generate scene and export it to .fbx file.
    robin_hood::unordered_map<const material*, FbxSurfaceMaterial*> fbxMats;
    robin_hood::unordered_map<const texture*, FbxTexture*> fbxTextures;
    int fbxNodeIndex = 0;

    in_fbx_export_children(scene.root_node(), *fbxScene->GetRootNode(),
        fbxMats, fbxTextures);

    in_fbx_link_children(*fbxScene, scene.root_node(), fbxNodeIndex);

    if (!fbxExporter.Export(fbxScene.get()))
    {
        throw std::runtime_error(fbxExporter.GetStatus().GetErrorString());
    }
}

static const char* in_fbx_get_version_string(fbx_version fbxVersion)
{
    switch (fbxVersion)
    {
    default:
        return nullptr;

    case fbx_version::fbx_53_mb55:
        return FBX_53_MB55_COMPATIBLE;

    case fbx_version::fbx_60:
        return FBX_60_COMPATIBLE;

    case fbx_version::fbx_2005:
        return FBX_2005_08_COMPATIBLE;

    case fbx_version::fbx_2006_02:
        return FBX_2006_02_COMPATIBLE;

    case fbx_version::fbx_2006_08:
        return FBX_2006_08_COMPATIBLE;

    case fbx_version::fbx_2006_11:
        return FBX_2006_11_COMPATIBLE;

    case fbx_version::fbx_2009_00:
        return FBX_2009_00_COMPATIBLE;

    case fbx_version::fbx_2009_00_V7:
        return FBX_2009_00_V7_COMPATIBLE;

    case fbx_version::fbx_2010:
        return FBX_2010_00_COMPATIBLE;

    case fbx_version::fbx_2011:
        return FBX_2011_00_COMPATIBLE;

    case fbx_version::fbx_2012:
        return FBX_2012_00_COMPATIBLE;

    case fbx_version::fbx_2013:
        return FBX_2013_00_COMPATIBLE;

    case fbx_version::fbx_2014:
        return FBX_2014_00_COMPATIBLE;

    case fbx_version::fbx_2016:
        return FBX_2016_00_COMPATIBLE;

    case fbx_version::fbx_2018:
        return FBX_2018_00_COMPATIBLE;

    case fbx_version::fbx_2019:
        return FBX_2019_00_COMPATIBLE;

    case fbx_version::fbx_2020:
        return FBX_2020_00_COMPATIBLE;
    }
}

void scene::export_fbx(stream& stream, fbx_type type, fbx_version version) const
{
    // Initialize FBX manager if necessary.
    if (!in_fbx_manager)
    {
        in_fbx_initialize_manager();
    }

    // Create FBX exporter.
    in_fbx_stream_wrapper fbxStream(stream);
    in_fbx_unique_ptr<FbxExporter> fbxExporter(in_fbx_manager.get(), "");

    if (!fbxExporter->Initialize(&fbxStream, nullptr,
        (type == fbx_type::ascii) ? in_fbx_ascii_writer_id :
        in_fbx_binary_writer_id, in_fbx_manager->GetIOSettings()))
    {
        throw std::runtime_error(fbxExporter->GetStatus().GetErrorString());
    }

    fbxExporter->SetFileExportVersion(in_fbx_get_version_string(version));

    // Export scene to .fbx file.
    in_fbx_export(*this, *fbxExporter.get());
}

void scene::export_fbx(const nchar* filePath, fbx_type type, fbx_version version) const
{
    file_stream stream(filePath, file::mode::write);
    export_fbx(stream, type, version);
}
#endif
} // hl
