/*
**
** Copyright 2011, Accenture Ltd
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/sched.h>

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/interrupt.h>

#include <asm/types.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <gles2_emulator_constants.h>


//#define DEBUG 1
#if DEBUG
#  define  DBGPRINT(...) printk(__VA_ARGS__)
#else
#  define  DBGPRINT(...) ((void)0)
#endif

#define TEST_ALLOC_SIZE 1024000

typedef struct
{
	int allocatedFlag;
	int theFileDescriptor;
	unsigned long theAddressOffset;
	unsigned long allocatedSize;
	unsigned long segmentSize;
} theMemoryPoolStruct;

theMemoryPoolStruct *
initMemoryPool(unsigned int totalSize, unsigned int segmentSize, unsigned long *numAllocatedSegments);

unsigned int
allocateFromPool(theMemoryPoolStruct *theMemoryPool, int theFileDescriptor, unsigned long theSize, unsigned int numberOfMemSegments);

unsigned int
releaseFromPool(theMemoryPoolStruct *theMemoryPool, int theFileDescriptor, unsigned int numberOfMemSegments);

unsigned long
listMemPoolTotalFree(theMemoryPoolStruct *theMemoryPool, unsigned int numberOfMemSegments);


extern char* hostNativeOpenGL_getContiguousAddrFromInit(void);


/* Main structure for the buffers and IRQ holding pen. */
struct goldfish_virtualDevice {
	uint32_t reg_base;
	int irq;
	spinlock_t lock;
	wait_queue_head_t wait;

	char __iomem *buffer_virt;		/* Buffer virtual address. */
	unsigned long buffer_phys;		/* Buffer physical address. */

	char __iomem *output_buffer_1;		/* Write buffer 1 physical address */
	char __iomem *output_buffer_2;		/* Write buffer 2 physical address */
	char __iomem *input_buffer_1;		/* Read buffer 1 physical address */
	char __iomem *input_buffer_2;		/* Read buffer 2 physical address */
	int buffer_status;
	int main_minor_channel_id;			/* Minor device numbers to determine file access */
	int control_minor_channel_id;
	
	char __iomem *mem_allocated_virtual;		/* Buffer virtual address. */
	unsigned long mem_allocated_physical;		/* Buffer physical address. */

	theMemoryPoolStruct *theMemoryPool;
	unsigned long numOfAllocatedSegs;
};


struct virtual_device_data
{
	int main_open_id, control_open_id, seq_id;   						// Track the FD count
	unsigned long memPoolIndex;
	struct goldfish_virtualDevice *g_virtualDevice;
	int thisFileDescriptor;
	int mem_segment_no;
	unsigned long region_physical_address_start;
};

// case we get multiple clients spamming
DEFINE_MUTEX(virtualDevice_mutex);

/* Module parameters*/
int	moduleParam_buffer_type = 0;
int moduleParam_number_of_buffers = NumberCommandBuffers;
int moduleParam_buffer_size = CommandBufferSize;
uint32_t total_buffers_size;

#define GOLDFISH_VIRTUALDEVICE_READ(data, addr) (readl(data->reg_base + addr))
#define GOLDFISH_VIRTUALDEVICE_WRITE(data, addr, x) (writel(x, data->reg_base + addr))


/* Used between goldfish_virtualDevice_probe() and goldfish_virtualDevice_open() */
static struct goldfish_virtualDevice *theSharedBufferData;


/* Counter for number of times opened. */
static atomic_t main_open_count = ATOMIC_INIT(0);   // Main channel open count.
static atomic_t control_open_count = ATOMIC_INIT(0);  // Control channel open count.


/* Read function - called when there is a read request from somewhere. */
static ssize_t
goldfish_virtualDevice_read (struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	struct virtual_device_data *v_drv_data=filp->private_data;
	struct goldfish_virtualDevice *data = v_drv_data->g_virtualDevice;
	int length;
	int result = 0;
	char __iomem *kbuf;

//	mutex_lock (&virtualDevice_mutex);

	DBGPRINT("[INFO (%s)] : -> Read request - count: %d\n", __FUNCTION__, count);

	/* Read if we have data to read :) */
	while (count > 0) {
		length = (count > moduleParam_buffer_size ? moduleParam_buffer_size : count);
		/* Send a command signal to the virtual device in Qemu to start sending data to this, and wait (nonblocking) until we've filled the buffer. */
		GOLDFISH_VIRTUALDEVICE_WRITE(data, VIRTUALDEVICE_START_INPUT, length);

		/* Wait here on an interrupt until we get a full buffer to pass on. */
		wait_event_interruptible (data->wait, (data->buffer_status & (VIRTUALDEVICE_INT_INPUT_BUFFER_1_FULL | VIRTUALDEVICE_INT_INPUT_BUFFER_2_FULL)));
		DBGPRINT("[INFO (%s)] : -> Read request - interrupt wait over :)\n", __FUNCTION__);

		/* Determine from the interrupt signal which buffer was filled. */
		if ((data->buffer_status & VIRTUALDEVICE_INT_INPUT_BUFFER_1_FULL) != 0) {
			kbuf = data->input_buffer_1;
			length = GOLDFISH_VIRTUALDEVICE_READ(data, VIRTUALDEVICE_INPUT_BUFFER_1_AVAILABLE);
			DBGPRINT("    (more) : Buffer 1 has been filled, available bytes: %d  - copying to userspace...\n", length);
		} else {
			kbuf = data->input_buffer_2;
			length = GOLDFISH_VIRTUALDEVICE_READ(data, VIRTUALDEVICE_INPUT_BUFFER_2_AVAILABLE);
			DBGPRINT("    (more) : Buffer 2 has been filled, available bytes: %d - copying to userspace...\n", length);
		}
   
		/* Copy data to user space. */
		if (copy_to_user (buf, kbuf, length))
		{
			DBGPRINT("    (ERROR) : copy_to_user() failed!\n");
//			mutex_unlock(&virtualDevice_mutex);
			return -EFAULT;
		}
		
		result += length;
		buf += length;
		count -= length;
	}

//	mutex_unlock (&virtualDevice_mutex);
	return result;

}


