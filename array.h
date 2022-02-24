#pragma once

#include <assert.h>
#include <string.h>
#include <stdio.h>

struct range
{   
    int start, stop;
};

//--------------------------------------

// Basic type representing a pointer to some
// data and the size of the data. `__restrict__`
// here is used to indicate the data should not
// alias against any other input parameters and 
// can sometimes produce important performance
// gains.
template<typename T>
struct slice1d
{
    int size;
    T* __restrict__ data;
    
    slice1d(int _size, T* _data) : size(_size), data(_data) {}

    slice1d& operator=(const slice1d<T>& rhs) { assert(rhs.size == size); memcpy(data, rhs.data, rhs.size * sizeof(T)); return *this; };
    
    slice1d<T> slice(int start, int stop) const { return slice1d<T>(stop - start, data + start); }
    slice1d<T> slice_from(int start) const { return slice1d<T>(size - start, data + start); }
    slice1d<T> slice(range r) const { return slice1d<T>(r.stop - r.start, data + r.start); }
    
    void zero() { memset((char*)data, 0, sizeof(T) * size); }
    void set(const T& x) { for (int i = 0; i < size; i++) { data[i] = x; } }
    
    inline T& operator()(int i) const { assert(i >= 0 && i < size); return data[i]; }
};

//--------------------------------------

// These types are used for the storage of arrays of data.
// They implicitly cast to slices so can be given directly 
// as inputs to functions requiring them.
template<typename T>
struct array1d
{
    int size;
    T* data;
    
    array1d() : size(0), data(NULL) {}
    array1d(int _size) : array1d() { resize(_size);  }
    array1d(const slice1d<T>& rhs) : array1d() { resize(rhs.size); memcpy(data, rhs.data, rhs.size * sizeof(T)); }
    array1d(const array1d<T>& rhs) : array1d() { resize(rhs.size); memcpy(data, rhs.data, rhs.size * sizeof(T)); }
    ~array1d() { resize(0); }
    
    array1d& operator=(const slice1d<T>& rhs) { resize(rhs.size); memcpy(data, rhs.data, rhs.size * sizeof(T)); return *this; };
    array1d& operator=(const array1d<T>& rhs) { resize(rhs.size); memcpy(data, rhs.data, rhs.size * sizeof(T)); return *this; };

    inline T& operator()(int i) const { assert(i >= 0 && i < size); return data[i]; }
    operator slice1d<T>() const { return slice1d<T>(size, data); }
    slice1d<T> slice(int start, int stop) const { return slice1d<T>(stop - start, data + start); }
    slice1d<T> slice_from(int start) const { return slice1d<T>(size - start, data + start); }
    slice1d<T> slice(range r) const { return slice1d<T>(r.stop - r.start, data + r.start); }
    
    void zero() { memset(data, 0, sizeof(T) * size); }
    void set(const T& x) { for (int i = 0; i < size; i++) { data[i] = x; } }
    
    void resize(int _size)
    {
        if (_size == 0 && size != 0)
        {
            free(data);
            data = NULL;
            size = 0;
        }
        else if (_size > 0 && size == 0)
        {
            data = (T*)malloc(_size * sizeof(T));
            assert(data != NULL);
            size = _size;
        }
        else if (_size > 0 && size > 0 && _size != size)
        {
            data = (T*)realloc(data, _size * sizeof(T));
            assert(data != NULL);           
            size = _size;
        }
    }
};

// These types are used for the storage of arrays of data.
// They implicitly cast to slices so can be given directly 
// as inputs to functions requiring them.
template<typename T, int N>
struct inplace_array1d
{
    int size;
    T* data;
    T buff[N];
    
    inplace_array1d() : size(0), data(NULL) {}
    inplace_array1d(int _size) : inplace_array1d() { resize(_size);  }
    inplace_array1d(const slice1d<T>& rhs) : inplace_array1d() { resize(rhs.size); memcpy(data, rhs.data, rhs.size * sizeof(T)); }
    inplace_array1d(const inplace_array1d<T,N>& rhs) : inplace_array1d() { resize(rhs.size); memcpy(data, rhs.data, rhs.size * sizeof(T)); }
    ~inplace_array1d() { resize(0); }
    
    inplace_array1d& operator=(const slice1d<T>& rhs) { resize(rhs.size); memcpy(data, rhs.data, rhs.size * sizeof(T)); return *this; };
    inplace_array1d& operator=(const inplace_array1d<T,N>& rhs) { resize(rhs.size); memcpy(data, rhs.data, rhs.size * sizeof(T)); return *this; };

    inline T& operator()(int i) const { assert(i >= 0 && i < size); return data[i]; }
    operator slice1d<T>() const { return slice1d<T>(size, data); }
    slice1d<T> slice(int start, int stop) const { return slice1d<T>(stop - start, data + start); }
    slice1d<T> slice_from(int start) const { return slice1d<T>(size - start, data + start); }
    slice1d<T> slice(range r) const { return slice1d<T>(r.stop - r.start, data + r.start); }
    
