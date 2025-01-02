#pragma once
#ifdef _WIN32
    #include <io.h>
    #include <direct.h>
#else
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>
    #include <sys/types.h>

#endif // _WIN32




#ifdef _WIN32
#define CP_access(file,mode) _access(file,mode)
#define CP_local_time(time,tm) localtime_s(&tm, &time)
#define CP_mkdir(dir) _mkdir(dir)
#define CP_remove(file) remove(file)


#else
#define CP_access(file,mode) access(file,mode)
#define CP_local_time(time,tm) localtime_r(&time, &tm)
#define CP_mkdir(dir) mkdir(dir, 775)


#endif // _WIN32

