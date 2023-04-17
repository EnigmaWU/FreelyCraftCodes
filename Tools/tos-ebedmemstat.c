/**
 * @file tos-ebedmemstat.c
* 【tos-ebedmemstat】：自制内存状态监测工具，运行时（总览 + 进程 + 内核），用量和分布
* 【默认：无参数】=【-S：统计模式】
    * 【Overview】：Total/Used/Free/Shared/Buffer/Cached/Available（=Free）
    * 【USpace】：
        * eachProcess（USS/PSS/RSS/VMZ=Program/Process          + Heap/MMAP/Stack + SharedObject/FileMMAP）
            * Kernel           = Task Stack + Socket Buffer + File Buffer/Cache
    * 【KSpace】：
        * Kernel = Static/Image +          Slab/<un>Reclaimable + Stack + PageTables
        * Module = Static/File +          kmalloc
* 【显式：传参数】
    * -U：用户态 vs -K：内核态（默认：U+K）
    * -P：哪个进程， 多个‘-P’表示哪几个进程
    * -T：跟踪模式（vs -S：统计模式），部分数据原本来自统计数据，切换为来自实时跟踪（Trace），以此提高监测精度
        * -TT/-TTT：实时跟踪数据的程度（比例），程度越深，精度越高，消耗越多、干扰增大
    * -Dn：细节程度（n=1/2/3）
    * -Cn：持续模式，间隔（n=1、2、…seconds）输出信息
 * 
 */

int main(int argc, char *argv[])
{

}
