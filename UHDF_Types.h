#ifndef UHDF_TYPES_H
#define UHDF_TYPES_H

#include <stdexcept>
#include <stdint.h>
#include <map>

// HDF4
#include "hdf/mfhdf.h"

// HDF5
#include "hdf5.h"

class UHDF_Exception : public std::runtime_error
{
public:
    UHDF_Exception( const std::string &errMessage) : std::runtime_error(errMessage)
    {}
};

typedef enum
{
    UHDF_READONLY  // read-only is the only mode supported in this version
} UHDF_FileAccess;

typedef enum
{
    UHDF_HDF4,
    UHDF_HDF5
} UHDF_FileType;

typedef union
{
    hid_t h5id;
    int32 h4id;
} UHDF_Identifier;

typedef enum
{
    UINT8,
    INT8,
    UINT16,
    INT16,
    UINT32,
    INT32,
    UINT64,
    INT64,
    FLOAT32,
    FLOAT64
} UHDF_DataType;

static const std::map<UHDF_DataType, int> UHDFToHDF4Map = {
    {UINT8,    DFNT_UINT8},
    {INT8,     DFNT_INT8},
    {UINT16,   DFNT_UINT16},
    {INT16,    DFNT_INT16},
    {UINT32,   DFNT_UINT32},
    {INT32,    DFNT_INT32},
    {UINT64,   DFNT_UINT64},
    {INT64,    DFNT_INT64},
    {FLOAT32,  DFNT_FLOAT32},
    {FLOAT64,  DFNT_FLOAT64}
};

static const std::map<int, UHDF_DataType> HDF4ToUHDFMap = {
    {DFNT_CHAR,    INT8},
    {DFNT_UCHAR,   UINT8},
    {DFNT_UINT8,   UINT8},
    {DFNT_INT8,    INT8},
    {DFNT_UINT16,  UINT16},
    {DFNT_INT16,   INT16},
    {DFNT_UINT32,  UINT32},
    {DFNT_INT32,   INT32},
    {DFNT_UINT64,  UINT64},
    {DFNT_INT64,   INT64},
    {DFNT_FLOAT32, FLOAT32},
    {DFNT_FLOAT64, FLOAT64}
};

static const std::map<UHDF_DataType, hid_t> UHDFToHDF5Map = {
    {UINT8,    H5T_NATIVE_UINT8},
    {INT8,     H5T_NATIVE_INT8},
    {UINT16,   H5T_NATIVE_UINT16},
    {INT16,    H5T_NATIVE_INT16},
    {UINT32,   H5T_NATIVE_UINT32},
    {INT32,    H5T_NATIVE_INT32},
    {UINT64,   H5T_NATIVE_UINT64},
    {INT64,    H5T_NATIVE_INT64},
    {FLOAT32,  H5T_NATIVE_FLOAT},
    {FLOAT64,  H5T_NATIVE_DOUBLE}
};

static inline int UHDFTypeToH4( const UHDF_DataType &t)
{
    const auto &iter = UHDFToHDF4Map.find(t);
    if (iter == UHDFToHDF4Map.end())
        throw UHDF_Exception("Couldn't convert UHDF type to HDF4");
    return iter->second;
}

static inline UHDF_DataType H4TypeToUHDF( const int &t)
{
    const auto &iter = HDF4ToUHDFMap.find(t);
    if (iter == HDF4ToUHDFMap.end())
        throw UHDF_Exception("Couldn't convert HDF4 type to UHDF");
    return iter->second;
}

static inline hid_t UHDFTypeToH5( const UHDF_DataType &t)
{
    const auto &iter = UHDFToHDF5Map.find(t);
    if (iter == UHDFToHDF5Map.end())
        throw UHDF_Exception("Couldn't convert UHDF type to HDF4");
    return iter->second;
}

//--------------------------------

template<typename T>
static inline UHDF_DataType getUHDFType()
{
    throw UHDF_Exception("Unknown type");
}

template<>
inline UHDF_DataType getUHDFType<uint8_t>()
{
    return UINT8;
}

template<>
inline UHDF_DataType getUHDFType<int8_t>()
{
    return INT8;
}

template<>
inline UHDF_DataType getUHDFType<uint16_t>()
{
    return UINT16;
}

template<>
inline UHDF_DataType getUHDFType<int16_t>()
{
    return INT16;
}

