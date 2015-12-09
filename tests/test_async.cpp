//
// Copyright (c) 2008, 2009 Boris Schaeling <boris@highscore.de>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include "dir_monitor.hpp"
#include "directory.hpp"

boost::asio::io_service io_service;

void create_file_handler(const boost::system::error_code &ec, const boost::asio::dir_monitor_event &ev)
{
    boost::filesystem::path fullpath = boost::filesystem::canonical(TEST_DIR1);
    fullpath /= TEST_FILE1;

    BOOST_CHECK_EQUAL(ec, boost::system::error_code());
    BOOST_CHECK_EQUAL(ev.path, fullpath);
    BOOST_CHECK_EQUAL(ev.type, boost::asio::dir_monitor_event::added);
}

BOOST_AUTO_TEST_CASE(create_file)
{
    directory dir(TEST_DIR1);

    boost::asio::dir_monitor dm(io_service);
    dm.add_directory(TEST_DIR1);

    dir.create_file(TEST_FILE1);

    dm.async_monitor(create_file_handler);
    io_service.run();
    io_service.reset();
}

void rename_file_handler_old(const boost::system::error_code &ec, const boost::asio::dir_monitor_event &ev)
{
    boost::filesystem::path fullpath = boost::filesystem::canonical(TEST_DIR1);
    fullpath /= TEST_FILE1;

    BOOST_CHECK_EQUAL(ec, boost::system::error_code());
    BOOST_CHECK_EQUAL(ev.path, fullpath);
    BOOST_CHECK_EQUAL(ev.type, boost::asio::dir_monitor_event::renamed_old_name);
}

void rename_file_handler_new(const boost::system::error_code &ec, const boost::asio::dir_monitor_event &ev)
{
    boost::filesystem::path fullpath = boost::filesystem::canonical(TEST_DIR1);
    fullpath /= TEST_FILE2;

    BOOST_CHECK_EQUAL(ec, boost::system::error_code());
    BOOST_CHECK_EQUAL(ev.path, fullpath);
    BOOST_CHECK_EQUAL(ev.type, boost::asio::dir_monitor_event::renamed_new_name);
}

void modify_file_handler(const boost::system::error_code &ec, const boost::asio::dir_monitor_event &ev)
{
    boost::filesystem::path fullpath = boost::filesystem::canonical(TEST_DIR1);
    fullpath /= TEST_FILE2;

    BOOST_CHECK_EQUAL(ec, boost::system::error_code());
    BOOST_CHECK_EQUAL(ev.path, fullpath);
    BOOST_CHECK_EQUAL(ev.type, boost::asio::dir_monitor_event::modified);
}

BOOST_AUTO_TEST_CASE(rename_file)
{
    directory dir(TEST_DIR1);
    dir.create_file(TEST_FILE1);

    boost::asio::dir_monitor dm(io_service);
    dm.add_directory(TEST_DIR1);

    dir.rename_file(TEST_FILE1, TEST_FILE2);

    dm.async_monitor(rename_file_handler_old);
    io_service.run();
    io_service.reset();

    dm.async_monitor(rename_file_handler_new);
    io_service.run();
    io_service.reset();

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    dm.async_monitor(modify_file_handler);
    io_service.run();
    io_service.reset();
#endif
}

void remove_file_handler(const boost::system::error_code &ec, const boost::asio::dir_monitor_event &ev)
{
    boost::filesystem::path fullpath = boost::filesystem::canonical(TEST_DIR1);
    fullpath /= TEST_FILE1;

    BOOST_CHECK_EQUAL(ec, boost::system::error_code());
    BOOST_CHECK_EQUAL(ev.path, fullpath);
    BOOST_CHECK_EQUAL(ev.type, boost::asio::dir_monitor_event::removed);
}

BOOST_AUTO_TEST_CASE(remove_file)
{
    directory dir(TEST_DIR1);
    dir.create_file(TEST_FILE1);

    boost::asio::dir_monitor dm(io_service);
    dm.add_directory(TEST_DIR1);

    dir.remove_file(TEST_FILE1);

    dm.async_monitor(remove_file_handler);
    io_service.run();
    io_service.reset();
}

