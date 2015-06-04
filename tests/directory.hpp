//
// Copyright (c) 2008, 2009 Boris Schaeling <boris@highscore.de>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <fstream>

#define TEST_DIR1 "A95A7AE9-D5F5-459a-AB8D-28649FB1F3F4"
#define TEST_DIR2 "EA63DF88-7BFF-4038-B317-F37434DF4ED1"
#define TEST_FILE1 "test1.txt"
#define TEST_FILE2 "test2.txt"

class directory
{
public:
    directory(const char *name)
        : name_(boost::filesystem::complete(name))
    {
        boost::filesystem::create_directory(name_);
        BOOST_REQUIRE(boost::filesystem::is_directory(name_));
    }

    ~directory()
    {
        bool again;
        do
        {
            try
            {
                boost::filesystem::remove_all(name_);
                again = false;
            }
            catch (...)
            {
                boost::this_thread::yield();
                again = true;
            }
        } while (again);
    }

    void create_file(const char *file)
    {
        boost::filesystem::current_path(name_);
        BOOST_REQUIRE(boost::filesystem::equivalent(name_, boost::filesystem::current_path()));
        std::ofstream ofs(file);
        BOOST_REQUIRE(boost::filesystem::exists(file));
        boost::filesystem::current_path(boost::filesystem::initial_path());
        BOOST_REQUIRE(boost::filesystem::equivalent(boost::filesystem::current_path(), boost::filesystem::initial_path()));
    }

    void rename_file(const char *from, const char *to)
    {
        boost::filesystem::current_path(name_);
        BOOST_REQUIRE(boost::filesystem::equivalent(name_, boost::filesystem::current_path()));
        BOOST_REQUIRE(boost::filesystem::exists(from));
        boost::filesystem::rename(from, to);
        BOOST_REQUIRE(boost::filesystem::exists(to));
        boost::filesystem::current_path(boost::filesystem::initial_path());
        BOOST_REQUIRE(boost::filesystem::equivalent(boost::filesystem::current_path(), boost::filesystem::initial_path()));
    }

    void remove_file(const char *file)
    {
        boost::filesystem::current_path(name_);
        BOOST_REQUIRE(boost::filesystem::equivalent(name_, boost::filesystem::current_path()));
        BOOST_REQUIRE(boost::filesystem::exists(file));
        boost::filesystem::remove(file);
        boost::filesystem::current_path(boost::filesystem::initial_path());
        BOOST_REQUIRE(boost::filesystem::equivalent(boost::filesystem::current_path(), boost::filesystem::initial_path()));
    }

private:
    boost::filesystem::path name_;
};