/* Write function - called when device has data to write. */
static ssize_t
goldfish_virtualDevice_write (struct file *filp, const char __user *buf, size_t count, loff_t *pos)
{
	struct virtual_device_data *v_drv_data=filp->private_data;
	struct goldfish_virtualDevice *data = v_drv_data->g_virtualDevice;
	unsigned long irq_flags;
	ssize_t result = 0;
	char __iomem *kbuf;


//	mutex_lock (&virtualDevice_mutex);

	DBGPRINT("[INFO (%s)] : -> Write request - count: %d\n", __FUNCTION__, count);

	/* Write if we have data to write. */
	while (count > 0)
	{
		ssize_t copy = count;
		if (copy > moduleParam_buffer_size) copy = moduleParam_buffer_size;

		/* Wait until there is an available buffer to write to. */
		wait_event_interruptible (data->wait, (data->buffer_status & (VIRTUALDEVICE_INT_OUTPUT_BUFFER_1_EMPTY | VIRTUALDEVICE_INT_OUTPUT_BUFFER_2_EMPTY)));
		DBGPRINT("[INFO (%s)] : -> Write request - interrupt wait over :)\n", __FUNCTION__);
		
		/* Determine from the interrupt signal which buffer is ready to fill with data. */
		if ((data->buffer_status & VIRTUALDEVICE_INT_OUTPUT_BUFFER_1_EMPTY) != 0) {
			kbuf = data->output_buffer_1;
		} else {
			kbuf = data->output_buffer_2;
		}

		/* Copy from user space to the appropriate buffer. */
		if (copy_from_user(kbuf, buf, copy))
		{
			DBGPRINT("    (more) : copy_from_user() failed!\n");
			result = -EFAULT;
			break;
		}
		else
		{
			spin_lock_irqsave (&data->lock, irq_flags);

			/* Now we have filled the buffer, clear the buffer empty flag and signal to the Qemu device that there is a buffer with data available. */
			if (kbuf == data->output_buffer_1) {
				data->buffer_status &= ~VIRTUALDEVICE_INT_OUTPUT_BUFFER_1_EMPTY;
				GOLDFISH_VIRTUALDEVICE_WRITE(data, VIRTUALDEVICE_OUTPUT_BUFFER_1_AVAILABLE, copy);
			} else {
				data->buffer_status &= ~VIRTUALDEVICE_INT_OUTPUT_BUFFER_2_EMPTY;
				GOLDFISH_VIRTUALDEVICE_WRITE(data, VIRTUALDEVICE_OUTPUT_BUFFER_2_AVAILABLE, copy);
			}

			spin_unlock_irqrestore (&data->lock, irq_flags);
		}
 
		buf += copy;
		result += copy;
		count -= copy;
	}

//	mutex_unlock (&virtualDevice_mutex);
	return result;
}


/* Get this request's minor device id. */
static int get_id (struct file *file)
{
	return MINOR(file->f_dentry->d_inode->i_rdev);
}


/* Fsync function - called when a request is sent to ensure data is transferred. */
static int
goldfish_virtualDevice_fsync (struct file *fp, int datasync)
{
	//struct virtual_device_data *v_drv_data=fp->private_data;
	//struct goldfish_virtualDevice *data = v_drv_data->g_virtualDevice;

	DBGPRINT("[INFO (%s)] : -> Fsync request.  Datasync: %d\n", __FUNCTION__, datasync);

	return 0;
}


/* Flush function - called when a request is sent to ensure data is transferred. */
static int
goldfish_virtualDevice_flush (struct file *fp, fl_owner_t id)
{
	//struct virtual_device_data *v_drv_data=fp->private_data;
	//struct goldfish_virtualDevice *data = v_drv_data->g_virtualDevice;

	DBGPRINT("[INFO (%s)] : -> Flush request.\n", __FUNCTION__);

	return 0;
}


