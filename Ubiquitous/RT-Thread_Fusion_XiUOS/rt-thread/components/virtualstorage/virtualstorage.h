#ifndef __VIRTUALSTORAGE_H__
#define __VIRTUALSTORAGE_H__

#define Virtual_sector_size 512

#define Max_storage_size 64

#define start_sector 63UL
#define primary_index 0xFFFFFFFF
#define index_mask 0xF0000000
#define secnum_mask 0x0FFFFFFF

#define Magic_identity_number 0xABCDEF98

// typedef unsigned char BYTE;
enum dev_type{sd,udisk};

struct v_device_components{
    struct rt_device * dev;
    enum dev_type type;
    DWORD index;
    struct rt_device_blk_geometry geometry;
    DWORD next_allocated_sec_idx;
  
};

struct operation_pair{
    DWORD idx;
    DWORD real_sec_number;
}


struct virtual_storage_device
{
    rt_list_t list;
    u_int virtual_storage_size;
    struct rt_device dev;
    struct v_device_components sub_dev[10]; //all suboardinate device,maybe udisk,sd card
    int components_size;
    struct rt_device_blk_geometry geometry;
    rt_err_t  (*register_components)(rt_device_t dev,const char *name,rt_uint16_t flags,enum dev_type tp);
    // partition info
    // u_int primary_index;
    // u_int sub_index[10];
};

#endif