BOOST_AUTO_TEST_CASE(multiple_events)
{
    directory dir(TEST_DIR1);

    boost::asio::dir_monitor dm(io_service);
    dm.add_directory(TEST_DIR1);

    dir.create_file(TEST_FILE1);
    dir.rename_file(TEST_FILE1, TEST_FILE2);

    dm.async_monitor(create_file_handler);
    io_service.run();
    io_service.reset();

    dm.async_monitor(rename_file_handler_old);
    io_service.run();
    io_service.reset();

    dm.async_monitor(rename_file_handler_new);
    io_service.run();
    io_service.reset();

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    dm.async_monitor(modify_file_handler);
    io_service.run();
    io_service.reset();
#endif
}

void aborted_async_call_handler(const boost::system::error_code &ec, const boost::asio::dir_monitor_event &ev)
{
    BOOST_CHECK_EQUAL(ec, boost::asio::error::operation_aborted);
}

BOOST_AUTO_TEST_CASE(aborted_async_call)
{
    directory dir(TEST_DIR1);

    {
        boost::asio::dir_monitor dm(io_service);
        dm.add_directory(TEST_DIR1);

        dm.async_monitor(aborted_async_call_handler);
    }

    io_service.run();
    io_service.reset();
}

void blocked_async_call_handler_with_local_ioservice(const boost::system::error_code &ec, const boost::asio::dir_monitor_event &ev)
{
    BOOST_CHECK_EQUAL(ec, boost::asio::error::operation_aborted);
}

BOOST_AUTO_TEST_CASE(blocked_async_call)
{
    directory dir(TEST_DIR1);
    boost::thread t;

    {
        boost::asio::io_service io_service;

        boost::asio::dir_monitor dm(io_service);
        dm.add_directory(TEST_DIR1);

        dm.async_monitor(blocked_async_call_handler_with_local_ioservice);

        // run() is invoked on another thread to make async_monitor() call a blocking function.
        // When dm and io_service go out of scope they should be destroyed properly without
        // a thread being blocked.
        t = boost::thread(boost::bind(&boost::asio::io_service::run, boost::ref(io_service)));
        boost::system_time time = boost::get_system_time();
        time += boost::posix_time::time_duration(0, 0, 1);
        boost::thread::sleep(time);
    }

    t.join();
    io_service.reset();
}

void unregister_directory_handler(const boost::system::error_code &ec, const boost::asio::dir_monitor_event &ev)
{
    BOOST_CHECK_EQUAL(ec, boost::asio::error::operation_aborted);
}

BOOST_AUTO_TEST_CASE(unregister_directory)
{
    directory dir(TEST_DIR1);
    boost::thread t;

    {
        boost::asio::dir_monitor dm(io_service);
        dm.add_directory(TEST_DIR1);
        dm.remove_directory(TEST_DIR1);

        dir.create_file(TEST_FILE1);

        dm.async_monitor(unregister_directory_handler);

        // run() is invoked on another thread to make this test case return. Without using
        // another thread run() would block as the file was created after remove_directory()
        // had been called.
        t = boost::thread(boost::bind(&boost::asio::io_service::run, boost::ref(io_service)));
        boost::system_time time = boost::get_system_time();
        time += boost::posix_time::time_duration(0, 0, 1);
        boost::thread::sleep(time);
    }

    t.join();
    io_service.reset();
}

void two_dir_monitors_handler(const boost::system::error_code &ec, const boost::asio::dir_monitor_event &ev)
{
    BOOST_CHECK_EQUAL(ec, boost::asio::error::operation_aborted);
}

BOOST_AUTO_TEST_CASE(two_dir_monitors)
{
    directory dir1(TEST_DIR1);
    directory dir2(TEST_DIR2);
    boost::thread t;

    {
        boost::asio::dir_monitor dm1(io_service);
        dm1.add_directory(TEST_DIR1);

        boost::asio::dir_monitor dm2(io_service);
        dm2.add_directory(TEST_DIR2);

        dir2.create_file(TEST_FILE1);

        dm1.async_monitor(two_dir_monitors_handler);

        // run() is invoked on another thread to make this test case return. Without using
        // another thread run() would block as the directory the file was created in is
        // monitored by dm2 while async_monitor() was called for dm1.
        t = boost::thread(boost::bind(&boost::asio::io_service::run, boost::ref(io_service)));
        boost::system_time time = boost::get_system_time();
        time += boost::posix_time::time_duration(0, 0, 1);
        boost::thread::sleep(time);
    }

    t.join();
    io_service.reset();
}