/* When device is open()'ed, we initialise a few things. */
static int
goldfish_virtualDevice_open (struct inode *ip, struct file *fp)
{
	int minor_id;

	struct virtual_device_data *v_device_data ;

//	mutex_lock (&virtualDevice_mutex);

	if (!theSharedBufferData) {
		DBGPRINT("[INFO (%s)] : (ERROR) : No valid device data structure.\n", __FUNCTION__);
	//	mutex_unlock (&virtualDevice_mutex);
		return -ENODEV;
	}

	v_device_data = kzalloc(sizeof (struct virtual_device_data),GFP_KERNEL);
	v_device_data->main_open_id = atomic_read(&main_open_count);
	v_device_data->control_open_id = atomic_read(&control_open_count);
	v_device_data->seq_id = 0;
	v_device_data->g_virtualDevice = theSharedBufferData;
	v_device_data->region_physical_address_start = 0;

	fp->private_data = v_device_data;

	minor_id = get_id (fp);

	DBGPRINT("[INFO (%s)] : Device opened.  Main channel count: %d, control open count: %d, minor ID: %dd\n", __FUNCTION__, atomic_read(&main_open_count), atomic_read(&control_open_count), minor_id);
				
	if (minor_id == theSharedBufferData->main_minor_channel_id)
	{
		DBGPRINT("    (more) : On main channel.\n");
		if (atomic_inc_return (&main_open_count) == 1)
		{
		  	DBGPRINT("First global open.  Enabling interrupts.\n");
			theSharedBufferData->buffer_status = (VIRTUALDEVICE_INT_OUTPUT_BUFFER_1_EMPTY |  VIRTUALDEVICE_INT_OUTPUT_BUFFER_2_EMPTY);
			GOLDFISH_VIRTUALDEVICE_WRITE(theSharedBufferData, VIRTUALDEVICE_INITIALISE, VIRTUALDEVICE_INT_MASK);
		}
		goto open_unlock;
	} 
	else 
	{
		atomic_inc (&control_open_count);
		DBGPRINT("    (more) : On minor channel.  Opened on device control ID: %d.\n", atomic_read(&control_open_count));
	}

open_unlock:
//	mutex_unlock (&virtualDevice_mutex);
	return 0;
}


/* Called when device is released. */
static int
goldfish_virtualDevice_release (struct inode *ip, struct file* fp)
{
	struct virtual_device_data *v_drv_data=fp->private_data;
	struct goldfish_virtualDevice *data = v_drv_data->g_virtualDevice;
	int minor_id = get_id (fp);


//	mutex_lock (&virtualDevice_mutex);
	DBGPRINT("[INFO (%s)] : Device release requested on minor: %d  \n", __FUNCTION__, minor_id);

	/* Don't reset-close the parameters if the control channel closes down. */
	if (minor_id == data->main_minor_channel_id)
	{

		DBGPRINT("    (more) : MAIN CHANNEL: %d  Device_id: %d  \n", __LINE__, v_drv_data->main_open_id);
		if (atomic_dec_return(&main_open_count) == 0)
		{
			DBGPRINT("    (more) : Releasing and resetting device.\n");
			GOLDFISH_VIRTUALDEVICE_WRITE(theSharedBufferData, VIRTUALDEVICE_INITIALISE, 0);
		}
	} else {
		DBGPRINT("    (more) : CONTROL CHANNEL: %d  Device_id: %d  \n", __LINE__, v_drv_data->control_open_id);
	}	


	kfree(v_drv_data);

//	mutex_unlock (&virtualDevice_mutex);
	return 0;
}


#ifdef CONFIG_COMPAT
static long goldfish_virtualDevice_compat_ioctl (struct file *fp, unsigned int cmd_in, unsigned long arg)
{
	/* Sign-extend the argument so it can be used as a pointer. */
	return goldfish_virtualDevice_ioctl (fp, cmd_in, (unsigned long)compat_ptr(arg));
}
#endif


