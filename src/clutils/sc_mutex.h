#ifndef SC_MUTEX_H
#define SC_MUTEX_H

#ifdef HAVE_STD_THREAD
# include <mutex>
# include <thread>
#endif //HAVE_STD_THREAD
/*
 This class is a wrapper to std::mutex.
 It does nothing if HAVE_STD_THREAD is not defined.
*/
class sc_mutex {
    protected:
#ifdef HAVE_STD_THREAD
        std::mutex mtx;
#endif //HAVE_STD_THREAD

    public:
        void lock() {
#ifdef HAVE_STD_THREAD
            mtx.lock();
#endif //HAVE_STD_THREAD
        }

        void unlock() {
#ifdef HAVE_STD_THREAD
            mtx.unlock();
#endif //HAVE_STD_THREAD
        }
};

class sc_recursive_mutex {
    protected:
#ifdef HAVE_STD_THREAD
		std::recursive_mutex mtx;
#endif //HAVE_STD_THREAD

    public:
        void lock() {
#ifdef HAVE_STD_THREAD
            mtx.lock();
#endif //HAVE_STD_THREAD
        }

        void unlock() {
#ifdef HAVE_STD_THREAD
            mtx.unlock();
#endif //HAVE_STD_THREAD
        }
};
#endif //SC_MUTEX_H