template<>
inline UHDF_DataType getUHDFType<uint32_t>()
{
    return UINT32;
}

template<>
inline UHDF_DataType getUHDFType<int32_t>()
{
    return INT32;
}

template<>
inline UHDF_DataType getUHDFType<uint64_t>()
{
    return UINT64;
}

template<>
inline UHDF_DataType getUHDFType<int64_t>()
{
    return INT64;
}

template<>
inline UHDF_DataType getUHDFType<float>()
{
    return FLOAT32;
}

template<>
inline UHDF_DataType getUHDFType<double>()
{
    return FLOAT64;
}

//--------------------------------

template<typename T>
static inline int getH4Type()
{
    throw UHDF_Exception("Unknown type");
    return -1;
}

template<>
inline int getH4Type<uint8_t>()
{
    return DFNT_UINT8;
}

template<>
inline int getH4Type<int8_t>()
{
    return DFNT_INT8;
}

template<>
inline int getH4Type<uint16_t>()
{
    return DFNT_UINT16;
}

template<>
inline int getH4Type<int16_t>()
{
    return DFNT_INT16;
}

template<>
inline int getH4Type<uint32_t>()
{
    return DFNT_UINT32;
}

template<>
inline int getH4Type<int32_t>()
{
    return DFNT_INT32;
}

template<>
inline int getH4Type<uint64_t>()
{
    return DFNT_UINT64;
}

template<>
inline int getH4Type<int64_t>()
{
    return DFNT_INT64;
}

template<>
inline int getH4Type<float>()
{
    return DFNT_FLOAT32;
}

template<>
inline int getH4Type<double>()
{
    return DFNT_FLOAT64;
}

//--------------------------------

template<typename T>
static inline hid_t getH5Type()
{
    throw UHDF_Exception("Unknown type");
    return -1;
}

template<>
inline hid_t getH5Type<uint8_t>()
{
    return H5T_NATIVE_UINT8;
}

template<>
inline hid_t getH5Type<int8_t>()
{
    return H5T_NATIVE_INT8;
}

template<>
inline hid_t getH5Type<uint16_t>()
{
    return H5T_NATIVE_UINT16;
}

template<>
inline hid_t getH5Type<int16_t>()
{
    return H5T_NATIVE_INT16;
}

template<>
inline hid_t getH5Type<uint32_t>()
{
    return H5T_NATIVE_UINT32;
}

template<>
inline hid_t getH5Type<int32_t>()
{
    return H5T_NATIVE_INT32;
}

template<>
inline hid_t getH5Type<uint64_t>()
{
    return H5T_NATIVE_UINT64;
}

template<>
inline hid_t getH5Type<int64_t>()
{
    return H5T_NATIVE_INT64;
}

template<>
inline hid_t getH5Type<float>()
{
    return H5T_NATIVE_FLOAT;
}

template<>
inline hid_t getH5Type<double>()
{
    return H5T_NATIVE_DOUBLE;
}

//--------------------------------

static inline UHDF_DataType H5TypeToUHDF( const hid_t &t)
{
    const H5T_class_t H5class = H5Tget_class(t);
    const size_t size = H5Tget_size(t);

    switch(H5class)
    {
    case H5T_INTEGER:
    {
        const H5T_sign_t sign = H5Tget_sign(t);

        switch(size)
        {
        case 1:
            if (sign == H5T_SGN_NONE)
                return INT8;
            else
                return UINT8;
            break;
        case 2:
            if (sign == H5T_SGN_NONE)
                return INT16;
            else
                return UINT16;
            break;
        case 4:
            if (sign == H5T_SGN_NONE)
                return INT32;
            else
                return UINT32;
            break;
        case 8:
            if (sign == H5T_SGN_NONE)
                return INT64;
            else
                return UINT64;
            break;
        default:
            throw UHDF_Exception("Couldn't convert HDF5 type to UHDF");
        }

        break;
    }
    case H5T_FLOAT:
    {
        switch(size)
        {
        case 4:
            return FLOAT32;
        case 8:
            return FLOAT64;
        default:
            throw UHDF_Exception("Couldn't convert HDF5 type to UHDF");
        }

        break;
    }
    default:
        throw UHDF_Exception("Couldn't convert HDF5 type to UHDF");
    }
}

#endif
