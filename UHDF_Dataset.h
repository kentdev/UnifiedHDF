#ifndef UHDF_DATASET_H
#define UHDF_DATASET_H

#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>

#include "UHDF_Types.h"

class UHDF_Dataset
{
    friend class UHDF_File;

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


private:
    UHDF_FileType fileType;
    UHDF_Identifier id;
    UHDF_DataType dataType;
    std::string datasetname;

    std::vector<size_t> dimensions;

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
            int32 sdsNumAttrs;

            if (SDgetinfo( id.h4id, NULL, &sdsRank, sdsDimSizes, &sdsType, &sdsNumAttrs) < 0)
                throw UHDF_Exception("Error getting dataset info");

            for (intn i = 0; i < sdsRank; i++)
            {
                const int32 dimId = SDgetdimid(id.h4id, i);
                if (dimId < 0)
                    throw UHDF_Exception("Error getting dimension information for dimension " + boost::lexical_cast<std::string>(i));

                dimensions.push_back(sdsDimSizes[i]);
            }

            dataType = H4TypeToUHDF(sdsType);

            break;
        }
        case UHDF_HDF5:
        {
            id.h5id = H5Dopen2(ownerId.h5id, datasetname.c_str(), H5P_DEFAULT);
            if (id.h5id < 0)
                throw UHDF_Exception("Couldn't open dataset name '" + datasetname + "'");

            {
                const SpaceHolder spaceId(H5Dget_space(id.h5id));
                if (spaceId.get() < 0)
                    throw UHDF_Exception("Error getting dataset info (couldn't get dataspace)");

                const int rank = H5Sget_simple_extent_ndims(spaceId);
                if (rank < 0)
                    throw UHDF_Exception("Error getting dataset info (couldn't get rank)");

                if (rank > 0)
                {
                    std::unique_ptr<hsize_t[]> dims(new hsize_t[rank]);
                    std::unique_ptr<hsize_t[]> maxDims(new hsize_t[rank]);

                    if (H5Sget_simple_extent_dims(spaceId, dims.get(), maxDims.get()) < 0)
                        throw UHDF_Exception("Error getting dataset info (couldn't get dimensions)");

                    for (int i = 0; i < rank; i++)
                        dimensions.push_back(dims[i]);
                }
            }

            {
                const TypeHolder h5Type(H5Dget_type(id.h5id));
                dataType = H5TypeToUHDF(h5Type.get());
            }

            break;
        }
        }
    }
};

#endif
