#ifndef UHDF_ATTRIBUTE_H
#define UHDF_ATTRIBUTE_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include "UHDF_Types.h"
#include "UHDF_H5Holder.h"

class UHDF_Attribute
{
    friend class UHDF_File;
    friend class UHDF_Group;
    friend class UHDF_Dataset;

public:
    ~UHDF_Attribute()
    {
        switch (fileType)
        {
        case UHDF_HDF4:
            // attributes aren't separate objects in HDF4, and don't need to be closed
            break;
        case UHDF_HDF5:
            if (id.h5id >= 0)
                H5Aclose(id.h5id);
            break;
        }
    }

    const std::string &getName() const
    {
        return attributename;
    }

    size_t getNumElements() const
    {
        return numElements;
    }

    UHDF_DataType getType() const
    {
        return datatype;
    }

    std::string readAsString() const
    {
        std::vector<char> data = read<char>();
        data.push_back(0);
        return std::string(data.data());
    }

    bool isString() const
    {
        return datatype == UHDF_STRING;
    }

    template<typename T>
    std::vector<T> read() const
    {
        std::vector<T> data;
        data.resize(numElements);

        switch(fileType)
        {
        case UHDF_HDF4:
        {
            T *buffer = data.data();

            const UHDF_DataType outputType = getUHDFType<T>();
            if (datatype != outputType)
            {  // conversion needed
                if (SDreadattr(owner.h4id, id.h4id, buffer) < 0)
                    throw UHDF_Exception("Error reading attribute '" + attributename + "'");
            }
            else
            {  // need to convert from the field's type to the return type
                switch(datatype)
                {
                case UHDF_UINT8:
                    convertH4<uint8, T>(buffer);
                    break;
                case UHDF_INT8:
                    convertH4<int8, T>(buffer);
                    break;
                case UHDF_UINT16:
                    convertH4<uint16, T>(buffer);
                    break;
                case UHDF_INT16:
                    convertH4<int16, T>(buffer);
                    break;
                case UHDF_UINT32:
                    convertH4<uint32, T>(buffer);
                    break;
                case UHDF_INT32:
                    convertH4<int32, T>(buffer);
                    break;
                case UHDF_FLOAT32:
                    convertH4<float, T>(buffer);
                    break;
                case UHDF_FLOAT64:
                    convertH4<double, T>(buffer);
                    break;
                default:
                    throw UHDF_Exception("Unsupported datatype when doing conversion in read of dataset '" + attributename + "'");
                }
            }
            break;
        }

        case UHDF_HDF5:
        {
            if (datatype == UHDF_REFERENCE)
            {
                data.clear();
            }
            else if (datatype == UHDF_STRING)
            {
                UHDF_TypeHolder type(H5Tcopy(H5T_C_S1));

                data.resize(numElements + 1);
                if (H5Tset_size(type.get(), numElements + 1) < 0)
                    throw UHDF_Exception("Error setting type size when reading string attribute '" + attributename + "'");
                if (H5Tset_strpad(type.get(), H5T_STR_NULLTERM) < 0)
                    throw UHDF_Exception("Error setting padding when reading string attribute '" + attributename + "'");
                if (H5Tset_cset(type.get(), H5T_CSET_ASCII) < 0)
                    throw UHDF_Exception("Error setting ASCII encoding when reading string attribute '" + attributename + "'");

                if (H5Aread(id.h5id, type.get(), data.data()) < 0)
                    throw UHDF_Exception("Error reading string attribute '" + attributename + "'");
            }
            else
            {
                if (H5Aread(id.h5id, getH5Type<T>(), data.data()) < 0)
                    throw UHDF_Exception("Error reading attribute '" + attributename + "'");
            }
            break;
        }
        }

        return data;
    }

private:
    UHDF_FileType fileType;
    UHDF_Identifier owner;
    UHDF_Identifier id;
    std::string attributename;
    UHDF_DataType datatype;
    int numElements;