static long
goldfish_virtualDevice_ioctl (struct file *fp, unsigned int cmd_in, unsigned long arg)
{
	unsigned long irq_flags;

        struct virtual_device_data *v_drv_data=fp->private_data;
        struct goldfish_virtualDevice *data = v_drv_data->g_virtualDevice;


	int minor_id = get_id (fp);

//	if (!mutex_trylock (&virtualDevice_mutex))
//		return -EBUSY;

	DBGPRINT("[INFO (%s)] : Called from minor device: %d.\n", __FUNCTION__, minor_id);
	DBGPRINT("    (more) : -> command is %d , data %lx :)\n", cmd_in, arg);

	switch(cmd_in)
	{
		case VIRTUALDEVICE_IOCTL_ALLOCATE_SHAREDMEM:
			DBGPRINT("    (more) : -> command is VIRTUALDEVICE_IOCTL_ALLOCATE_SHAREDMEM, size: %d\n", (int)arg);
			break;

		case VIRTUALDEVICE_IOCTL_SYSTEM_RESET:
			DBGPRINT("    (more) : -> command is VIRTUALDEVICE_IOCTL_SYSTEM_RESET\n");
			spin_lock_irqsave (&data->lock, irq_flags);
			GOLDFISH_VIRTUALDEVICE_WRITE(data, VIRTUALDEVICE_IOCTL_SYSTEM_RESET, arg);
			spin_unlock_irqrestore (&data->lock, irq_flags);
			break;

		case VIRTUALDEVICE_IOCTL_GRALLOC_ALLOCATED_REGION_INFO:
			DBGPRINT("    (more) : -> command is VIRTUALDEVICE_IOCTL_GRALLOC_ALLOCATED_REGION_INFO\n");
			spin_lock_irqsave (&data->lock, irq_flags);
			GOLDFISH_VIRTUALDEVICE_WRITE(data, VIRTUALDEVICE_IOCTL_GRALLOC_ALLOCATED_REGION_INFO, arg);
			spin_unlock_irqrestore (&data->lock, irq_flags);
			break;

		case VIRTUALDEVICE_IOCTL_SIGNAL_BUFFER_SYNC:
			DBGPRINT("    (more) : -> command is VIRTUALDEVICE_IOCTL_SIGNAL_SYNC\n");
			spin_lock_irqsave (&data->lock, irq_flags);
			GOLDFISH_VIRTUALDEVICE_WRITE(data, VIRTUALDEVICE_IOCTL_SIGNAL_BUFFER_SYNC, arg);
			spin_unlock_irqrestore (&data->lock, irq_flags);
			break;

		case VIRTUALDEVICE_IOCTL_REGION_PHYSICAL_ADDR_START:
			DBGPRINT("    (more) : -> command is VIRTUALDEVICE_IOCTL_REGION_PHYSICAL_ADDR_START\n");
			return 	v_drv_data->region_physical_address_start;
			break;

		default:
//				mutex_unlock (&virtualDevice_mutex);
				return -EINVAL;
			break;
	}

//	mutex_unlock (&virtualDevice_mutex);
	return 0;
}


static void
goldfish_virtualDevice_mmap_open (struct vm_area_struct *vma)
{
    //struct file *filp = vma->vm_file;
    //struct virtual_device_data *v_drv_data=filp->private_data;
    //struct goldfish_virtualDevice *data = v_drv_data->g_virtualDevice;

	DBGPRINT("[INFO (%s)] : >>>>>>>>> MMAP OPEN >>>>>>>>> \n", __FUNCTION__);	
}


/* Release a mapped register area, and disconnect the client. */
static void
goldfish_virtualDevice_mmap_close (struct vm_area_struct *vma)
{
    struct file *filp = vma->vm_file;
    struct virtual_device_data *v_drv_data=filp->private_data;
    struct goldfish_virtualDevice *data = v_drv_data->g_virtualDevice;

    mutex_lock (&virtualDevice_mutex);

	DBGPRINT("[INFO (%s)] : <<<<<<<< MMAP CLOSE (%d) Seq: %d  Fd: %d <<<<<<<< \n", __FUNCTION__, v_drv_data->control_open_id, v_drv_data->seq_id, (int)v_drv_data->thisFileDescriptor);

	v_drv_data->seq_id--;

	if (v_drv_data->seq_id <= 0)
	{
		DBGPRINT("    (more) : Sequence id is 0 or less - releasing fragments!\n");
		releaseFromPool(data->theMemoryPool, v_drv_data->thisFileDescriptor, data->numOfAllocatedSegs);	
		listMemPoolTotalFree(data->theMemoryPool, data->numOfAllocatedSegs);
	}

	vma->vm_private_data = 0;

       mutex_unlock (&virtualDevice_mutex);

}

/* Operations for kernel to deal with a mapped register area. */
static struct vm_operations_struct
goldfish_virtualDevice_mmap_ops = {
	.open = goldfish_virtualDevice_mmap_open,
	.close = goldfish_virtualDevice_mmap_close,
};


