#include "simple_logger.h"

#include "gf3d_vgraphics.h"
#include "gf3d_swapchain.h"
#include "gf3d_buffers.h"
#include "gf3d_obj_load.h"
#include "gfc_list.h"
#include "gf3d_camera.h"
#include "gf3d_mesh.h"

#define MESH_ATTRIBUTE_COUNT 3

typedef struct
{
    Mesh                               *mesh_list;
    Uint32                              mesh_count;
    Uint32                              chain_length;
    VkDevice                            device;
    Pipeline                           *pipe;
    VkVertexInputAttributeDescription   attributeDescriptions[MESH_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription     bindingDescription;
    Texture                            *defaultTexture;
} MeshManager;

extern int __DEBUG;
static MeshManager mesh_manager = {0};

void gf3d_mesh_free();
void gf3d_mesh_destroy(Mesh* mesh);
void gf3d_mesh_primitive_destroy(MeshPrimitive* prim);
void gf3d_mesh_manager_close();
void gf3d_mesh_create_vertex_buffer_from_vertices(MeshPrimitive *prim);
void gf3d_mesh_create_face_buffer_from_vertices(MeshPrimitive *primitive);
void gf3d_mesh_queue_render(Mesh *mesh,Pipeline* pipe, MeshUBO* uboData,Texture *texture);

void gf3d_mesh_init(Uint32 mesh_max) {
    Uint32 count = 0;
    if (mesh_max == 0)
    {
        slog("cannot intilizat sprite manager for 0 sprites");
        return;
    }
    mesh_manager.chain_length = gf3d_swapchain_get_chain_length();
    mesh_manager.mesh_list = (Mesh *)gfc_allocate_array(sizeof(Mesh),mesh_max);
    mesh_manager.mesh_count = mesh_max;
    mesh_manager.device = gf3d_vgraphics_get_default_logical_device();

    gf3d_mesh_get_attribute_descriptions(&count);
    mesh_manager.pipe = gf3d_pipeline_create_from_config(
        gf3d_vgraphics_get_default_logical_device(),
        "config/model_pipeline.cfg",
        gf3d_vgraphics_get_view_extent(),
        mesh_max,
        gf3d_mesh_get_bind_description(),
        gf3d_mesh_get_attribute_descriptions(NULL),
        count,
        sizeof(MeshUBO),
        VK_INDEX_TYPE_UINT16
    );
    mesh_manager.defaultTexture = gf3d_texture_load("images/default.png");
    if(__DEBUG)slog("mesh manager initiliazed");
    atexit(gf3d_mesh_manager_close);
}

void gf3d_mesh_manager_close() {
    int i;
    for (i = 0; i < mesh_manager.mesh_count;i++)
    {
        if (mesh_manager.mesh_list[i]._refCount > 0) {
            gf3d_mesh_free(&mesh_manager.mesh_list[i]);
        }
    }
    if (mesh_manager.mesh_list)
    {
        free(mesh_manager.mesh_list);
    }
    gf3d_pipeline_free(mesh_manager.pipe);
    gf3d_texture_free(mesh_manager.defaultTexture);
    memset(&mesh_manager,0,sizeof(MeshManager));
    if(__DEBUG)slog("mesh manager closed");
}

Mesh *gf3d_mesh_new() {
    slog("creating new mesh");
    int i;
    for (i = 0; i < mesh_manager.mesh_count; i++)
    {
        if (mesh_manager.mesh_list[i]._refCount > 0)continue;
        memset(&mesh_manager.mesh_list[i], 0, sizeof(Mesh));
        mesh_manager.mesh_list[i]._refCount = 1;
        slog("created new mesh");
        return &mesh_manager.mesh_list[i];
    }
    slog("gf3d_mesh_new: no free slots for new meshes");
    return NULL;
}

Mesh *gf3d_mesh_load(const char *filename) {
    slog("loading mesh");
    Mesh* mesh;
    ObjData* data;
    MeshPrimitive* prim;
    if (!filename) return NULL;

    // Create ourselves a mesh
    mesh = gf3d_mesh_new();
    if (!mesh) return NULL;

    mesh->primitives = gfc_list_new_size(1);

    // Create and load a model obj data (.obj)
    data = gf3d_obj_load_from_file(filename);
    if (!data) {
        gf3d_mesh_free(mesh);
        return NULL;
    }

    slog("loaded obj data");

    // Create a mesh primitive which represents the obj data as a sub-mesh
    prim = gf3d_mesh_primitive_new();
    if (!prim) {
        gf3d_mesh_free(mesh);
        gf3d_obj_free(data);
        return NULL;
    }

    slog("created primitive");

    // Populate buffers with mesh data (faces & verts)
    gfc_list_append(mesh->primitives, prim);
    prim->objData = data;

    slog("attempting to create buffers");
    gf3d_mesh_create_vertex_buffer_from_vertices(prim);
    gf3d_mesh_create_face_buffer_from_vertices(prim);
    return mesh;
}

/**
 * @brief draw a mesh given the parameters
 */
void gf3d_mesh_draw(Mesh *mesh,GFC_Matrix4 modelMat,GFC_Color mod,Texture *texture) {
    MeshUBO ubo = {0};
    slog("drawing mesh");
    if (!mesh) {
        slog("Cannot draw NULL mesh");
        return;
    }

    if (!texture) {
        texture = mesh_manager.defaultTexture;
    }

    gfc_matrix4_copy(ubo.model, modelMat);
    gf3d_vgraphics_get_view(&ubo.view);
    gf3d_vgraphics_get_projection_matrix(&ubo.proj);
    ubo.color = gfc_color_to_vector4f(mod);
    ubo.camera = gfc_vector3dw(gf3d_camera_get_position(), 1.0);
    //ubo.lightPos = gfc_vector3dw(pos, 1.0);
    //ubo.lightCol = gfc_color_to_vector4f(col);

    //ubo = gf3d_mesh_get_ubo(modelMat, mod);
    gfc_matrix4_slog(modelMat);
    gf3d_mesh_queue_render(mesh,mesh_manager.pipe,&ubo,texture);
}

void gf3d_mesh_queue_primitive(MeshPrimitive* primitive, Pipeline* pipe, MeshUBO* ubo, Texture *texture) {
    if (!primitive || !ubo || !texture) {
        slog("Failed to queue mesh primitive");
        return;
    }

    slog("pipeline queue prim");
    gf3d_pipeline_queue_render(pipe, primitive->vertexBuffer, primitive->vertexCount,
        primitive->faceBuffer, ubo, texture);
}

void gf3d_mesh_queue_render(Mesh *mesh, Pipeline* pipe, MeshUBO* uboData,Texture *texture) {
    int i,c;
    MeshPrimitive *prim;
    if ((!mesh) || (!pipe) || (!uboData))return;

    c = gfc_list_count(mesh->primitives);
    for (i = 0;i < c; i++)
    {
        prim = (MeshPrimitive*) gfc_list_nth(mesh->primitives,i);
        if (!prim)continue;
        gf3d_mesh_queue_primitive(prim,pipe,uboData,texture);
    }

}

MeshPrimitive *gf3d_mesh_primitive_new() {
    return gfc_allocate_array(sizeof(MeshPrimitive), 1);
}


VkVertexInputAttributeDescription * gf3d_mesh_get_attribute_descriptions(Uint32 *count) {
    mesh_manager.attributeDescriptions[0].binding = 0;
    mesh_manager.attributeDescriptions[0].location = 0;
    mesh_manager.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    mesh_manager.attributeDescriptions[0].offset = offsetof(Vertex, vertex);

    mesh_manager.attributeDescriptions[1].binding = 0;
    mesh_manager.attributeDescriptions[1].location = 1;
    mesh_manager.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    mesh_manager.attributeDescriptions[1].offset = offsetof(Vertex, normal);

    mesh_manager.attributeDescriptions[2].binding = 0;
    mesh_manager.attributeDescriptions[2].location = 2;
    mesh_manager.attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    mesh_manager.attributeDescriptions[2].offset = offsetof(Vertex, texel);
    if(count) *count = MESH_ATTRIBUTE_COUNT;
    return mesh_manager.attributeDescriptions;
}

VkVertexInputBindingDescription * gf3d_mesh_get_bind_description() {
    mesh_manager.bindingDescription.binding = 0;
    mesh_manager.bindingDescription.stride = sizeof(Vertex);
    mesh_manager.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return &mesh_manager.bindingDescription;
}

void gf3d_mesh_free(Mesh *mesh) {
    if (!mesh) {
        return;
    }

    mesh->_refCount--;
    if (mesh->_refCount-- <= 0) {
        gf3d_mesh_destroy(mesh);
    }
}

void gf3d_mesh_destroy(Mesh *mesh) {
    int i, c;
    MeshPrimitive* prim;

    if (!mesh) {
        return;
    }

    c = gfc_list_count(mesh->primitives);
    for (i = 0; i < c; i++) {
        prim = (MeshPrimitive*) gfc_list_nth(mesh->primitives,i);
        if (!prim) {
            slog("WARN: empty prim in list");
            continue;
        }

        gf3d_mesh_primitive_destroy(prim);
    }

    memset(mesh, 0, sizeof(Mesh));
}

void gf3d_mesh_create_vertex_buffer_from_vertices(MeshPrimitive *prim) {
    void *data = NULL;
    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    Vertex* vertices;
    Uint32 vcount;
    size_t bufferSize;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

    if (!prim)
    {
        slog("no mesh primitive provided");
        return;
    }

    vertices = prim->objData->faceVertices;
    vcount = prim->objData->face_vert_count;
    bufferSize = sizeof(Vertex) * vcount;

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->vertexBuffer, &prim->vertexBufferMemory);

    gf3d_buffer_copy(stagingBuffer, prim->vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    prim->vertexCount = vcount;

}

void gf3d_mesh_create_face_buffer_from_vertices(MeshPrimitive *primitive) {
    VkDevice device;
    VkDeviceSize bufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    ObjData* objData;
    void* data;
    if (!primitive || !primitive->objData) {
        slog("Cannot make vertex buffer from NULL primitive or NULL obj data");
        return;
    }

    device = mesh_manager.device;
    objData = primitive->objData;
    bufferSize = objData->face_count * sizeof(Face);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, objData->outFace, (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &primitive->faceBuffer, &primitive->faceBufferMemory);

    gf3d_buffer_copy(stagingBuffer, primitive->faceBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);
}

Pipeline *gf3d_mesh_get_pipeline() {
    return mesh_manager.pipe;
}

MeshUBO gf3d_mesh_get_ubo(GFC_Matrix4 modelMat, GFC_Color colorMod) {
        MeshUBO ubo = {0};
        gfc_matrix4_copy(ubo.model, modelMat);
        gf3d_vgraphics_get_view(&ubo.view);
        gf3d_vgraphics_get_projection_matrix(&ubo.proj);
        ubo.color = gfc_color_to_vector4(colorMod);
        ubo.camera = gfc_vector3dw(gf3d_camera_get_position(), 1);
        return ubo;
}

void gf3d_mesh_primitive_destroy(MeshPrimitive* prim) {
    VkDevice device;
    if (!prim) {
        return;
    }

    device = mesh_manager.device;
    vkUnmapMemory(device, prim->vertexBufferMemory);
    vkUnmapMemory(device, prim->faceBufferMemory);
    gf3d_obj_free(prim->objData);
    free(prim);
}
