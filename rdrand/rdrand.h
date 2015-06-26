#pragma once

#define RETRIES (1024)

inline int _rdrand16(uint64_t *ret16)
{
        uint16_t _rand;
        int cf_rand_avail;
        asm("\n\
             rdrand %%ax;\n\
             mov $1,%%edx;\n\
             cmovae %%ax,%%dx;\n\
             mov %%edx,%1;\n\
             mov %%ax, %0;"
             : "=r"(_rand),
               "=r"(cf_rand_avail)
             :
             : "%ax","%dx"
             );
             *ret16 = _rand;
        return cf_rand_avail;
}

inline uint16_t rand16()
{
        uint64_t ret16;
        int count = 0;
        while (_rdrand16(&ret16) == 0 && count++ < RETRIES);
        if (count == RETRIES) {
                std::cout << "retries " << count << std::endl;
        }
        return ret16;
}
inline int _rdrand32(uint64_t *ret32)
{
        uint32_t _rand;
        int cf_rand_avail;
        asm("\n\
             rdrand %%eax;\n\
             mov $1,%%edx;\n\
             cmovae %%eax,%%edx;\n\
             mov %%edx,%1;\n\
             mov %%eax, %0;"
             : "=r"(_rand),
               "=r"(cf_rand_avail)
             :
             : "%eax","%edx"
             );
             *ret32 = _rand;
        return cf_rand_avail;
}

inline uint32_t rand32()
{
        uint64_t ret32;
        int count = 0;
        while (_rdrand32(&ret32) == 0 && count++ < RETRIES);
        if (count == RETRIES) {
                std::cout << "retries " << count << std::endl;
        }
        return ret32;
}

inline int _rdrand64(uint64_t *ret64)
{
        unsigned long long int _rand;
        int cf_rand_avail;
        asm("\n\
             rdrand %%rax;\n\
             mov $1,%%edx;\n\
             cmovae %%rax,%%rdx;\n\
             mov %%edx,%1;\n\
             mov %%rax, %0;"
             : "=r"(_rand),
               "=r"(cf_rand_avail)
             :
             : "%rax","%rdx"
             );
             *ret64 = _rand;
        return cf_rand_avail;
}

inline uint64_t rand64()
{
        uint64_t ret64;
        int count = 0;
        while (_rdrand64(&ret64) == 0 && count++ < RETRIES);
        if (count == RETRIES) {
                std::cout << "retries " << count << std::endl;
        }
        return ret64;
}
