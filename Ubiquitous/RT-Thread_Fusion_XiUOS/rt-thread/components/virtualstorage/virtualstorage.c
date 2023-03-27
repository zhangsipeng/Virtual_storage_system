#include <rtthread.h>
#include <dfs_fs.h>
#include "virtualstorage.h"
#include "../dfs/filesystems/elmfat/ff.h"




static struct virtual_storage_device * virtual_dev=RT_NULL;

void register_virtual_device(){
    virtual_dev = rt_calloc(1, sizeof(struct virtual_storage_device));
    virtual_dev->virtual_storage_size=Max_storage_size;
    virtual_dev->components_size=0;
    virtual_dev->dev.user_data=virtual_dev;
    virtual_dev->register_components=rt_vs_register_components;
    //set geometry
    virtual_dev->geometry.bytes_per_sector=Virtual_sector_size;
    virtual_dev->geometry.sector_count=1000;
    virtual_dev->geometry.block_size=1;
    //
    //bind function
    virtual_dev->dev.init=rt_vs_init;
    virtual_dev->dev.close=rt_vs_close;
    virtual_dev->dev.open=rt_vs_open;
    virtual_dev->dev.control=rt_vs_control;
    virtual_dev->dev.read=rt_vs_read;
    virtual_dev->dev.write=rt_vs_write;
    //
    rt_device_register(&virtual_dev->dev, "virtual_storage",
                    RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_REMOVABLE | RT_DEVICE_FLAG_STANDALONE);
}

static rt_err_t rt_vs_register_components(rt_device_t dev,const char *name,rt_uint16_t flags,enum dev_type tp){
    dev->flag=flags;
    int o_size=virtual_dev->components_size;
    virtual_dev->components_size+=1;
    virtual_dev->sub_dev[o_size].dev=dev;
    virtual_dev->sub_dev[o_size].type=tp;
    virtual_dev->sub_dev[o_size].index=0;
    //if o_size==0, initiate the mapping
    return RT_EOK;
}

static rt_err_t rt_vs_init(rt_device_t dev)
{
    struct virtual_storage_device *v_dev = (struct virtual_storage_device *)(dev->user_data);
    boolean primary=FALSE;
    BYTE * buf = rt_malloc(4096);
    DWORD max_index=0;
    for (u_int i=0;i<v_dev->components_size;i++){
        DWORD identifier,index,next_allocated;
        rt_device_t tmp=v_dev->sub_dev[i].dev; 
        tmp->read(tmp,start_sector,buf,1);
        identifier=ld_dword(buf);
        if (identifier!=Magic_identity_number){
            //err
            continue;
        }
        index=ld_dword(buf+4);
        v_dev->sub_dev[i].index=index;

        next_allocated=ld_dword(buf+8);
        v_dev->sub_dev[i].next_allocated_sec_idx=next_allocated;
        if (index==primary_index) primary=TRUE;
        //init geometry TODO?

    }
    
    for (u_int i=0;i<v_dev->components_size;i++){
        if (!primary){
            if (v_dev->sub_dev[i].index!=0) continue;
            v_dev->sub_dev[i].index=primary_index;
            primary=TRUE;
            DWORD allocate_table_offset=(v_dev->virtual_storage_size)*16*1024;
            v_dev->sub_dev[i].next_allocated_sec_idx=start_sector+1+allocate_table_offset;
            

            st_dword(buf,Magic_identity_number);
            st_dword(buf+4,primary_index);
            st_dword(buf+8,v_dev->sub_dev[i].next_allocated_sec_idx);
           
            //TODO:check return status and debug;
             v_dev->sub_dev[i].dev->write(v_dev->sub_dev[i].dev,start_sector,buf,1);
            break;
        }
        else {
            if (v_dev->sub_dev[i].index!=0) continue;
            v_dev->sub_dev[i].index=max_index+1;
            max_index+=1;
            v_dev->sub_dev[i].next_allocated_sec_idx=start_sector+1;
            st_dword(buf,Magic_identity_number);
            st_dword(buf+4,max_index);
            st_dword(buf+8,v_dev->sub_dev[i].next_allocated_sec_idx);
        }
    }
    rt_free(buf);

    
    return RT_EOK;
}

static rt_err_t rt_vs_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t rt_vs_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t rt_vs_control(rt_device_t dev, int cmd, void *args)
{
    struct virtual_storage_device *v_dev = (struct virtual_storage_device *)(dev->user_data);
    switch (cmd)
    {
    case RT_DEVICE_CTRL_BLK_GETGEOME:
        rt_memcpy(args, &v_dev->geometry, sizeof(struct rt_device_blk_geometry));
        break;
    default:
        break;
    }
    return RT_EOK;
}

struct operation_pair get_mapping( struct virtual_storage_device * dev,rt_off_t pos){
    struct operation_pair ret;
    BYTE * buf = rt_malloc(4096);
    for (u_int i=0;i<dev->components_size;i++){
        if (dev->sub_dev[i].index==primary_index){
            u_int r=pos%128;
            r*=4;
            u_int q=(pos-r)/128;
            dev->sub_dev[i].dev->read(dev->sub_dev[i].dev,start_sector+q+1,buf,1);
            DWORD true_sec_number=ld_dword(buf+r);
            if (true_sec_number==0){
                //
            }
            DWORD idx=true_sec_number&index_mask;
            idx=idx>>28;
            DWORD sec_num=true_sec_number&secnum_mask;
            ret.idx=idx;
            ret.real_sec_number=sec_num;
            
        }
    }
    return ret;
}
static rt_size_t rt_vs_read(rt_device_t dev,
                               rt_off_t    pos,
                               void       *buffer,
                               rt_size_t   size)
{
     struct virtual_storage_device *v_dev = (struct virtual_storage_device *)(dev->user_data);
     struct operation_pair pr=get_mapping( v_dev, pos);
     for (u_int i=0;i<v_dev->components_size;i++){
        if (v_dev->sub_dev[i].index==pr.idx){
            return v_dev->sub_dev[i].dev->read(v_dev->sub_dev[i].dev,pr.real_sec_number,buffer,size);
        }
     }
     //err invalid sec_num
}

static rt_size_t rt_vs_write(rt_device_t dev,
                               rt_off_t    pos,
                               void       *buffer,
                               rt_size_t   size)
{
}

static WORD ld_word (const BYTE* ptr)	/*	 Load a 2-byte little-endian word */
{
	WORD rv;

	rv = ptr[1];
	rv = rv << 8 | ptr[0];
	return rv;
}

static DWORD ld_dword (const BYTE* ptr)	/* Load a 4-byte little-endian word */
{
	DWORD rv;

	rv = ptr[3];
	rv = rv << 8 | ptr[2];
	rv = rv << 8 | ptr[1];
	rv = rv << 8 | ptr[0];
	return rv;
}
static void st_word (BYTE* ptr, WORD val)	/* Store a 2-byte word in little-endian */
{
	*ptr++ = (BYTE)val; val >>= 8;
	*ptr++ = (BYTE)val;
}

static void st_dword (BYTE* ptr, DWORD val)	/* Store a 4-byte word in little-endian */
{
	*ptr++ = (BYTE)val; val >>= 8;
	*ptr++ = (BYTE)val; val >>= 8;
	*ptr++ = (BYTE)val; val >>= 8;
	*ptr++ = (BYTE)val;
}

