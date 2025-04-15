//
//  StomaSenseFIFO.h
//  stoma_sense_algorithm
//
//  Created by Lautaro Silbergleit on 24/03/2025.
//

#ifndef _STOMA_SENSE_DATATYPES_H_
#define _STOMA_SENSE_DATATYPES_H_

#include <initializer_list>
#include <cstring>
#include <type_traits>
#include <limits>

namespace StomaSense
{

    template <typename T, typename IDX, size_t N>
    class Array
    {
        static_assert(std::is_arithmetic<T>::value || std::is_pointer<T>::value, "T should be an arithmetic type (for example int or float) or a pointer");
        static_assert(std::is_unsigned<IDX>::value, "IDX should be an unsigned integral type (for example uint)");

    private:
        T _data[N] = {0};
        IDX _size = 0;

        void _swap(IDX i, IDX j)
        {
            T temp;

            temp = _data[i];
            _data[i] = _data[j];
            _data[j] = temp;
        }

    public:
        IDX size() const { return _size; }
        bool full() const { return size() == N; }
        bool empty() const { return size() == 0; }
        void clear() { _size = 0; }

        void fill(T v)
        {
            memset(_data, v, N);
        }

        bool contains(T v, IDX *idx)
        {
            // returns true if v is in the array
            // if idx != nullptr, it will store the index where the first appearence of the element is
            for (IDX i = 0; i < size(); ++i)
            {
                if (_data[i] == v)
                {
                    if (idx)
                        *idx = i;
                    return true;
                }
            }
            return false;
        }

        // Adding
        bool append(T v)
        {
            if (full())
                return false;

            _data[size()] = v;

            ++_size;
            return true;
        }
        bool append(const T *a, IDX length)
        {
            if (length > N - size())
                return false;

            memcpy(&_data[size()], a, sizeof(T) * length);

            _size += length;
            return true;
        }
        bool append(std::initializer_list<T> l)
        {
            return append(l.begin(), static_cast<IDX>(l.size()));
        }
        bool insert(T v, IDX i)
        {
            if (full() || i >= size())
                return false;

            memcpy(&_data[i + 1], &_data[i], sizeof(T) * (size() - i));

            _data[i] = v;

            ++_size;
            return true;
        }
        bool insert(const T *a, IDX length, IDX i)
        {
            if (i >= size() || length > N - size())
                return false; // TODO: check this

            memcpy(&_data[i + length], &_data[i], sizeof(T) * (size() - i));
            memcpy(&_data[i], a, sizeof(T) * length);

            _size += length;
            return true;
        }
        bool insert(std::initializer_list<T> l, IDX i)
        {
            return insert(l.begin(), static_cast<IDX>(l.size()), i);
        }
        bool prepend(T v)
        {
            return insert(v, 0);
        }
        bool prepend(std::initializer_list<T> l)
        {
            return insert(l, 0);
        }
        bool prepend(const T *a, IDX length)
        {
            return insert(a, length, 0);
        }

        // Peeking
        bool peek(T *v, IDX i)
        {
            if (empty() || i >= size())
                return false;

            (*v) = _data[i];

            return true;
        }

        // Removing
        bool pop(T *v)
        {
            if (empty())
                return false;

            (*v) = _data[size() - 1];

            --_size;
            return true;
        }
        bool remove(T *v, IDX i)
        {
            if (!peek(v, i))
                return false;

            if (i < size() - 1)
                memcpy(&_data[i], &_data[i + 1], sizeof(T) * (size() - i));

            --_size;
            return true;
        }
        bool pop_front(T *v)
        {
            return remove(v, 0);
        }

        // casting & data
        const T *c_array() const { return _data; }
        T *_c_array_unsafe() { return _data; }
        void _set_c_array_size_unsafe(size_t size)
        {
            // this can be used in conjunction with _c_array_unsafe()
            _size = size;
        }

