#pragma once
#include "vrender.h"

#include "managers/asset_manager.h"
#include "rendering/material.h"

namespace scene
{
    inline size_t  GlobalNodeCounter = 0;
    
    class Object
    {
    private:
        size_t Handle = GlobalNodeCounter++;
    public:
        virtual ~Object() {}

        inline size_t GetHandle()
        {
            return Handle;
        }
    };

    class Spatial : public Object
    {
    public:
        glm::vec3 Position = glm::vec3(0.0f);
        glm::vec3 Scale = glm::vec3(1.0f);
        glm::vec4 Rotation = { glm::vec3(1.0f), 0.0f };
    };

    enum class NodeChannel
    {
        MeshRenderable,
        PointLight,
        Spotlight
    };

	class Node : public Spatial
	{
	private:
        Node* Parent = nullptr;
		std::unordered_map<size_t, Node*> Childs;
	public:
        inline void SetParent(Node* node)
        {
           Parent = node;
        }
        
        inline void AttachChild(Node* node)
        {
           Childs[node->GetHandle()] = node;

           node->SetParent(this);
        }

        inline void DetachChild(Node* node)
        {
           Childs.erase(node->GetHandle());

           node->SetParent(nullptr);
        }

        inline glm::vec3 GetWorldPosition() const
        {
            if (!Parent)
                return Position;
            
            return Parent->GetWorldPosition() + Position;
        }

        inline glm::vec3 GetWorldScale() const
        {
            if (!Parent)
                return Scale;

            return Parent->GetWorldScale() * Scale;
        }

        inline glm::vec4 GetWorldRotation() const
        {
            if (!Parent)
                return Rotation;

            auto w = Parent->GetWorldRotation();
            auto l = Rotation;
            auto rot = glm::vec4(glm::vec3(w) * glm::vec3(l), w.w + l.w);

            return rot;
        }

        virtual bool CheckChannel(const NodeChannel c) const 
        { 
            return false;
        }

        //Looks up through child nodes and return nodes with desired channel
        template<typename T, std::enable_if_t<std::is_base_of_v<Node, T>>* = nullptr>
        std::vector<T*> GetNodesWithChannel(const NodeChannel channel)
        {
            std::vector<T*> v;

            for (auto&[h, c] : Childs)
            {
                auto cv = c->GetNodesWithChannel<T>(channel);
                v.insert(v.end(), cv.begin(), cv.end());

                if (c->CheckChannel(channel))
                    v.push_back(dynamic_cast<T*>(c));
            }

            return v;
        }
    };

    struct RenderInfo
    {
        VkCompareOp DepthCompareOp = VK_COMPARE_OP_LESS;
        VkCullModeFlags FacesCullMode = VK_CULL_MODE_BACK_BIT;
    };

    class MeshRenderable : public Node
    {
    public:
        RenderInfo Render;

        std::shared_ptr<render::BaseMaterial> Material;

        asset::MeshInfo Info;

        virtual bool CheckChannel(const NodeChannel c) const override
        {
            return (c == NodeChannel::MeshRenderable);
        }
    };

    class PointLight : public Node
    {
    public:
        glm::vec3 Color;

        virtual bool CheckChannel(const NodeChannel c) const override
        {
            return (c == NodeChannel::PointLight);
        }
    };

    class Spotlight : public PointLight
    {
    public:
        float OuterAngle;
        float InnerAngle;

        virtual bool CheckChannel(const NodeChannel c) const override
        {
            return (c == NodeChannel::Spotlight);
        }
    };
}