    template <typename T>
    UHDF_Attribute (UDHF_FileType format, UHDF_Identifier ownerId, const std::string &attributeName, const size_t numElements, const T *const dataBuffer)
    {
        T dummy = 0;
        T* buffer = dataBuffer;

        // if 0-element attribute, create a fake 1st element to avoid an HDF4 bug
        size_t numElems = numElements;
        if (numElems == 0)
        {
            numElems = 1;
            buffer = &dummy;
        }

        fileType = format;
        owner = ownerId;
        attributename = attributeName;
        datatype = getUHDFType<T>();
        numElements = numElems;

        switch (fileType)
        {
        case UHDF_HDF4:
            if (SDsetattr(owner.h4id, attributename.c_str(), UHDFTypeToH4(datatype), numElems, buffer) < 0)
                throw UHDF_Exception("Error creating attribute '" + attributename + "'");

            id.h4id = SDfindattr(owner.h4id, attributename.c_str());
            if (id.h4id < 0)
                throw UHDF_Exception("Can't find newly-created attribute '" + attributename + "'");

            break;

        case UHDF_HDF5:
        {
            UHDF_TypeHolder type(H5Tcopy(UHDFTypeToH5(datatype)));
            std::unique_ptr<UHDF_SpaceHolder> space;

            if (isString())
            {
                // string attribute: space is 1, type is numElems-length string
                if (H5Tset_size(type.get(), numElements) < 0)
                    throw UHDF_Exception("Error setting size of attribute '" + attributename + "'");

                const hsize_t spaceElems = 1;
                space.reset(new UHDF_SpaceHolder(H5Screate_simple(1, &spaceElems, NULL)));
            }
            else
            {
                space.reset(new UHDF_SpaceHolder(H5Screate_simple(1, &elems, NULL)));
            }

            id.h5id = H5Acreate2(ownerId.h5id, attributename.c_str(), type.get(), space.get(), H5P_DEFAULT, H5P_DEFAULT);
            if (id.h5id < 0)
                throw UHDF_Exeception("Error creating attribute '" + attributename + "'");

            if (H5Awrite(id.h5id, type.get(), buffer) < 0)
                throw UHDF_Exception("Error writing data to newly-created attribute '" + attributename + "'");

            break;
        }
        }
    }

    UHDF_Attribute (UHDF_FileType format, UHDF_Identifier ownerId, const std::string &attributeName)
    {
        fileType = type;
        attributename = attributeName;
        owner = ownerId;

        switch(format)
        {
        case UHDF_HDF4:
        {
            id.h4id = SDfindattr(owner.h4id, attributename.c_str());
            if (id.h4id < 0)
                throw UHDF_Exception("Can't find attribute named '" + attributename + "'");

            char dummyName[MAX_NC_NAME + 1];
            int32 iType;
            int32 iCount;

            if (SDattrinfo(ownerId.h4id, id.h4id, dummyName, &iType, &iCount) < 0)
                throw UHDF_Exception("Can't open attribute '" + attributename + "'");

            datatype = H4TypeToUHDF(iType);
            numElements = iCount;

            break;
        }
        case UHDF_HDF5:
        {
            id.h5id = H5Aopen(ownerId.h5id, attributeName.c_str(), H5P_DEFAULT);
            if (id.h5id < 0)
                throw UHDF_Exception("Can't open attribute '" + attributename + "'");

            const UHDF_SpaceHolder space(H5Aget_space(id.h5id));
            const UHDF_TypeHolder type(H5Aget_type(id.h5id));

            datatype = H5TypeToUHDF(type.get());

            if (datatype == UHDF_REFERENCE)
                numElements = 1;
            else if (datatype == UHDF_STRING)
                numElements = H5Tget_size(type.get());
            else
                numElements = H5Sget_simple_extent_npoints(space.get());

            if (numElements < 0)
                throw UHDF_Exception("Error getting number of elements in attribute '" + attributename + "'");

            break;
        }
        }
    }

    template<typename FILE_T, typename MEM_T>
    void convertH4 (MEM_T* buffer) const
    {
        std::unique_ptr<FILE_T[]> unconverted(new FILE_T[numElements]);

        if (SDreadattr(owner.h4id, id.h4id, unconverted.get()) < 0)
            throw UHDF_Exception("Error reading attribute '" + attributename + "'");

        for (size_t i = 0; i < numElements; i++)
            buffer[i] = static_cast<MEM_T>(unconverted[i]);
    }
};

#endif // UHDF_ATTRIBUTE_H