/* Memory Map from physical address pool */
static int goldfish_mmap (struct file *filp, struct vm_area_struct *vma)
{
	int ret;
    int size = vma->vm_end - vma->vm_start;
    struct virtual_device_data *v_drv_data=filp->private_data;
    struct goldfish_virtualDevice *data = v_drv_data->g_virtualDevice;

	unsigned long offset, offset_phy, offset_pg;
	char *address;


     mutex_lock (&virtualDevice_mutex);

	DBGPRINT("[INFO (%s)] : Mmap requested, size: %d, file: %s pd: %pd\n",__FUNCTION__, size, filp->f_path.dentry->d_name.name, filp->f_path.dentry);
        DBGPRINT("    (more) : File * %p, Sequence id: %d, Current file count: %lu\n", filp, v_drv_data->seq_id, file_count(filp));	

    address = hostNativeOpenGL_getContiguousAddrFromInit();

    DBGPRINT("    (more) : Physical pool address = 0x%x\n", (unsigned int)virt_to_phys(address));
    //align page boundaries:)	
    offset = (unsigned long)address /*+ vma->vm_pgoff*/;

	if (v_drv_data->seq_id == 0)
	{
		v_drv_data->thisFileDescriptor = v_drv_data->control_open_id;
		DBGPRINT("    (more) : First open, sequence id: %d\n", v_drv_data->seq_id);
		v_drv_data->memPoolIndex = allocateFromPool(data->theMemoryPool, v_drv_data->thisFileDescriptor, size, data->numOfAllocatedSegs);
		if (v_drv_data->memPoolIndex == data->numOfAllocatedSegs)
		{
			DBGPRINT("    (more) : Failed to find enough contiguous segments.\n");
		    mutex_unlock (&virtualDevice_mutex);
			return -EIO;
		}
		listMemPoolTotalFree(data->theMemoryPool, data->numOfAllocatedSegs);
	}

	offset += data->theMemoryPool[v_drv_data->memPoolIndex].theAddressOffset;

	DBGPRINT("    (more) : This offset: %lu, addr: 0x%lu\n", data->theMemoryPool[v_drv_data->memPoolIndex].theAddressOffset, offset);


    DBGPRINT("    (more) : Offset before page align = 0x%x\n", (unsigned int)offset);	
    offset = ((offset + PAGE_SIZE - 1) & PAGE_MASK);
    DBGPRINT("    (more) : Offset after page align = 0x%x\n", (unsigned int)offset);
    offset_phy = virt_to_phys((void *)offset);

	v_drv_data->region_physical_address_start = offset_phy;

    offset_pg = offset_phy >> PAGE_SHIFT;
     DBGPRINT("    (more) : Offset is 0x%x address is 0x%x , vmoffset is 0x%x,size is 0x%d offset_pg 0x%x\n", (unsigned int)offset, (unsigned int)address, (unsigned int)vma->vm_pgoff, (unsigned int)vma->vm_end - (unsigned int)vma->vm_start, (unsigned int)offset_pg );

	ret = 0;

	ret = remap_pfn_range(vma,
			    vma->vm_start,
			    offset_pg,
			    size,
			    vma->vm_page_prot);
     if ( ret < 0) {
		DBGPRINT("    (more) : remap_pfn_range failed: %d\n", ret);
        mutex_unlock (&virtualDevice_mutex);
		return -EIO;
	}
	DBGPRINT("    (more) : remap_pfn_range is succesfull addr: 0x%x\n", (unsigned int)ret);

	vma->vm_ops = &goldfish_virtualDevice_mmap_ops;

	v_drv_data->seq_id++;

     mutex_unlock (&virtualDevice_mutex);
     return ret;
}


/* Device interrupt handler. */
static irqreturn_t
goldfish_virtualDevice_interrupt (int irq, void *dev_id)
{
	unsigned long irq_flags;
	struct goldfish_virtualDevice *data = dev_id;
	uint32_t status;


	DBGPRINT("[INFO (%s)] : Interrupt signal %d.\n", __FUNCTION__, irq);

	spin_lock_irqsave (&data->lock, irq_flags);

	/* Read buffer status flags. */
	status = GOLDFISH_VIRTUALDEVICE_READ(data, VIRTUALDEVICE_INT_STATUS);
	status &= VIRTUALDEVICE_INT_MASK;
	/* If buffers are newly empty, wake up blocked goldfish_virtualDevice_write() call. */
	if (status) {
		data->buffer_status = status;
		wake_up (&data->wait);
	}
	
	spin_unlock_irqrestore (&data->lock, irq_flags);
	return status ? IRQ_HANDLED : IRQ_NONE;
}


/* File operations for this device. */
static struct file_operations
goldfish_virtualDevice_fops = {
	.owner = THIS_MODULE,
	.read = goldfish_virtualDevice_read,
	.write = goldfish_virtualDevice_write,
	.fsync = goldfish_virtualDevice_fsync,
	.flush = goldfish_virtualDevice_flush,
	.open = goldfish_virtualDevice_open,
	.release = goldfish_virtualDevice_release,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = goldfish_virtualDevice_compat_ioctl,
#endif
	.unlocked_ioctl = goldfish_virtualDevice_ioctl,			/* Jose: later kernels need unlocked_ioctl and so on. */
	.mmap = goldfish_mmap,

};


/* Device will appear in default /dev/... when loaded */
static struct miscdevice
goldfish_virtualDevice_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "virtual_device",
	.fops = &goldfish_virtualDevice_fops,
};
/* Device will appear in default /dev/... when loaded */
static struct miscdevice
goldfish_virtualDevice_device_control = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "virtual_device_control",
	.fops = &goldfish_virtualDevice_fops,
};
/* Device will appear in default /dev/... when loaded */
static struct miscdevice
goldfish_virtualDevice_device_exchange = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "virtual_device_exchange",
	.fops = &goldfish_virtualDevice_fops,
};

