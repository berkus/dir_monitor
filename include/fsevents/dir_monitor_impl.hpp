//
// FSEvents-based implementation for OSX
//
// Apple documentation about FSEvents interface:
// https://developer.apple.com/library/mac/documentation/Darwin/Reference/FSEvents_Ref/
//     Reference/reference.html#//apple_ref/doc/c_ref/kFSEventStreamCreateFlagFileEvents
//
// Copyright (c) 2014 Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <string>
#include <deque>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <CoreServices/CoreServices.h>

namespace boost {
namespace asio {

class dir_monitor_impl :
    public boost::enable_shared_from_this<dir_monitor_impl>
{
public:
    dir_monitor_impl()
        : run_(true)
        , work_thread_(&boost::asio::dir_monitor_impl::work_thread, this)
    {}

    ~dir_monitor_impl()
    {
        stop_work_thread();
        work_thread_.join();
        stop_fsevents();
    }

    void add_directory(std::string dirname)
    {
        boost::unique_lock<boost::mutex> lock(dirs_mutex_);
        dirs_.insert(dirname);
        stop_fsevents();
        start_fsevents();
    }

    void remove_directory(const std::string &dirname)
    {
        boost::unique_lock<boost::mutex> lock(dirs_mutex_);
        dirs_.erase(dirname);
        stop_fsevents();
        start_fsevents();
    }

    void destroy()
    {
        boost::unique_lock<boost::mutex> lock(events_mutex_);
        run_ = false;
        events_cond_.notify_all();
    }

    dir_monitor_event popfront_event(boost::system::error_code &ec)
    {
        boost::unique_lock<boost::mutex> lock(events_mutex_);
        while (run_ && events_.empty()) {
            events_cond_.wait(lock);
        }
        dir_monitor_event ev;
        if (!events_.empty())
        {
            ec = boost::system::error_code();
            ev = events_.front();
            events_.pop_front();
        }
        else
            ec = boost::asio::error::operation_aborted;
        return ev;
    }

    void pushback_event(dir_monitor_event ev)
    {
        boost::unique_lock<boost::mutex> lock(events_mutex_);
        if (run_)
        {
            events_.push_back(ev);
            events_cond_.notify_all();
        }
    }

private:
    void start_fsevents()
    {
        // @todo Put paths into CFArrayRef
        CFStringRef mypath = CFSTR("F95A7AE9-D5F5-459a-AB8D-28649FB1FF34");
        CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void **)&mypath, 1, NULL);

        FSEventStreamContext context = {0, this, NULL, NULL, NULL};
        fsevents_ =
            FSEventStreamCreate(
                kCFAllocatorDefault,
                &boost::asio::dir_monitor_impl::fsevents_callback,
                &context,
                pathsToWatch, /* need to recreate on each added/removed dir? */
                kFSEventStreamEventIdSinceNow, /* only new modifications */
                (CFTimeInterval)5.0, /* 5 seconds latency interval */
                kFSEventStreamCreateFlagIgnoreSelf|kFSEventStreamCreateFlagFileEvents);

        if (!fsevents_)
        {
            boost::system::system_error e(boost::system::error_code(errno, boost::system::get_system_category()), "boost::asio::dir_monitor_impl::init_kqueue: kqueue failed");
            boost::throw_exception(e);
        }

        while (!runloop_) {
            boost::this_thread::yield();
        }

        FSEventStreamScheduleWithRunLoop(fsevents_, runloop_, kCFRunLoopDefaultMode);
        FSEventStreamStart(fsevents_);
        runloop_cond_.notify_all();
        FSEventStreamFlushAsync(fsevents_);
    }

    void stop_fsevents()
    {
        if (fsevents_)
        {
            FSEventStreamStop(fsevents_);
            FSEventStreamUnscheduleFromRunLoop(fsevents_, runloop_, kCFRunLoopDefaultMode);
            FSEventStreamInvalidate(fsevents_);
            FSEventStreamRelease(fsevents_);
        }
    }

