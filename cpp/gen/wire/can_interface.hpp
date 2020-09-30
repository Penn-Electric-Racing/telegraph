#ifndef __TELEGRAPH_GEN_CAN_INTERFACE_HPP__
#define __TELEGRAPH_GEN_CAN_INTERFACE_HPP__

namespace wire {
    // a can interface can be added to all
    // of these different interface types
    class can_interface : 
        public interface<uint8_t>,
        public interface<uint16_t>
        public interface<uint32_t>
        public interface<uint64_t> {
    public:
        void send(int16_t id, uint8_t len, uint8_t* buf) = 0;
    private:
    };
}


#endif