    void zero() { memset(data, 0, sizeof(T) * size); }
    void set(const T& x) { for (int i = 0; i < size; i++) { data[i] = x; } }
    
    void resize(int _size)
    {
        if (_size == 0 && size != 0)
        {
            if (size > N)
            {
                free(data);
            }
            
            data = NULL;
            size = 0;
        }
        else if (_size > 0 && size == 0)
        {   
            if (_size > N)
            {
                data = (T*)malloc(_size * sizeof(T));
                assert(data != NULL);
            }
            else
            {
                data = buff;
            }
            
            size = _size;
        }
        else if (_size > 0 && size > 0 && _size != size)
        { 
            if (_size > N && size > N)
            {
                data = (T*)realloc(data, _size * sizeof(T));
                assert(data != NULL);   
            }
            else if (_size > N && size <= N)
            {
                data = (T*)malloc(_size * sizeof(T));
                assert(data != NULL);
                memcpy(data, buff, size * sizeof(T));
            }
            else if (size > N && _size <= N)
            {
                memcpy(buff, data, _size * sizeof(T));
                free(data);
                data = buff;
            }
        
            size = _size;
        }
    }
};


//--------------------------------------

static inline int bit_alloc_size(int size)
{
    return ((size + 8 - 1) / 8) * 8;
}

static inline bool bit_get(const unsigned char* __restrict__ data, int i)
{
    return (data[i / 8] >> (i % 8)) & 1;
}

static inline void bit_set(unsigned char* __restrict__ data, int i, bool v)
{
    data[i / 8] ^= (-v ^ data[i / 8]) & (1 << (i % 8));    
}

struct slice1d_bit
{
    int size;
    int offset;
    unsigned char* __restrict__ data;
    
    slice1d_bit(int _size, int _offset, unsigned char* _data) : size(_size), offset(_offset), data(_data) {}

    slice1d_bit& operator=(const slice1d_bit& rhs) { assert(size == rhs.size); for (int i = 0; i < size; i++) { set(i, rhs.get(i)); } return *this; };
    
    inline bool get(int i) const { assert(i >= 0 && i < size); return bit_get(data, i + offset); }
    inline void set(int i, bool v) { assert(i >= 0 && i < size); bit_set(data, i + offset, v); }
    
    slice1d_bit slice(int start, int stop) const { return slice1d_bit(stop - start, offset + start % 8, data + start / 8); }
    slice1d_bit slice_from(int start) const { return slice1d_bit(size - start, offset + start % 8, data + start / 8); }
    slice1d_bit slice(range r) const { return slice1d_bit(r.stop - r.start, offset + r.start % 8, data + r.start / 8); }
    
    void zero() { for (int i = 0; i < size; i++) { set(i, false); } }
    void one() { for (int i = 0; i < size; i++) { set(i, true); } }
};

struct array1d_bit
{
    int size;
    unsigned char* data;
    
    array1d_bit() : size(0), data(NULL) {}
    array1d_bit(int _size) : array1d_bit() { resize(_size);  }
    array1d_bit(const slice1d_bit& rhs) : array1d_bit() { resize(rhs.size); for (int i = 0; i < size; i++) { set(i, rhs.get(i)); } }
    array1d_bit(const array1d_bit& rhs) : array1d_bit() { resize(rhs.size); for (int i = 0; i < size; i++) { set(i, rhs.get(i)); } }
    ~array1d_bit() { resize(0); }
    
    array1d_bit& operator=(const slice1d_bit& rhs) { resize(rhs.size); for (int i = 0; i < size; i++) { set(i, rhs.get(i)); } return *this; };
    array1d_bit& operator=(const array1d_bit& rhs) { resize(rhs.size); for (int i = 0; i < size; i++) { set(i, rhs.get(i)); } return *this; };

    inline bool get(int i) const { assert(i >= 0 && i < size); return bit_get(data, i); }
    inline void set(int i, bool v) { assert(i >= 0 && i < size); bit_set(data, i, v); }
    
    slice1d_bit slice(int start, int stop) const { return slice1d_bit(stop - start, start % 8, data + start / 8); }
    slice1d_bit slice_from(int start) const { return slice1d_bit(size - start, start % 8, data + start / 8); }
    slice1d_bit slice(range r) const { return slice1d_bit(r.stop - r.start, r.start % 8, data + r.start / 8); }
    
    void zero() { for (int i = 0; i < size; i++) { set(i, false); } }
    void one() { for (int i = 0; i < size; i++) { set(i, true); } }
    
    void resize(int _size)
    {
        if (_size == 0 && size != 0)
        {
            free(data);
            data = NULL;
            size = 0;
        }
        else if (_size > 0 && size == 0)
        {
            data = (unsigned char*)malloc(bit_alloc_size(_size));
            assert(data != NULL);
            size = _size;
        }
        else if (_size > 0 && size > 0 && _size != size)
        {
            data = (unsigned char*)realloc(data, bit_alloc_size(_size));
            assert(data != NULL);           
            size = _size;
        }
    }
};


