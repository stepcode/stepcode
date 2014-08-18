#ifndef SC_THREAD_H
#define SC_THREAD_H

#ifdef HAVE_STD_THREAD
# include <thread>
#endif //HAVE_STD_THREAD

#ifdef HAVE_STD_THREAD
    typedef std::thread::id thread_id_t;
#else //dummy equivalent
    typedef int thread_id_t;
#endif //HAVE_STD_THREAD

/*
 This class is a wrapper to std::thread.
 It will have wrapper functions of different functionalities
 provided by the std::thread class.
 It does nothing if HAVE_STD_THREAD is not defined.
*/
class sc_thread {
    public:

        static thread_id_t getthread_id() {

#ifdef HAVE_STD_THREAD
            return std::this_thread::get_id();
#else
            return 0;
#endif //HAVE_STD_THREAD

        }
};

#endif //SC_THREAD_H
