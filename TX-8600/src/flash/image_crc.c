
static inline unsigned int nibblerev(unsigned int data)
{
    return ((data<<4)&0xf0f0f0f0) | ((data>>4)&0x0f0f0f0f);
}

unsigned int CRC32(unsigned int * data, unsigned int num_words, unsigned int expected_crc)
{
    unsigned int tmp;
    unsigned int crc = 0xFFFFFFFF;
    unsigned int polynom = 0xedb88320;
    for(unsigned int i = 0; i < num_words; i++)
    {
        tmp = nibblerev(data[i]);
        asm volatile("crc32 %0, %2, %3" : "=r" (crc) : "0" (crc), "r"  (tmp), "r" (polynom));
    }
    asm volatile("crc32 %0, %2, %3" : "=r" (crc) : "0" (crc), "r" (expected_crc), "r" (polynom));

    return ~crc;
}

static const unsigned int g_polynom = 0xedb88320;
static unsigned int g_expected_crc = 0;
static unsigned int g_crc = 0;
//noly use upgrade image, because foctory image startaddrss not start in page
unsigned int start_image_crc(unsigned char page_256[])
{
    unsigned int *data = (unsigned int *)page_256;
    g_crc = 0xFFFFFFFF;
    g_expected_crc = nibblerev(data[1]);
    unsigned int tmp;
    for(unsigned int i = 3; i < 64; i++)
    {
        tmp = nibblerev(data[i]);
        asm volatile("crc32 %0, %2, %3" : "=r" (g_crc) : "0" (g_crc), "r"  (tmp), "r" (g_polynom));
    }
    tmp = g_crc;
    asm volatile("crc32 %0, %2, %3" : "=r" (tmp) : "0" (tmp), "r" (g_expected_crc), "r" (g_polynom));
    if(~tmp != 0)
    {
        return 1;
    }
    g_expected_crc = nibblerev(data[2]);
    return 0;
}


void put_image_crc(unsigned char page_256[], unsigned int size)
{
    unsigned int *data = (unsigned int *)page_256;
    size = ((size>>2)>64)?64:(size>>2);
    unsigned int tmp;
    for(unsigned int i = 0; i < size; i++)
    {
        tmp = nibblerev(data[i]);
        asm volatile("crc32 %0, %2, %3" : "=r" (g_crc) : "0" (g_crc), "r"  (tmp), "r" (g_polynom));
    }
}

unsigned int stop_image_crc()
{
    asm volatile("crc32 %0, %2, %3" : "=r" (g_crc) : "0" (g_crc), "r" (g_expected_crc), "r" (g_polynom));
    return ~g_crc;
}
