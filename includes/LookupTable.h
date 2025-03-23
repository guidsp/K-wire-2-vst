#pragma once

template <typename T, size_t size>
struct LookupTable {
    LookupTable()
    {
        table = (T*)calloc(size, sizeof(T));

        if (table == nullptr) 
            throw std::bad_alloc();
    }

    ~LookupTable()
    {
        free(table);
    }

    // Index is normalised.
    T lookup(T index)
    {
        assert(index >= 0.0 && index <= 1.0);

        index *= (size - 1);
        const int baseIndex = int(index);

        const T& current = table[baseIndex];
        const T& next = table[(baseIndex + 1) % size];
        const T fraction = index - T(baseIndex);

        return std::lerp(current, next, fraction);
    }

    T* table = nullptr;
};