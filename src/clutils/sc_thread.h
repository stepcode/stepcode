#ifndef SC_THREAD_H
#define SC_THREAD_H

#ifdef HAVE_STD_THREAD
# include <thread>
#endif //HAVE_STD_THREAD

#ifdef HAVE_STD_THREAD
    typedef std::thread::id thread_id_t;

    static thread_id_t sc_getthread_id() {
        return std::this_thread::get_id();
    }

#else //dummy equivalent
    typedef int thread_id_t;

    static thread_id_t sc_getthread_id() {
        return 0;
    }

#endif //HAVE_STD_THREAD

#endif //SC_THREAD_H
