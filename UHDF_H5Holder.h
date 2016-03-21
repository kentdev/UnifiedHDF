#ifndef UHDF_H5HOLDER_H
#define UHDF_H5HOLDER_H

#include "hdf5.h"
#include "UHDF_Types.h"

class UHDF_SpaceHolder
{
private:
    hid_t id;

public:
    UHDF_SpaceHolder(hid_t spaceId) :
        id (spaceId)
    {
        if (spaceId < 0)
            throw UHDF_Exception("Negative H5S ID received");
    }

    ~UHDF_SpaceHolder()
    {
        if (id >= 0)
            H5Sclose(id);
    }

    const hid_t get() const
    {
        return id;
    }
};

class UHDF_TypeHolder
{
private:
    hid_t id;

public:
    UHDF_TypeHolder(hid_t typeId) :
        id (typeId)
    {
        if (typeId < 0)
            throw UHDF_Exception("Negative H5T ID received");
    }

    ~UHDF_TypeHolder()
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
