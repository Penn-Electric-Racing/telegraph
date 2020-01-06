#ifndef __TELEGRAPH_NAMESPACE_HPP__
#define __TELEGRAPH_NAMESPACE_HPP__

namespace telegraph {
    class name_space {
    public:
        name_space(const std::string& uuid) : uuid_(uuid) {}

        const std::string& get_uuid() const { return uuid_; }
    private:
        std::string uuid_;
    };
}

#endif
