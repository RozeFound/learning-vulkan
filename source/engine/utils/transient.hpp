#pragma once

namespace engine {

    class TransientBuffer {

        vk::Queue queue;
        vk::CommandPool pool;
        vk::CommandBuffer buffer;
        vk::UniqueFence submition_completed;

        public:

        TransientBuffer (bool graphics_capable = false);
        ~TransientBuffer ( );

        vk::CommandBuffer& get ( );
        void submit ( );

    };

}