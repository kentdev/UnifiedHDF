#ifndef UHDF_ATTRIBUTE_H
#define UHDF_ATTRIBUTE_H

#include <string>
#include <vector>
#include <memory>

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
        data.push(0);
        return std::string(data.get());
    }

    bool isString() const
    {
        if (datatype != UHDF_INT8)
            return false;

        switch(filetype)
        {
        case UHDF_HDF4:
            return true;  // assume all 8-bit signed-type attributes are character arrays
        case UHDF_HDF5:
        {
            const UHDF_TypeHolder type(H5Aget_type(id.h5id));
            return (H5Tget_class(type.get()) == H5T_STRING);
        }
        }

        return false;
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
                case UINT8:
                    convertH4<uint8, T>(buffer);
                    break;
                case INT8:
                    convertH4<int8, T>(buffer);
                    break;
                case UINT16:
                    convertH4<uint16, T>(buffer);
                    break;
                case INT16:
                    convertH4<int16, T>(buffer);
                    break;
                case UINT32:
                    convertH4<uint32, T>(buffer);
                    break;
                case INT32:
                    convertH4<int32, T>(buffer);
                    break;
                case FLOAT32:
                    convertH4<float, T>(buffer);
                    break;
                case FLOAT64:
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
            if (H5Aread(id.h5id, getH5Type<T>(), data.data()) < 0)
                throw UHDF_Exception("Error reading attribute '" + attributename + "'");
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
    size_t numElements;

    UHDF_Attribute (UHDF_FileType type, UHDF_Identifier ownerId, const std::string &attributeName)
    {
        fileType = type;
        attributename = attributeName;
        owner = ownerId;

        switch(type)
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

            datatype =

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