/* Device probe/init function - called when the device is loaded. */
static int
goldfish_virtualDevice_probe (struct platform_device *pdev)
{
	int ret;
	struct resource *r;
	struct goldfish_virtualDevice *data;
	dma_addr_t buf_addr;
	char *rag_memalloc;


	DBGPRINT ("[INFO (%s)] : Starting up.\n", __FUNCTION__);

	data = kzalloc (sizeof(*data), GFP_KERNEL);
	if (data == NULL) {
		ret = -ENOMEM;
		goto err_data_alloc_failed;
	}
	spin_lock_init (&data->lock);
	init_waitqueue_head (&data->wait);
	platform_set_drvdata (pdev, data);

	r = platform_get_resource (pdev, IORESOURCE_MEM, 0);
	if (r == NULL) {
		DBGPRINT ("    (ERROR) : platform_get_resource() failed!\n");
		ret = -ENODEV;
		goto err_no_io_base;
	}
	data->reg_base = IO_ADDRESS(r->start - IO_START);

	data->irq = platform_get_irq (pdev, 0);
	if (data->irq < 0) {
		DBGPRINT ("    (ERROR) : platform_get_irq() failed!\n");
		ret = -ENODEV;
		goto err_no_irq;
	}

	total_buffers_size = moduleParam_number_of_buffers * moduleParam_buffer_size * 2;			/* x2 because of read-write */

	data->buffer_virt = dma_alloc_writecombine (&pdev->dev, total_buffers_size, &buf_addr, GFP_KERNEL);
	if(data->buffer_virt == 0) {
		ret = -ENOMEM;
		goto err_alloc_write_buffer_failed;
	}
	data->buffer_phys = buf_addr;
	data->output_buffer_1 = data->buffer_virt;
	data->output_buffer_2 = data->output_buffer_1 + moduleParam_buffer_size;
	data->input_buffer_1 = data->output_buffer_2 + moduleParam_buffer_size;
	data->input_buffer_2 = data->input_buffer_1 + moduleParam_buffer_size;
	DBGPRINT ("    (more) : Physical: %x, Virtual base: %x\n", buf_addr, (unsigned int)data->buffer_virt);
	
	ret = request_irq (data->irq, goldfish_virtualDevice_interrupt, IRQF_SHARED, pdev->name, data);
	if (ret)
		goto err_request_irq_failed;

	if ((ret = misc_register (&goldfish_virtualDevice_device))) 
	{
		DBGPRINT ("    (ERROR) : misc_register(%s) returned %d in goldfish_virtualDevice_init\n", goldfish_virtualDevice_device.name, ret);
		goto err_misc_register_failed;
	}
	DBGPRINT ("    (more) : Registered device '%s' with minor: %d\n", goldfish_virtualDevice_device.name, goldfish_virtualDevice_device.minor);
	data->main_minor_channel_id = goldfish_virtualDevice_device.minor;

	if ((ret = misc_register (&goldfish_virtualDevice_device_control))) 
	{
		DBGPRINT ("    (ERROR) : misc_register(%s) returned %d in goldfish_virtualDevice_init\n", goldfish_virtualDevice_device_control.name, ret);
		goto err_misc_register_failed;
	}
	DBGPRINT ("    (more) : Registered device '%s' with minor: %d\n", goldfish_virtualDevice_device_control.name, goldfish_virtualDevice_device_control.minor);
	data->control_minor_channel_id = goldfish_virtualDevice_device_control.minor;

	if ((ret = misc_register (&goldfish_virtualDevice_device_exchange))) 
	{
		DBGPRINT ("    (ERROR) : misc_register(%s) returned %d in goldfish_virtualDevice_init\n", goldfish_virtualDevice_device_exchange.name, ret);
		goto err_misc_register_failed;
	}
	DBGPRINT ("    (more) : Registered device '%s' with minor: %d\n", goldfish_virtualDevice_device_exchange.name, goldfish_virtualDevice_device_exchange.minor);
	data->control_minor_channel_id = goldfish_virtualDevice_device_exchange.minor;

	/* Send command to the Host's Qemu device driver */
	GOLDFISH_VIRTUALDEVICE_WRITE(data, VIRTUALDEVICE_INITIALISE, buf_addr);
	GOLDFISH_VIRTUALDEVICE_WRITE(data, SET_OUTPUT_BUFFER_1_ADDRESS, buf_addr);
	GOLDFISH_VIRTUALDEVICE_WRITE(data, SET_OUTPUT_BUFFER_2_ADDRESS, buf_addr + moduleParam_buffer_size);
	GOLDFISH_VIRTUALDEVICE_WRITE(data, SET_INPUT_BUFFER_1_ADDRESS, buf_addr + (moduleParam_buffer_size * 2));
	GOLDFISH_VIRTUALDEVICE_WRITE(data, SET_INPUT_BUFFER_2_ADDRESS, buf_addr + (moduleParam_buffer_size * 3));

	theSharedBufferData = data;
	
	rag_memalloc = hostNativeOpenGL_getContiguousAddrFromInit();

	if(rag_memalloc)
	{
		DBGPRINT(KERN_INFO "    (more) : Obtained 8MB of allocated physical bootmem at virtual address = 0x%x,  physical address = 0x%x\n", (unsigned int)rag_memalloc, (unsigned int)virt_to_phys(rag_memalloc));

		data->theMemoryPool = initMemoryPool(0x800000, 32768, &data->numOfAllocatedSegs);
		if (!data->theMemoryPool)
		{
			DBGPRINT(KERN_INFO "    (more) : Failed to allocate pool segment structures.\n");
			goto err_data_alloc_failed;
		}

	} else {
		DBGPRINT(KERN_INFO "    (more) : Failed to allocate extra bootmem of 8MB_from Virtual Driver\n");
		goto err_data_alloc_failed;
	}

	return 0;


err_misc_register_failed:
err_request_irq_failed:
	dma_free_writecombine (&pdev->dev, total_buffers_size, data->buffer_virt, data->buffer_phys);
err_alloc_write_buffer_failed:
err_no_irq:
err_no_io_base:
err_data_alloc_failed:
	kfree(data);
	return ret;
}

