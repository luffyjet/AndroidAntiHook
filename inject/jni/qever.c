#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <android/log.h>
#include <pthread.h>

#define LOG_TAG "qever"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__);

#ifndef u1
#define u1 unsigned char
#define u2 unsigned short
#define u4 unsigned int
#endif
#define LOADED_CLASSES_OFFSET   0xAC
#define NAME_OF_CLASS_OFFSET    24
#define METHOD_OF_CLASS_OFFSET  0x64

typedef struct{
    u4 hashValue;
    void* data;
} HashEntry ;

typedef struct{
    int         tableSize;          /* must be power of 2 */
    int         numEntries;         /* current #of "live" entries */
    int         numDeadEntries;     /* current #of tombstone entries */
    HashEntry*  pEntries;           /* array on heap */
    void * freeFunc;
    void * lock;
} HashTable ;

typedef struct{
    u1*             clazz;
    u4              accessFlags;
    u2              methodIndex;
    u2              registersSize;
    u2              outsSize;
    u2              insSize;
    const char*     name;
    void*           prototype;
    const char*     shorty;
    const u2*       insns;   
    int             jniArgInfo;
    void*           nativeFunc;
    void*           x;
    void*           xx;
    void*           xxx;
    void*           xxxx;
} Method ;
u1 * get_gdvm_address()
{
    u4 * gDvm;
    void * handle = dlopen("libdvm.so", RTLD_NOW);
    if(handle == NULL){
        LOGD("[E] %s > dlopen error, reson : %s", __FUNCTION__, dlerror());
        return NULL;
    }
    
    gDvm = (u4*)dlsym(handle, "gDvm");
    if(gDvm == NULL){
        LOGD("[E] %s > dlsym error, reson : %s", __FUNCTION__, dlerror());
    }
    
    close(handle);
    return gDvm;
}

void DumpMemory(u4 * addr, u4 size)
{
    LOGD("DumpMemory of 0x%08x", addr);
    int i;
    for(i = 0; i < size / sizeof(u4); i+=4){
        LOGD("%08x %08x %08x %08x", addr[i], addr[i + 1], addr[i+2],addr[i+3]);
    }
}

void DumpMemoryByChar(u1 * addr, u4 size)
{
    LOGD("DumpMemoryByChar of 0x%08x", addr);
    int i;
    for(i = 0; i < size; i+= 4){
        LOGD("%02x %02x %02x %02x", addr[i], addr[i + 1], addr[i+2],addr[i+3]);
    }
    
}
    
void DumpString(u4 * addr, u4 num)
{
    LOGD("DumpString of 0x%08x", addr);
    int i;
    for(i = 8; i < num; i++){
        char * p = (char *)addr[i];
        LOGD("%d String : %s", i, p);
    }    
}

int get_module_range(char * name, uint32_t * b, uint32_t * e)
{
    uint32_t begin, end, tmp;
    FILE * fp;
    char maps[0x40];
    char line[1024];
    pid_t pid = getpid();
    snprintf(maps, 0x40, "/proc/%d/maps", pid);
    
    fp = fopen(maps, "r");
    if(fp == NULL){
        LOGD("[E] %s > fopen error(%d)", __FUNCTION__, errno);
        return -1;
    }
    
    begin = 0xFFFFFFFF;
    end = 0;
    while(fgets(line, 1024,fp)!= NULL){
        if(strstr(line, name)){
            char * p = strtok(line, " ");
            p = strtok(p, "-");
            tmp = strtoul(p, 0, 16);
            if(tmp < begin){
                begin = tmp;
            }
            p = strtok(NULL, "-");
            tmp = strtoul(p, 0, 16);
            if(tmp > end){
                end = tmp;
            }
        }
    }
    
    LOGD("[I] %s > 0x%08x-0x%08x    %s", __FUNCTION__, begin, end, name);
    
    *b = begin;
    *e = end;
    
    return 0;
}

void DumpSo(char *SoName)
{
    FILE * fp;
    uint32_t begin,end;
    get_module_range(SoName, &begin, &end);
    
    fp = fopen("/data/data/com.crackme/dump.so", "wb");
    if(fp == NULL){
        LOGD("[E] %s > fopen error!", __FUNCTION__);
        return;
    }
    fwrite((void*)begin, sizeof(char), end - begin, fp);
    fclose(fp);
}

