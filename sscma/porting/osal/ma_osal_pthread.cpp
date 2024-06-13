#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>

#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <sys/syscall.h>

#include "core/ma_common.h"
#include "porting/ma_osal.h"

namespace ma {
ma_tick_t Tick::current() {
    struct timespec ts;
    ma_tick_t tick;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    tick = ts.tv_sec;
    tick *= 1000000;
    tick += ts.tv_nsec;

    return tick;
}

ma_tick_t Tick::fromMicroseconds(uint32_t us) {
    return us * 1000;
}

ma_tick_t Tick::fromMilliseconds(uint32_t ms) {
    return ms * 1000 * 1000;
}

void Tick::sleep(ma_tick_t tick) {
    struct timespec ts;
    struct timespec remain;

    ts.tv_sec  = tick / (1000000000);
    ts.tv_nsec = tick % (1000000000);
    while (clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, &remain) != 0) {
        ts = remain;
    }
}

Thread::Thread(const char* name,
               void (*entry)(void*),
               uint32_t priority,
               size_t stacksize,
               ma_stack_t* stack) noexcept
    : m_name(name), m_entry(entry), m_priority(priority), m_stacksize(stacksize), m_stack(stack) {}


void Thread::threadEntryPoint(void) {
    if (m_entry != nullptr) {
        m_entry(m_arg);
    }
}

void* Thread::threadEntryPointStub(void* arg) {
    Thread* thread = static_cast<Thread*>(arg);
    if (thread != nullptr) {
        MA_LOGI("asdf", "Thread %s started", thread->m_name);
        thread->threadEntryPoint();
    }
    return nullptr;
}

Thread::~Thread() noexcept {}

Thread::operator bool() const {
    return true;
}

void Thread::start(void* arg) {

    int result = 0;
    m_arg      = arg;
    result     = pthread_create(&m_thread, nullptr, threadEntryPointStub, this);
    pthread_setname_np(m_thread, m_name);
    pthread_detach(m_thread);
}

bool Thread::operator==(const Thread& other) const {
    return pthread_equal(m_thread, other.m_thread) != 0;
}


void Thread::yield() {
    sched_yield();
}

ma_thread_t* Thread::self() {
    return reinterpret_cast<ma_thread_t*>(pthread_self());
}


Mutex::Mutex() noexcept {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

Mutex::operator ma_mutex_t*() const {
    return &m_mutex;
}

Mutex::~Mutex() noexcept {

    pthread_mutex_destroy(&m_mutex);
}

Mutex::operator bool() const {
    return true;
}

bool Mutex::tryLock(uint32_t timeout) {
    return pthread_mutex_trylock(&m_mutex) == 0;
}

bool Mutex::lock() const {
    return pthread_mutex_lock(&m_mutex) == 0;
}

bool Mutex::unlock() const {
    return pthread_mutex_unlock(&m_mutex) == 0;
}


Guard::Guard(const Mutex& mutex) noexcept : m_mutex(mutex) {
    m_mutex.lock();
}

Guard::~Guard() noexcept {
    m_mutex.unlock();
}

Guard::operator bool() const {
    return true;
}


Semaphore::Semaphore(size_t count) noexcept {
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    pthread_cond_init(&m_sem.cond, &cattr);
    pthread_condattr_destroy(&cattr);
    m_sem.count = count;
}

Semaphore::~Semaphore() noexcept {
    pthread_cond_destroy(&m_sem.cond);
}


Semaphore::operator bool() const {
    return true;
}


bool Semaphore::wait(uint32_t timeout) {
    struct timespec ts;
    int error     = 0;
    uint64_t nsec = (uint64_t)timeout * 1000000;

    if (timeout != Tick::waitForever) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        nsec += ts.tv_nsec;
        if (nsec > (1000000000)) {
            ts.tv_sec += nsec / (1000000000);
            nsec %= (1000000000);
        }
        ts.tv_nsec = nsec;
    }

    Guard guard(m_mutex);

    while (m_sem.count == 0) {
        if (timeout != Tick::waitForever) {
            error = pthread_cond_timedwait(&m_sem.cond, static_cast<ma_mutex_t*>(m_mutex), &ts);
            MA_ASSERT(error != EINVAL);
            if (error) {
                return false;
            }
        } else {
            error = pthread_cond_wait(&m_sem.cond, static_cast<ma_mutex_t*>(m_mutex));
            MA_ASSERT(error != EINVAL);
        }
    }
    m_sem.count--;
    return true;
}


uint32_t Semaphore::getCount() const {
    return m_sem.count;
}

void Semaphore::signal() {
    Guard guard(m_mutex);
    if (m_sem.count == 0) {
        pthread_cond_signal(&m_sem.cond);
    }
    m_sem.count++;
}


Event::Event() noexcept {
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    pthread_cond_init(&m_event.cond, &cattr);
    pthread_condattr_destroy(&cattr);
}

Event::~Event() noexcept {
    pthread_cond_destroy(&m_event.cond);
}

Event::operator bool() const {
    return true;
}


