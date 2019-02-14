#ifndef FLASH_SPEC
#define FLASH_SPEC
//---------------------------------------------------------------------------------------------------------
#define   FLASH080\
          {01,                             /* IS25LQ080 */ \
           256,                            /* page size */ \
           4096,                           /* num pages */ \
           3,                              /* address size */\
           4,                              /* log2 clock divider */\
           0x9F,                           /* QSPI_RDID */ \
           0,                              /* id dummy bytes */ \
           3,                              /* id size in bytes */ \
           0x9D4014,                       /* device id */ \
           0x20,                           /* QSPI_SE */ \
           4096,                           /* Sector erase is always 4KB */ \
           0x06,                           /* QSPI_WREN */ \
           0x04,                           /* QSPI_WRDI */ \
           PROT_TYPE_NONE,                 /* no protection */ \
           {{0,0},{0x00,0x00}},            /* QSPI_SP, QSPI_SU */ \
           0x02,                           /* QSPI_PP */ \
           0xEB,                           /* QSPI_READ_FAST */ \
           1,                              /* 1 read dummy byte */ \
           SECTOR_LAYOUT_REGULAR,          /* mad sectors */ \
           {4096,{0,{0}}},                 /* regular sector sizes */ \
           0x05,                           /* QSPI_RDSR */ \
           0x01,                           /* QSPI_WRSR */ \
           0x01}                           /* QSPI_WIP_BIT_MASK */ \

#define   FLASHGD64\
          {02,                             /* IS25LQ080 */ \
           256,                            /* page size */ \
           32768,                           /* num pages */ \
           3,                              /* address size */\
           4,                              /* log2 clock divider */\
           0x9F,                           /* QSPI_RDID */ \
           0,                              /* id dummy bytes */ \
           3,                              /* id size in bytes */ \
           0xC84017,                       /* device id */ \
           0x20,                           /* QSPI_SE */ \
           4096,                           /* Sector erase is always 4KB */ \
           0x06,                           /* QSPI_WREN */ \
           0x04,                           /* QSPI_WRDI */ \
           PROT_TYPE_NONE,                 /* no protection */ \
           {{0,0},{0x00,0x00}},            /* QSPI_SP, QSPI_SU */ \
           0x02,                           /* QSPI_PP */ \
           0xEB,                           /* QSPI_READ_FAST */ \
           1,                              /* 1 read dummy byte */ \
           SECTOR_LAYOUT_REGULAR,          /* mad sectors */ \
           {4096,{0,{0}}},                 /* regular sector sizes */ \
           0x05,                           /* QSPI_RDSR */ \
           0x01,                           /* QSPI_WRSR */ \
           0x01}                           /* QSPI_WIP_BIT_MASK */ \

#endif