        // sorting
        template <typename Func>
        void sort(Func score)
        {
            static_assert(std::is_arithmetic<typename std::result_of<Func(T)>::type>::value,
                          "sort expects a function that returns an arithmetic type and takes one parameter of type T");
            typedef typename std::result_of<Func(T)>::type SCR;

            /// https://en.wikipedia.org/wiki/Insertion_sort
            /// Insertion sort because it is fast for small arrays

            if (empty())
                return;

            SCR s;
            IDX j;
            T temp;
            for (IDX i = 1; i < size(); ++i)
            {
                peek(&temp, i);
                s = score(temp);
                j = i;

                while (j > 0)
                {
                    if (score(_data[j - 1]) < s)
                        break;
                    _data[j] = _data[j - 1];
                    --j;
                }
                _data[j] = temp;
            }
        }
        template <typename Func>
        void sort_distance(Func distance, T &v_start)
        {
            // Check if Func is callable

            static_assert(std::is_arithmetic<typename std::result_of<Func(T, T)>::type>::value,
                          "sort_distance expects a function that returns an arithmetic type and takes two parameters of type T");
            typedef typename std::result_of<Func(T, T)>::type DST;

            /// https://en.wikipedia.org/wiki/Travelling_salesman_problem
            /// Traveling salesman algorithm in 1D

            if (empty())
                return;

            const DST max = std::numeric_limits<DST>::max(); // TODO: Maybe change const for constexpr
            DST dst, closest_dst = max;
            IDX i, j, closest_idx = 0;

            // find starting element of array
            for (j = 0; j < size(); ++j)
            {
                dst = distance(v_start, _data[j]);
                if (dst < closest_dst)
                {
                    closest_dst = dst;
                    closest_idx = j;
                }
            }
            if (0 != closest_idx)
                _swap(0, closest_idx);

            // find every other element
            for (i = 1; i < size() - 1; ++i)
            {
                closest_dst = max;
                for (j = i; j < size(); ++j)
                {
                    dst = distance(_data[i - 1], _data[j]);
                    if (dst < closest_dst)
                    {
                        closest_dst = dst;
                        closest_idx = j;
                    }
                }
                if (i != closest_idx)
                    _swap(i, closest_idx);
            }
        }
        template <typename Func>
        void sort_distance(Func distance)
        {
            if (empty())
                return;
            sort_distance<decltype(distance)>(distance, _data[0]);
        }

        // boolean tests
        template <typename Func>
        bool any(Func test)
        {
            static_assert(std::is_same<bool, typename std::result_of<Func(T)>::type>::value,
                          "any expects a function that returns a bool and takes two parameters of type T");
            for (IDX i = 0; i < size(); ++i)
            {
                if (test(_data[i]))
                    return true;
            }
            return false;
        }
        template <typename Func>
        IDX bool_test(Func test)
        {
            static_assert(std::is_same<bool, typename std::result_of<Func(T)>::type>::value,
                          "bool_test expects a function that returns a bool and takes two parameters of type T");
            IDX s = 0;
            for (IDX i = 0; i < size(); ++i)
            {
                if (test(_data[i]))
                    ++s;
            }
            return s;
        }
        template <typename Func>
        bool all(Func test)
        {
            static_assert(std::is_same<bool, typename std::result_of<Func(T)>::type>::value,
                          "any expects a function that returns a bool and takes two parameters of type T");
            if (empty())
                return false;
            for (IDX i = 0; i < size(); ++i)
            {
                if (!test(_data[i]))
                    return false;
            }
            return true;
        }

        bool compare(std::initializer_list<T> l)
        {
            if (size() != l.size())
                return false;

            return memcmp(&_data[0], l.begin(), sizeof(T) * size()) == 0;
        }
        bool compare(const T *a, IDX length)
        {
            if (size() != length)
                return false;

            return memcmp(&_data[0], a, sizeof(T) * size()) == 0;
        }
    };

}

#endif /* _STOMA_SENSE_DATATYPES_H_ */