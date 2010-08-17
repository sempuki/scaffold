/* stdheaders.hpp -- exported function header
 *
 *			Ryan McDougall
 */

#ifndef STD_HEADERS_H_
#define STD_HEADERS_H_

#include <cassert>
#include <cmath>
#include <ctime>

#include <ios>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <memory>
#include <algorithm>
#include <iterator>

#include <tr1/cstdint>
#include <tr1/memory>
#include <tr1/functional>
#include <tr1/type_traits>
    
#include <QMutex>

using std::isnan;
using std::isfinite;
using std::string;
using std::stringstream;
using std::for_each;
using std::pair;
using std::make_pair;
using std::auto_ptr;
using std::tr1::shared_ptr;
using std::tr1::function;
using std::tr1::bind;
using std::tr1::mem_fn;
using std::cout;
using std::cerr;
using std::endl;
        
using namespace std::tr1::placeholders;

typedef uint32_t msg_id_t;
typedef int frame_delta_t;
typedef QMutex Mutex;

template <typename T, bool fundamental> struct rvalue_helper {};
template <typename T> struct rvalue_helper <T, true> { typedef T type; };
template <typename T> struct rvalue_helper <T, false> { typedef T const& type; };
template <typename T> struct rvalue_helper <T*, false> { typedef T const* type; };

template <typename T>
struct rvalue
{
    typedef typename rvalue_helper <T,std::tr1::is_fundamental<T>::value>::type type;
};

template <typename T> void safe_delete (T* &ptr) { delete ptr; ptr = 0; }
template <typename T> void safe_array_delete (T* &ptr) { delete [] ptr; ptr = 0; }

struct Locker
{
    Mutex &mutex;
    Locker (Mutex &m) : mutex (m) { mutex.lock(); }
    ~Locker () { mutex.unlock(); }
};

#endif //STD_HEADERS_H_