    static void fsevents_callback(
            ConstFSEventStreamRef streamRef,
            void *clientCallBackInfo,
            size_t numEvents,
            void *eventPaths,
            const FSEventStreamEventFlags eventFlags[],
            const FSEventStreamEventId eventIds[])
    {
        size_t i;
        char **paths = (char**)eventPaths;

        printf("Callback called\n");
        for (i=0; i<numEvents; i++) {
            std::cout << "Change " << eventIds[i] << " in " << paths[i] << ", flags " << [](unsigned flags) {
                std::ostringstream oss;
                if (flags & kFSEventStreamEventFlagMustScanSubDirs) oss << "MustScanSubDirs,";
                if (flags & kFSEventStreamEventFlagUserDropped) oss << "UserDropped,";
                if (flags & kFSEventStreamEventFlagKernelDropped) oss << "KernelDropped,";
                if (flags & kFSEventStreamEventFlagEventIdsWrapped) oss << "EventIdsWrapped,";
                if (flags & kFSEventStreamEventFlagHistoryDone) oss << "HistoryDone,";
                if (flags & kFSEventStreamEventFlagRootChanged) oss << "RootChanged,";
                if (flags & kFSEventStreamEventFlagMount) oss << "Mount,";
                if (flags & kFSEventStreamEventFlagUnmount) oss << "Unmount,";
                if (flags & kFSEventStreamEventFlagItemCreated) oss << "ItemCreated,";
                if (flags & kFSEventStreamEventFlagItemRemoved) oss << "ItemRemoved,";
                if (flags & kFSEventStreamEventFlagItemInodeMetaMod) oss << "ItemInodeMetaMod,";
                if (flags & kFSEventStreamEventFlagItemRenamed) oss << "ItemRenamed,";
                if (flags & kFSEventStreamEventFlagItemModified) oss << "ItemModified,";
                if (flags & kFSEventStreamEventFlagItemFinderInfoMod) oss << "ItemFinderInfoMod,";
                if (flags & kFSEventStreamEventFlagItemChangeOwner) oss << "ItemChangeOwner,";
                if (flags & kFSEventStreamEventFlagItemXattrMod) oss << "ItemXattrMod,";
                if (flags & kFSEventStreamEventFlagItemIsFile) oss << "ItemIsFile,";
                if (flags & kFSEventStreamEventFlagItemIsDir) oss << "ItemIsDir,";
                if (flags & kFSEventStreamEventFlagItemIsSymlink) oss << "ItemIsSymlink,";
                return oss.str();
            }(eventFlags[i]) << std::endl;
            // @todo Split filename into dir and relative fname
            // Figure out event types from set of flags.
            // pushback_event(dir_monitor_event(dir, filename, dir_monitor_event::modified));
       }
    }

    void work_thread()
    {
        runloop_ = CFRunLoopGetCurrent();

        while (running())
        {
            boost::unique_lock<boost::mutex> lock(runloop_mutex_);
            runloop_cond_.wait(lock);
            CFRunLoopRun();
        }
    }

    bool running()
    {
        boost::unique_lock<boost::mutex> lock(work_thread_mutex_);
        return run_;
    }

    void stop_work_thread()
    {
        // Access to run_ is sychronized with running().
        boost::mutex::scoped_lock lock(work_thread_mutex_);
        run_ = false;
        CFRunLoopStop(runloop_); // exits the thread
        runloop_cond_.notify_all();
    }

    bool run_{false};
    CFRunLoopRef runloop_;
    boost::mutex runloop_mutex_;
    boost::condition_variable runloop_cond_;
    boost::mutex work_thread_mutex_;
    boost::thread work_thread_;

    FSEventStreamRef fsevents_;

    boost::mutex dirs_mutex_;
    boost::unordered_set<std::string> dirs_;

    boost::mutex events_mutex_;
    boost::condition_variable events_cond_;
    std::deque<dir_monitor_event> events_;
};

} // asio namespace
} // boost namespace

