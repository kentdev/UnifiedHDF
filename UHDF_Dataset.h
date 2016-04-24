#ifndef UHDF_DATASET_H
#define UHDF_DATASET_H

#include <string>
#include <array>

#include <boost/lexical_cast.hpp>
#include <boost/multi_array.hpp>

#include "UHDF_Types.h"
#include "UHDF_H5Holder.h"
#include "UHDF_Interfaces.h"

class UHDF_Dataset// : public UHDF_AttributeHolder
{
    friend class UHDF_File;
    friend class UHDF_Group;

public:
    ~UHDF_Dataset()
    {
        switch(fileType)
        {
        case UHDF_HDF4:
            if (id.h4id >= 0)
                SDendaccess(id.h4id);
            break;
        case UHDF_HDF5:
            if (id.h5id >= 0)
                H5Dclose(id.h5id);
            break;
        }
    }

    const std::string &getName() const
    {
        return datasetname;
    }

    const std::vector<size_t> &getDimensions() const
    {
        return dimensions;
    }

    const size_t getRank() const
    {
        return dimensions.size();
    }

    const size_t getNumElements() const
    {
        size_t elems = 1;
        for (auto n : dimensions)
        {
            elems *= n;
        }
        return elems;
    }

    const UHDF_DataType getType() const
    {
        return dataType;
    }

    void rawRead( const int32 *const start,
                  const int32 *const stride,
                  const int32 *const count,
                  void *buffer) const
    {
        if (dataType == UHDF_UNKNOWN)
        {
            throw UHDF_Exception("Can't read: unknown/unsupported datatype");
        }

        switch(fileType)
        {
        case UHDF_HDF4:
        {
            if (SDreaddata(id.h4id, const_cast<int32*>(start), const_cast<int32*>(stride), const_cast<int32*>(count), buffer) < 0)
                throw UHDF_Exception("Error reading HDF4 dataset '" + datasetname + "'");
            break;
        }
        case UHDF_HDF5:
        {
            const UHDF_SpaceHolder fileSpaceId(H5Scopy(H5Dget_space(id.h5id)));

            if (sizeof(hsize_t) == sizeof(int32))
            {
                H5Sselect_hyperslab(fileSpaceId.get(), H5S_SELECT_SET, (hsize_t*)start, (hsize_t*)stride, (hsize_t*)count, NULL);
            }
            else
            {
                std::unique_ptr<hsize_t[]> hstart(new hsize_t[rank]);
                std::unique_ptr<hsize_t[]> hstride(new hsize_t[rank]);
                std::unique_ptr<hsize_t[]> hcount(new hsize_t[rank]);
                for (size_t i = 0; i < rank; i++)
                {
                    hstart[i] = start[i];
                    hstride[i] = stride[i];
                    hcount[i] = count[i];
                }
                H5Sselect_hyperslab(fileSpaceId.get(), H5S_SELECT_SET, hstart.get(), hstride.get(), hcount.get(), NULL);
            }

            if (H5Dread(id.h5id, UHDFTypeToH5(dataType), H5S_ALL, fileSpaceId.get(), H5P_DEFAULT, buffer) < 0)
                throw UHDF_Exception("Error reading HDF5 dataset '" + datasetname + "'");
            break;
        }
        }
    }

    void rawRead( const int32 *const start,
                  const int32 *const count,
                  void *buffer) const
    {
        std::unique_ptr<int32[]> stride(new int32[rank]);
        for (size_t i = 0; i < rank; i++)
            stride[i] = 1;

        rawRead( start, stride.get(), count, buffer);
    }