/* Called when device is unloaded. */
static int 
goldfish_virtualDevice_remove (struct platform_device *pdev)
{
	struct goldfish_virtualDevice *data = platform_get_drvdata (pdev);


//	mutex_lock (&virtualDevice_mutex);

	DBGPRINT ("[INFO (%s)] : Module shutting down...  ", __FUNCTION__);

	misc_deregister (&goldfish_virtualDevice_device);
	misc_deregister (&goldfish_virtualDevice_device_control);
	free_irq (data->irq, data);
	kfree (data);
	kfree(data->theMemoryPool);
	theSharedBufferData = NULL;
	DBGPRINT ("Done.\n");
//	mutex_unlock (&virtualDevice_mutex);
	return 0;
}


static struct platform_driver
goldfish_virtualDevice_driver = {
	.probe		= goldfish_virtualDevice_probe,
	.remove		= goldfish_virtualDevice_remove,
	.driver = {
	.name = "virtual-device"					/* Must match the device dev.name registered in Qemu. */
	}
};


/* Module registration. */
static int __init
goldfish_virtualDevice_init (void)
{
	int ret;
	
	ret = platform_driver_register (&goldfish_virtualDevice_driver);
	if (ret < 0)
	{
		DBGPRINT ("[INFO (%s)] : platform_driver_register() returned %d\n", __FUNCTION__, ret);
		return ret;
	}

	return ret;
}


/* virtual device exit */
static void __exit
goldfish_virtualDevice_exit (void)
{
	DBGPRINT ("[INFO (%s)] : Device driver exiting.\n", __FUNCTION__);
	platform_driver_unregister (&goldfish_virtualDevice_driver);
}


theMemoryPoolStruct *
initMemoryPool(unsigned int totalSize, unsigned int segmentSize, unsigned long *numAllocatedSegments)
{
unsigned long i, k, numberOfSegments;
theMemoryPoolStruct *thisMemoryPool;


	numberOfSegments = totalSize / segmentSize;
	DBGPRINT ("[INFO (%s)] : -> Number of segments needed: %lu\n", __FUNCTION__, numberOfSegments);

	DBGPRINT ("    (more) : Size of memory pool structures: %lu\n", (sizeof(theMemoryPoolStruct) * numberOfSegments));
	thisMemoryPool = (theMemoryPoolStruct *)kzalloc((sizeof(theMemoryPoolStruct) * numberOfSegments), GFP_KERNEL);
	if (!thisMemoryPool) return (0);
	k = 0;
	for (i = 0; i < numberOfSegments; i++)
	{
		thisMemoryPool[i].allocatedFlag = 0;
		thisMemoryPool[i].theFileDescriptor = 0;
		thisMemoryPool[i].theAddressOffset = k;
		thisMemoryPool[i].allocatedSize = 0;
		thisMemoryPool[i].segmentSize = segmentSize;
		k += segmentSize;
	}
	*numAllocatedSegments = numberOfSegments;
	return(thisMemoryPool);

}


