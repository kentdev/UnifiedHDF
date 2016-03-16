#ifndef UHDF_H5HOLDER_H
#define UHDF_H5HOLDER_H

#include "hdf5.h"

class SpaceHolder
{
private:
    hid_t id;

public:
    SpaceHolder(hid_t spaceId) :
        id (spaceId)
    {}

    ~SpaceHolder()
    {
        if (id >= 0)
            H5Sclose(id);
    }

    const hid_t get() const
    {
        return id;
    }
};

class TypeHolder
{
private:
    hid_t id;

public:
    TypeHolder(hid_t typeId) :
        id (typeId)
    {}

    ~TypeHolder()
    {
        if (id >= 0)
            H5Tclose(id);
    }

    const hid_t get() const
    {
        return id;
    }
};


#endif // UHDF_H5HOLDER_H