    template<typename T>
    void read( const int32 *const start,
               const int32 *const stride,
               const int32 *const count,
               T* buffer) const
    {
        if (dataType == UHDF_UNKNOWN)
        {
            throw UHDF_Exception("Can't read: unknown/unsupported datatype");
        }

        switch(fileType)
        {
        case UHDF_HDF4:
        {
            const UHDF_DataType outputType = getUHDFType<T>();
            if (dataType == outputType)
            {  // no conversion needed
                rawRead(start, stride, count, buffer);
            }
            else
            {  // need to convert from the field's type to the return type
                switch(dataType)
                {
                case UHDF_UINT8:
                    convertH4<uint8, T>(start, stride, count, buffer);
                    break;
                case UHDF_INT8:
                    convertH4<int8, T>(start, stride, count, buffer);
                    break;
                case UHDF_UINT16:
                    convertH4<uint16, T>(start, stride, count, buffer);
                    break;
                case UHDF_INT16:
                    convertH4<int16, T>(start, stride, count, buffer);
                    break;
                case UHDF_UINT32:
                    convertH4<uint32, T>(start, stride, count, buffer);
                    break;
                case UHDF_INT32:
                    convertH4<int32, T>(start, stride, count, buffer);
                    break;
                case UHDF_FLOAT32:
                    convertH4<float, T>(start, stride, count, buffer);
                    break;
                case UHDF_FLOAT64:
                    convertH4<double, T>(start, stride, count, buffer);
                    break;
                default:
                    throw UHDF_Exception("Unsupported datatype when doing conversion in read of dataset '" + datasetname + "'");
                }
            }
            break;
        }
        case UHDF_HDF5:
        {
            const UHDF_SpaceHolder fileSpaceId(H5Scopy(H5Dget_space(id.h5id)));

            if (sizeof(hsize_t) == sizeof(int32))
            {
                H5Sselect_hyperslab(fileSpaceId.get(), H5S_SELECT_SET, (hsize_t*)start, (hsize_t*)stride, (hsize_t*)count, NULL);
            }
            else
            {
                std::unique_ptr<hsize_t[]> hstart(new hsize_t[rank]);
                std::unique_ptr<hsize_t[]> hstride(new hsize_t[rank]);
                std::unique_ptr<hsize_t[]> hcount(new hsize_t[rank]);
                for (size_t i = 0; i < rank; i++)
                {
                    hstart[i] = start[i];
                    hstride[i] = stride[i];
                    hcount[i] = count[i];
                }
                H5Sselect_hyperslab(fileSpaceId.get(), H5S_SELECT_SET, hstart.get(), hstride.get(), hcount.get(), NULL);
            }

            if (H5Dread(id.h5id, getH5Type<T>(), H5S_ALL, fileSpaceId.get(), H5P_DEFAULT, buffer) < 0)
                throw UHDF_Exception("Error reading HDF5 dataset '" + datasetname + "'");
            break;
        }
        }
    }

    template <typename T>
    void read( const int32 *const start,
               const int32 *const count,
               T* buffer) const
    {
        std::unique_ptr<int32[]> stride(new int32[rank]);
        for (size_t i = 0; i < rank; i++)
            stride[i] = 1;

        read (start, stride, count, buffer);
    }

    template <typename T, size_t DIMS>
    boost::multi_array<T, DIMS> read( const std::array<int32, DIMS> &start,
                                      const std::array<int32, DIMS> &stride,
                                      const std::array<int32, DIMS> &count) const
    {
        if (dimensions.size() != DIMS)
            throw UHDF_Exception("When reading, provided dimensions don't match dataset rank");

        boost::multi_array<T, DIMS> output (boost::extents(count));

        read( start.get(), stride.get(), count.get(), output.data());

        return output;
    }

    template <typename T, size_t DIMS>
    boost::multi_array<T, DIMS> read( const std::array<int32, DIMS> &start,
                                      const std::array<int32, DIMS> &count)
    {
        std::array<int32, DIMS> stride;
        stride.fill(1);

        return read(start, stride, count);
    }

    template <typename T>
    std::vector<T> readAll() const
    {
        std::vector<T> buffer;

        buffer.resize(getNumElements(), 0);
        std::unique_ptr<int32[]> start( new int32[rank]);
        std::unique_ptr<int32[]> stride( new int32[rank]);
        std::unique_ptr<int32[]> count( new int32[rank]);

        for (size_t i = 0; i < rank; i++)
        {
            start[i] = 0;
            stride[i] = 1;
            count[i] = dimensions[i];
        }

        read (start.get(), stride.get(), count.get(), buffer.data());
        return buffer;
    }

    std::list<std::string> getAttributeNames() const
    {
        std::list<std::string> names;

        switch(fileType)
        {
        case UHDF_HDF4:
        {
            for (int32 i = 0; i < h4NumAttrs; i++)
            {
                char name[MAX_NC_NAME + 1];
                int32 attType;
                int32 attCount;

                memset(name, 0, MAX_NC_NAME + 1);
                if (SDattrinfo(id.h4id, i, name, &attType, &attCount) < 0)
                    throw UHDF_Exception("Error getting attribute name from dataset '" + datasetname + "'");

                names.push_back(std::string(name));
            }
            break;
        }
        case UHDF_HDF5:
        {
            const int numAttrs = H5Aget_num_attrs(id.h5id);
            if (numAttrs < 0)
                throw UHDF_Exception("Error retrieving the number of attributes in dataset '" + datasetname + "'");

            for (int i = 0; i < numAttrs; i++)
            {
                const ssize_t nameLength = H5Aget_name_by_idx(id.h5id, ".", H5_INDEX_NAME, H5_ITER_NATIVE, i, NULL, 0, H5P_DEFAULT);
                if (nameLength < 0)
                    throw UHDF_Exception("Error getting name of attribute " + boost::lexical_cast<std::string>(i) + " of dataset '" + datasetname + "'");
                if (nameLength == 0)
                    continue;

                std::unique_ptr<char[]> name(new char[nameLength + 1]);
                memset(name.get(), 0, nameLength + 1);

                if (H5Aget_name_by_idx(id.h5id, ".", H5_INDEX_NAME, H5_ITER_NATIVE, i, name.get(), nameLength + 1, H5P_DEFAULT) < 0)
                    throw UHDF_Exception("Error getting name of attribute " + boost::lexical_cast<std::string>(i) + " of dataset '" + datasetname + "'");

                names.push_back(std::string(name.get()));
            }
            break;
        }
        }

        return names;
    }

