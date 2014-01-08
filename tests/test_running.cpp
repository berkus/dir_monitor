#include <boost/filesystem.hpp>
#include "dir_monitor.hpp"

#define TEST_DIR1 "F95A7AE9-D5F5-459a-AB8D-28649FB1FF34"

boost::asio::io_service io_service;

void create_file_handler(const boost::system::error_code &ec, const boost::asio::dir_monitor_event &ev)
{
    std::cout << "Dir monitor event "
        << [](int type) { switch(type) {
            case boost::asio::dir_monitor_event::added: return "ADDED";
            case boost::asio::dir_monitor_event::removed: return "REMOVED";
            case boost::asio::dir_monitor_event::modified: return "MODIFIED";
            case boost::asio::dir_monitor_event::renamed_old_name: return "RENAMED (OLD NAME)";
            case boost::asio::dir_monitor_event::renamed_new_name: return "RENAMED (NEW NAME)";
            case boost::asio::dir_monitor_event::recursive_rescan: return "RESCAN DIR";
            default: return "UKNOWN";
        } } (ev.type) << " " << ev.path << std::endl;
}

int main()
{
    boost::filesystem::create_directory(TEST_DIR1);

    boost::asio::dir_monitor dm(io_service);
    dm.add_directory(TEST_DIR1);

    dm.async_monitor(create_file_handler);
    io_service.run();
    // io_service.reset();
}