bool Event::wait(uint32_t mask, uint32_t* value, uint32_t timeout) {
    struct timespec ts;
    int error     = 0;
    uint64_t nsec = (uint64_t)timeout * 1000000;

    if (timeout != Tick::waitForever) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        nsec += ts.tv_nsec;
        if (nsec > (1000000000)) {
            ts.tv_sec += nsec / (1000000000);
            nsec %= (1000000000);
        }
        ts.tv_nsec = nsec;
    }

    Guard guard(m_mutex);
    while ((m_event.value & mask) == 0) {
        if (timeout != Tick::waitForever) {
            error = pthread_cond_timedwait(&m_event.cond, static_cast<ma_mutex_t*>(m_mutex), &ts);
            MA_ASSERT(error != EINVAL);
            if (error) {
                return false;
            }
        } else {
            error = pthread_cond_wait(&m_event.cond, static_cast<ma_mutex_t*>(m_mutex));
            MA_ASSERT(error != EINVAL);
        }
    }

    if (value != nullptr) {
        *value = m_event.value & mask;
    }
    return true;
}


void Event::clear(uint32_t value) {
    Guard guard(m_mutex);
    m_event.value &= ~value;
}

void Event::set(uint32_t value) {
    Guard guard(m_mutex);
    m_event.value |= value;
}

uint32_t Event::get() const {
    return m_event.value;
}


MessageBox::MessageBox(size_t size) noexcept {
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    pthread_cond_init(&m_mbox.cond, &cattr);
    pthread_condattr_destroy(&cattr);
    m_mbox.r     = 0;
    m_mbox.w     = 0;
    m_mbox.count = 0;
    m_mbox.size  = size;
}

MessageBox::~MessageBox() noexcept {
    pthread_cond_destroy(&m_mbox.cond);
}

MessageBox::operator bool() const {
    return true;
}


bool MessageBox::fetch(const void** msg, uint32_t timeout) {
    struct timespec ts;
    int error     = 0;
    uint64_t nsec = (uint64_t)timeout * 1000000;

    if (timeout != Tick::waitForever) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        nsec += ts.tv_nsec;
        if (nsec > (1000000000)) {
            ts.tv_sec += nsec / (1000000000);
            nsec %= (1000000000);
        }
        ts.tv_nsec = nsec;
    }

    Guard guard(m_mutex);
    while (m_mbox.count == 0) {
        if (timeout != Tick::waitForever) {
            error = pthread_cond_timedwait(&m_mbox.cond, static_cast<ma_mutex_t*>(m_mutex), &ts);
            MA_ASSERT(error != EINVAL);
            if (error) {
                return false;
            }
        } else {
            error = pthread_cond_wait(&m_mbox.cond, static_cast<ma_mutex_t*>(m_mutex));
            MA_ASSERT(error != EINVAL);
        }
    }
    m_mbox.count--;
    *msg     = m_mbox.msg[m_mbox.r];
    m_mbox.r = (m_mbox.r + 1) % m_mbox.size;
    return true;
}


bool MessageBox::post(const void* msg, uint32_t timeout) {
    struct timespec ts;
    int error     = 0;
    uint64_t nsec = (uint64_t)timeout * 1000000;

    if (timeout != Tick::waitForever) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        nsec += ts.tv_nsec;
        if (nsec > (1000000000)) {
            ts.tv_sec += nsec / (1000000000);
            nsec %= (1000000000);
        }
        ts.tv_nsec = nsec;
    }

    Guard guard(m_mutex);
    while (m_mbox.count == m_mbox.size) {
        if (timeout != Tick::waitForever) {
            error = pthread_cond_timedwait(&m_mbox.cond, static_cast<ma_mutex_t*>(m_mutex), &ts);
            MA_ASSERT(error != EINVAL);
            if (error) {
                return false;
            }
        } else {
            error = pthread_cond_wait(&m_mbox.cond, static_cast<ma_mutex_t*>(m_mutex));
            MA_ASSERT(error != EINVAL);
        }
    }
    m_mbox.msg[m_mbox.w] = msg;
    m_mbox.count++;
    m_mbox.w = (m_mbox.w + 1) % m_mbox.size;
    pthread_cond_signal(&m_mbox.cond);

    return true;
}

// Timer::Timer(uint32_t ms, void (*fn)(ma_timer_t*, void*), void* arg, bool oneshot) noexcept {
//     pthread_condattr_t cattr;
//     pthread_condattr_init(&cattr);
//     pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
//     pthread_cond_init(&m_timer.cond, &cattr);
//     pthread_condattr_destroy(&cattr);
//     m_timer.ms      = ms;
//     m_timer.fn      = fn;
//     m_timer.arg     = arg;
//     m_timer.exit    = false;
//     m_timer.oneshot = oneshot;
//     m_timer.thread_id = 0;

//     /* Block SIGALRM in calling thread */
//     sigemptyset(&sigset);
//     sigaddset(&sigset, SIGALRM);
//     sigprocmask(SIG_BLOCK, &sigset, NULL);
// }


// Timer::~Timer() noexcept {
//     pthread_cond_destroy(&m_timer.cond);
// }


// Timer::operator bool() const {
//     return true;
// }


// void Timer::set(uint32_t ms) {
//     Guard guard(m_mutex);
//     m_timer.ms = ms;
// }


// void Timer::start() {
//     Guard guard(m_mutex);
//     m_timer.exit = false;
//     pthread_cond_signal(&m_timer.cond);
// }

// void Timer::stop() {
//     Guard guard(m_mutex);
//     m_timer.exit = true;
//     pthread_cond_signal(&m_timer.cond);
// }


}  // namespace ma