    UHDF_Attribute openAttribute(const std::string &attributeName) const
    {
        return UHDF_Attribute(fileType, id, attributeName);
    }

private:
    UHDF_FileType fileType;
    UHDF_Identifier id;
    UHDF_DataType dataType;
    std::string datasetname;
    int rank;
    std::vector<size_t> dimensions;
    int32 h4NumAttrs;

    UHDF_Dataset( UHDF_FileType format, UHDF_Identifier ownerId, const std::string &datasetName)
    {
        fileType = format;
        datasetname = datasetName;

        switch(fileType)
        {
        case UHDF_HDF4:
        {
            int32 ix = SDnametoindex(ownerId.h4id, datasetname.c_str());
            if (ix < 0)
                throw UHDF_Exception("Couldn't find dataset named '" + datasetname + "'");

            id.h4id = SDselect( ownerId.h4id, ix);
            if (id.h4id < 0)
                throw UHDF_Exception("Couldn't open dataset named '" + datasetname + "'");

            int32 sdsRank;
            int32 sdsDimSizes[MAX_VAR_DIMS];
            int32 sdsType;

            if (SDgetinfo( id.h4id, NULL, &sdsRank, sdsDimSizes, &sdsType, &h4NumAttrs) < 0)
                throw UHDF_Exception("Error getting dataset info");

            for (intn i = 0; i < sdsRank; i++)
            {
                const int32 dimId = SDgetdimid(id.h4id, i);
                if (dimId < 0)
                    throw UHDF_Exception("Error getting dimension information for dimension " + boost::lexical_cast<std::string>(i));

                dimensions.push_back(sdsDimSizes[i]);
            }
            rank = static_cast<size_t>(sdsRank);

            try
            {
                dataType = H4TypeToUHDF(sdsType);
            }
            catch (UHDF_Exception &)
            {
                dataType = UHDF_UNKNOWN;
            }

            break;
        }
        case UHDF_HDF5:
        {
            id.h5id = H5Dopen2(ownerId.h5id, datasetname.c_str(), H5P_DEFAULT);
            if (id.h5id < 0)
                throw UHDF_Exception("Couldn't open dataset name '" + datasetname + "'");

            const UHDF_SpaceHolder spaceId(H5Dget_space(id.h5id));
            if (spaceId.get() < 0)
                throw UHDF_Exception("Error getting dataset info (couldn't get dataspace)");

            rank = H5Sget_simple_extent_ndims(spaceId.get());
            if (rank < 0)
                throw UHDF_Exception("Error getting dataset info (couldn't get rank)");

            if (rank > 0)
            {
                std::unique_ptr<hsize_t[]> dims(new hsize_t[rank]);
                std::unique_ptr<hsize_t[]> maxDims(new hsize_t[rank]);

                if (H5Sget_simple_extent_dims(spaceId.get(), dims.get(), maxDims.get()) < 0)
                    throw UHDF_Exception("Error getting dataset info (couldn't get dimensions)");

                for (int i = 0; i < rank; i++)
                    dimensions.push_back(dims[i]);
            }

            const UHDF_TypeHolder h5Type(H5Dget_type(id.h5id));

            try
            {
                dataType = H5TypeToUHDF(h5Type.get());
            }
            catch (UHDF_Exception &)
            {
                dataType = UHDF_UNKNOWN;
            }

            break;
        }
        }
    }

    template<typename FILE_T, typename MEM_T>
    void convertH4 (const int32 *const start,
                    const int32 *const stride,
                    const int32 *const count,
                    MEM_T* buffer) const
    {
        size_t numSelectedElements = 1;
        for (size_t i = 0; i < rank; i++)
        {
            if (count[i] <= 0)
                throw UHDF_Exception("Zero or negative count given when reading");

            numSelectedElements *= count[i];
        }

        std::unique_ptr<FILE_T[]> unconverted(new FILE_T[numSelectedElements]);

        rawRead(start, stride, count, unconverted.get());

        for (size_t i = 0; i < numSelectedElements; i++)
            buffer[i] = static_cast<MEM_T>(unconverted[i]);
    }
};

#endif
