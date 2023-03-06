#pragma once

namespace engine {

    class TransientBuffer {

        vk::Queue queue;
        vk::CommandPool pool;
        vk::CommandBuffer buffer;

        public:

        TransientBuffer ( );
        ~TransientBuffer ( );

        vk::CommandBuffer& get ( );
        void submit ( );

    };

}