void DumpDex(char *DexName)
{
    FILE * fp;
    uint32_t begin,end;
    get_module_range(DexName, &begin, &end);
    
    fp = fopen("/data/data/com.crackme/dump.dex", "wb");
    if(fp == NULL){
        LOGD("[E] %s > fopen error!", __FUNCTION__);
        return;
    }
    fwrite((void*)begin, sizeof(char), end - begin, fp);
    fclose(fp);
}

int do_task()
{
    static u1* gDvm = NULL;
    static u1* CrackMe = NULL;  
    static u4  MethodNum = 0;
    static Method* MethodList = NULL;
 
    int i;
    LOGD("Do Task ... ");
    if(CrackMe == NULL){
        if(MethodList == NULL){
            HashTable * table;
            HashEntry * entry;
            int hash_size, i;
            u1 * class_list;

            if(gDvm == NULL){
                gDvm = get_gdvm_address();
                if(gDvm == NULL){
                    LOGD("[E] %s > Unable to get gDvm! exit!", __FUNCTION__, dlerror());
                    return 0;
                }
            }
            LOGD("[I] %s > gDvm = 0x%08x", __FUNCTION__, gDvm);
            table = (HashTable *)*(u4*)(gDvm + LOADED_CLASSES_OFFSET);
            if(table == NULL){
                LOGD("[E] %s > Unable to get HashTable! exit!", __FUNCTION__);
                DumpMemory(gDvm, 0x300);
                return 0;
            }
            
            LOGD("[I] %s > HashTable = 0x%08x", __FUNCTION__, table);
            
            hash_size = table->tableSize;
            entry = table->pEntries;
            if(entry == NULL){
                LOGD("[E] %s > Unable to get entry of HashTable! exit!", __FUNCTION__);
                return 0;
            }
            
            LOGD("[I] %s > HashTableSize = 0x%x, Entry = 0x%08x", __FUNCTION__, hash_size, entry);
            
            for(i = 0; i < hash_size; i++){
                if(entry[i].data != NULL){
                    u1 * clazz = (u1*)entry[i].data;
                    
                    char * name = (char *)*(u4*)(clazz + NAME_OF_CLASS_OFFSET);
                    if(name != NULL){
                        if(strcmp(name, "Lcom/crackme/MainActivity;") == 0){
                            LOGD("[I] %s > Class > 0x%08x - %s", __FUNCTION__, clazz, name);
                            MethodList = (Method*)*(u4*)(clazz + METHOD_OF_CLASS_OFFSET);
                            if(MethodList == NULL){
                                LOGD("[E] %s > Unable to get MethodList! exit!", __FUNCTION__, dlerror());
                                DumpMemory(clazz, 0x100);
                                return 0;
                            }
                            MethodNum = *(u4*)(clazz + METHOD_OF_CLASS_OFFSET - 4);
                        }
                    }
                    
                }
            }
            
        }
             
        LOGD("[I] %s > MethodList = 0x%08x, MethodNum = %d", __FUNCTION__, MethodList, MethodNum);
        if(MethodList == NULL){
            return -1;
        }
        i = 0;
        for(i = 0; i < MethodNum; i++){
            LOGD("[I] %s > Method > 0x%08x : %s", __FUNCTION__, MethodList[i].insns, MethodList[i].name);
            if(strcmp(MethodList[i].name, "crackme") == 0){
                LOGD("[I] %s > Native Method > 0x%08x : %s", __FUNCTION__, MethodList[i].jniArgInfo, MethodList[i].name);
                return 0;
            }
        }
    }
}
void * new_entry(void * param)
{
    while(do_task()){
        sleep(3);
    }

    DumpSo("libcrackme.so");
    DumpDex("classes.dex");
}


void * entry(void * param)
{
    LOGD("Hook success, pid = %d\n", getpid());
//    pthread_t thread;
//    pthread_create(&thread, NULL, new_entry, NULL);
    return 0;
}
