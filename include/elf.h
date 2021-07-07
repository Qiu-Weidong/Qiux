#ifndef QIUX_ELF_H_
#define QIUX_ELF_H_

#define EI_NIDENT 16

typedef unsigned int        Elf32_Addr;     // 无符号地址
typedef unsigned short      Elf32_Half;     // 无符号段整数
typedef unsigned int        Elf32_Off;      // 无符号文件偏移
typedef signed int          Elf32_Sword;    // 有符号大整数
typedef unsigned int        Elf32_Word;     // 无符号大整数


// ELF header
typedef struct
{
    unsigned char   e_ident[EI_NIDENT];     // 用来标识一个elf文件
    Elf32_Half      e_type;                 // 文件的类型，2表示可执行文件
    Elf32_Half      e_machine;              // 运行程序需要的体系结构，3表示intel80386
    Elf32_Word      e_version;              // 文件的版本
    Elf32_Addr      e_entry;                // 程序的入口地址，一般为0x80480A0
    Elf32_Off       e_phoff;                // program header table在文件中的偏移量(字节为单位)
    Elf32_Off       e_shoff;                // Section header table在文件中的偏移(字节为单位)
    Elf32_Word      e_flags;                // 对IA32而言，为0
    Elf32_Half      e_ehsize;               // ELF header的大小(字节为单位)
    Elf32_Half      e_phentsize;            // program header table每个条目的大小
    Elf32_Half      e_phnum;                // program header table的条目个数
    Elf32_Half      e_shentsize;            // section header table的每个条目的大小
    Elf32_Half      e_shnum;                // section header table的条目个数
    Elf32_Half      e_shstrndx;             // 包含节名称的字符串表是第几个节(从零开始)
} Elf32_Ehdr;

// Program header
typedef struct
{
    Elf32_Word      p_type;                 // 当前Program header所描述的段的类型
    Elf32_Off       p_offset;               // 段的第一个字节在文件中的偏移
    Elf32_Addr      p_vaddr;                // 段的第一个字节在内存中的虚拟地址
    Elf32_Addr      p_paddr;                // 段的第一个字节在内存中的物理地址，用于在物理地址定位相关的体系中
    Elf32_Word      p_filesz;               // 段在文件中的长度
    Elf32_Word      p_memsz;                // 段在内存中的长度
    Elf32_Word      p_flags;                // 与段相关的标志
    Elf32_Word      p_align;                // 段在文件以及内存中如何对齐
} Elf32_Phdr;

// Section header
typedef struct
{
    Elf32_Word      sh_name;        // 节区名，是节区头部字符串表节区（Section Header String Table Section）的索引。名字是一个 NULL 结尾的字符串。
    Elf32_Word      sh_type;        // 为节区类型
    Elf32_Word      sh_flags;       // 节区标志
    Elf32_Addr      sh_addr;        // 如果节区将出现在进程的内存映像中，此成员给出节区的第一个字节应处的位置。否则，此字段为 0。
    Elf32_Off       sh_offset;      // 此成员的取值给出节区的第一个字节与文件头之间的偏移。
    Elf32_Word      sh_size;        // 此 成 员 给 出 节 区 的 长 度 （ 字 节 数 ）。
    Elf32_Word      sh_link;        // 此成员给出节区头部表索引链接。其具体的解释依赖于节区类型。
    Elf32_Word      sh_info;        // 此成员给出附加信息，其解释依赖于节区类型。
    Elf32_Word      sh_addralign;   // 某些节区带有地址对齐约束.
    Elf32_Word      sh_entsize;     // 某些节区中包含固定大小的项目，如符号表。对于这类节区，此成员给出每个表项的长度字节数。
} Elf32_Shdr;

#endif // QIUX_ELF_H_