unsigned int
allocateFromPool (theMemoryPoolStruct *theMemoryPool, int theFileDescriptor, unsigned long theSize, unsigned int numberOfMemSegments)
{
int i, j, k, stitchedSegments=0, numberOfTailSegments, leastTailSegments, leastTailSegmentsIndex, theFirstStitchIndex;
unsigned long theSearchSize;


	DBGPRINT ("[INFO (%s)] : Allocating from pool %lu bytes (aligned: %lu) with file descriptor: %d.\n", __FUNCTION__, theSize, ((theSize + PAGE_SIZE - 1) & PAGE_MASK), theFileDescriptor);
	theSize = ((theSize + PAGE_SIZE - 1) & PAGE_MASK);

	leastTailSegmentsIndex = 0;
	leastTailSegments = numberOfMemSegments;
	/* Try to find and stitch contiguous segments together if one is not enough. */
	for (i = 0; i < numberOfMemSegments; i++)
	{
		stitchedSegments = 0;
		theSearchSize = 0;
		if (theMemoryPool[i].allocatedFlag == 0)
		{
			theFirstStitchIndex = i;
			for (j = i; j < numberOfMemSegments && theMemoryPool[j].allocatedFlag == 0; j++)
			{
				numberOfTailSegments = 0;
				stitchedSegments++;
				theSearchSize += theMemoryPool[j].segmentSize;
				if (theSearchSize >= theSize)
				{
					for (k = j + 1; k < numberOfMemSegments && theMemoryPool[k].allocatedFlag == 0; k++)
					{
						numberOfTailSegments++;
					}
					if(numberOfTailSegments < leastTailSegments)
					{
						leastTailSegments = numberOfTailSegments;
						leastTailSegmentsIndex = theFirstStitchIndex;
					}
					DBGPRINT ("    (more) : Found %d suitable segments at index: %d, total segment size: %ld, tail segments: %d\n", stitchedSegments, i, theSearchSize, numberOfTailSegments);
				}
				j += numberOfTailSegments;
			}
			i = j;
		}
	}

	if (leastTailSegments == numberOfMemSegments)
	{
		DBGPRINT ("    (more) : Couldn't find contiguous segments for that size.\n");
		return (numberOfMemSegments);
	} else {
		DBGPRINT ("    (more) : Address offset: 0x%x\n", (unsigned int)theMemoryPool[leastTailSegmentsIndex].theAddressOffset);
		DBGPRINT ("    (more) : Done.  Least tail segments at index: %d with %d segments (%d tails).\n", leastTailSegmentsIndex, stitchedSegments, leastTailSegments);
		for (i = leastTailSegmentsIndex; i < (leastTailSegmentsIndex + stitchedSegments); i++)
		{
			theMemoryPool[i].allocatedFlag = 1;
			theMemoryPool[i].theFileDescriptor = theFileDescriptor;
		}
		return(leastTailSegmentsIndex);
	}
}


unsigned int
releaseFromPool (theMemoryPoolStruct *theMemoryPool, int theFileDescriptor, unsigned int numberOfMemSegments)
{
unsigned int i;
unsigned long totalReleased;


	DBGPRINT ("[INFO (%s)] : Releasing from pool, file index %d.\n", __FUNCTION__, theFileDescriptor);

	totalReleased = 0;
	for (i = 0; i < numberOfMemSegments; i++)
	{
		if ((theMemoryPool[i].theFileDescriptor == theFileDescriptor) && (theMemoryPool[i].allocatedFlag == 1))
		{
			DBGPRINT ("    (more) : Released from pool (%d) at index: %d\n", theFileDescriptor, i);
			theMemoryPool[i].allocatedFlag = 0;
			theMemoryPool[i].theFileDescriptor = 0;
			totalReleased += theMemoryPool[i].segmentSize;
		}
	}
	DBGPRINT ("    (more) : Released %lu bytes. \n", totalReleased);
	return(0);
}


unsigned long
listMemPoolTotalFree (theMemoryPoolStruct *theMemoryPool, unsigned int numberOfMemSegments)
{
unsigned int i;
unsigned long totalSize;


	DBGPRINT ("[INFO (%s)] : Adding up free segments...  Map layout ->\n", __FUNCTION__);
	DBGPRINT ("|");

	totalSize = 0;
	for (i = 0; i < numberOfMemSegments; i++)
	{
		if (theMemoryPool[i].allocatedFlag == 0)
		{
			totalSize += theMemoryPool[i].segmentSize;
			DBGPRINT ("-");
		} else {
			DBGPRINT ("%d:", theMemoryPool[i].theFileDescriptor);
		}
	}
	DBGPRINT ("|\n    (more) : Total %lu bytes are free. \n", totalSize);
	return(totalSize);
}


/* Standard module init - exit symbol */
module_init(goldfish_virtualDevice_init);
module_exit(goldfish_virtualDevice_exit);


/* Module info data */
MODULE_AUTHOR("Jose Miguel Commins, jose.m.commins@accenture.com");
MODULE_DESCRIPTION("Android virtual device module");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");

module_param(moduleParam_buffer_type, int, 0);
MODULE_PARM_DESC(moduleParam_buffer_type,
		 "Buffer type."
		 "(0 = test option, 1 = test option (default = 0)");
module_param(moduleParam_number_of_buffers, int, 0);
MODULE_PARM_DESC(moduleParam_number_of_buffers,
		 "Number of buffers."
		 "(1...8 (default = 2, hardcoded atm.)");
module_param(moduleParam_buffer_size, int, 0);
MODULE_PARM_DESC(moduleParam_buffer_size,
		 "Buffer size in bytes (aligned to nearest 4096 bytes)."
		 "(4096... (default = 4096)");


