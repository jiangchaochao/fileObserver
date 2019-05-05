#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <jni.h>
#include<android/log.h>

#define TAG "fileobserver"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型

#define MAX_FILES 1000
#define EPOLL_COUNT 1000
#define MAXCOUNT 500

jclass gl_class;                            /*类*/
JavaVM *gl_jvm;                             /*java虚拟机*/
jobject gl_object;                          /*引用类型*/



int RUN = 1;


char *pathName[4096] = {NULL};      //save fd--->path name
int inotifyWd[4096] = {-1};        //保存 inotify_add_watch 返回值，删除的时候需要

char monitorPath[1024]={0};       //用来保存监听的目录


static char *epoll_files[MAX_FILES];

static struct epoll_event mPendingEventItems[EPOLL_COUNT];

int mINotifyFd,mEpollFd,i;

char inotifyBuf[MAXCOUNT];

char epollBuf[MAXCOUNT];

typedef struct t_name_fd {
	int fd;
	char name[30];

}  T_name_fd;


T_name_fd  t_name_fd[100];
int count_name_fd;



int getfdFromName(char* name)
{
	int i;
	for(i=0; i<MAX_FILES; i++)
	{
		if (!epoll_files[i])
			continue;

		if(0 == strcmp(name,epoll_files[i]))
		{
			return i;
		}
	}
	return -1;
}


/**
 *
 *create event
 */
void CreateEvent(JNIEnv *env, jclass cls, char *path)
{
    jmethodID jmethodid = NULL;
    jmethodid = (*env)->GetStaticMethodID(env, cls, "CreateEvent", "(Ljava/lang/String;)V");
    if (jmethodid == NULL)
    {
        LOGE("create event   jmethodid == null");
        return ;
    }
    jstring str = (*env)->NewStringUTF(env, path);
    (*env)->CallStaticVoidMethod(env, cls, jmethodid, str);

    /*delete local reference*/
    (*env)->DeleteLocalRef(env, str);
}

/**
 *
 *delete event
 *
 */
void DeleteEvent(JNIEnv *env, jclass cls,char *path)
{
    jmethodID jmethodid = NULL;
    jmethodid = (*env)->GetStaticMethodID(env, cls, "DeleteEvent", "(Ljava/lang/String;)V");
    if (jmethodid == NULL)
    {
        LOGE("delete event   jmethodid == null");
        return ;
    }

    jstring str = (*env)->NewStringUTF(env, path);
    (*env)->CallStaticVoidMethod(env, cls, jmethodid, str);

    /*delete local reference*/
    (*env)->DeleteLocalRef(env, str);
}



struct inotify_event*  curInotifyEvent;
char name[30];
int readCount = 0;
int fd;


void scan_dir(const char *dir, int depth)   // 定义目录扫描函数
{
	DIR *dp;                      // 定义子目录流指针
	struct dirent *entry;         // 定义dirent结构指针保存后续目录
	struct stat statbuf;          // 定义statbuf结构保存文件属性
	struct epoll_event eventItem; //epoll event
	struct inotify_event  inotifyEvent;//inotify event
	int lnotifyFD;
	int lwd;  //inotify_add_watch 返回值
	char path[1024] = {0};
	if((dp = opendir(dir)) == NULL) // 打开目录，获取子目录流指针，判断操作是否成功
	{
		LOGE("can't open dir  ------> %d\n", errno);
		return;
	}
	chdir (dir);                     // 切换到当前目录
	while((entry = readdir(dp)) != NULL)  // 获取下一级目录信息，如果未否则循环
	{
		lstat(entry->d_name, &statbuf); // 获取下一级成员属性
		if(S_IFDIR &statbuf.st_mode)    // 判断下一级成员是否是目录
		{
			if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
				continue;
			//printf("%*s%s/\n", depth, "", entry->d_name);  // 输出目录名称
			char *path1 = realpath("./", NULL);
			if (NULL != path1)
			{
				lnotifyFD = inotify_init();

				sprintf(path, "%s/%s", path1, entry->d_name);   //get absolute path
				lwd = inotify_add_watch(lnotifyFD, path, IN_DELETE | IN_CREATE);//监听xxx目录下的 delete、create事件
				if (-1 == lwd)
				{
				    LOGE("-1 == LWD\n");
					continue;
				}
				eventItem.events = EPOLLIN;
				eventItem.data.fd = lnotifyFD;
				epoll_ctl(mEpollFd, EPOLL_CTL_ADD, lnotifyFD, &eventItem);  //add to epoll
				if (lnotifyFD < 4096)
				{
					char *p = malloc(strlen(path) + 1);
					memset(p, 0, strlen(path) + 1);
					if (NULL != p)
					{
						memcpy(p, path, strlen(path));
						pathName[lnotifyFD] = p;
						inotifyWd[lnotifyFD] = lwd;
					}
				}
				free(path1);
			}
			scan_dir(entry->d_name, depth+4);              // 递归调用自身，扫描下一级目录的内容
		}
	}
	chdir("..");                                                  // 回到上级目录
	closedir(dp);                                                 // 关闭子目录流
}

char creatPath[1024] = {0};
char deletePath[1024] = {0};
int a = -1;
int *fileObserver_init(const char *path)
{
    int i = 0;
	struct epoll_event eventItem;        //epoll event
	struct inotify_event  inotifyEvent;  //inotify event
	JNIEnv *env;
	jmethodID jmethodid = NULL;

	if (gl_jvm == NULL)
    {
    	LOGE("gl_jvm is NULL");
    	return (int *)-1;
    }
	(*gl_jvm)->AttachCurrentThread(gl_jvm, &env, NULL);
	//0. add sub dir inotify
	if (NULL == path)
	{
	    LOGE("path == null \n");
		a = -1;
		return &a;
	}
	mEpollFd = epoll_create(1000);
	// 1. init inotify &  epoll
	int homeINotifyFd = inotify_init();
	char *p = malloc(strlen(path));
	if (NULL == p)
	{
	    LOGE("malloc failed  = NULL \n");
		a = -1;
		return &a;
	}
	memset(p, 0, strlen(path));
	memcpy(p, path, strlen(path));
	pathName[homeINotifyFd] = p;
//	LOGE("pathName[homeINotifyFd] = %s\n", pathName[homeINotifyFd]);
	// 2.add inotify watch dir
	int lwd = inotify_add_watch(homeINotifyFd, pathName[homeINotifyFd], IN_DELETE | IN_CREATE);//监听xxx目录下的 delete、create事件
	if (-1 == lwd)
	{
	    LOGE(" inotify_add_watch  -------> errno = %d\n", errno);
		return &a;
	}
	inotifyWd[homeINotifyFd] = lwd;
	// 3. add inotify fd to epoll
	eventItem.events = EPOLLIN;
	eventItem.data.fd = homeINotifyFd;
	epoll_ctl(mEpollFd, EPOLL_CTL_ADD, homeINotifyFd, &eventItem);
	scan_dir(path, 0);
	while(RUN)
	{
		// 4.epoll检测文件的可读变化
		int pollResult = epoll_wait(mEpollFd, mPendingEventItems, EPOLL_COUNT, -1);
		for(i=0; i <pollResult; i++)
		{
			readCount  = 0;
			readCount = read(mPendingEventItems[i].data.fd,inotifyBuf,MAXCOUNT);

			if (readCount <  sizeof(inotifyEvent))
			{
				LOGE("error inotify event \n");
				return &a;
			}

			// cur 指针赋值
			curInotifyEvent = (struct inotify_event*)inotifyBuf;

			while(readCount >= sizeof(inotifyEvent))
			{
				if (curInotifyEvent->len > 0)
				{
					if(curInotifyEvent->mask & IN_CREATE)
					{
						if (pathName[mPendingEventItems[i].data.fd] != NULL)
						{
							memset(creatPath, 0, sizeof(creatPath));
							sprintf(creatPath, "%s/%s", pathName[mPendingEventItems[i].data.fd], curInotifyEvent->name);
						//	LOGE("create event path = %s\n", creatPath);
							CreateEvent(env, gl_class,creatPath);
						}
						else
						{
							LOGE("create name[mPendingEventItems[i].data.fd] == NULL\n");
						}

					}
					else if(curInotifyEvent->mask & IN_DELETE)
					{
						if (pathName[mPendingEventItems[i].data.fd] != NULL)
						{
							memset(deletePath, 0, sizeof(deletePath));
							sprintf(deletePath, "%s/%s", pathName[mPendingEventItems[i].data.fd], curInotifyEvent->name);
						//	LOGE("delete event path = %s\n", deletePath);
							DeleteEvent(env, gl_class,deletePath);
						}else
						{
							LOGE("delete name[mPendingEventItems[i].data.fd] == NULL\n");
						}

					}
				}
				curInotifyEvent --;
				readCount -= sizeof(inotifyEvent);
			}
		}
	}
    (*gl_jvm)->DetachCurrentThread(gl_jvm);
     LOGE("退出");
	return 0;
}



/**
 *
 *释放
 * 
 */
int FileObserverDestroy()
{
	int i = 0;
	for (i = 0; i < 4096; i ++)     //这里释放inotify的fd和申请的内存
	{
		if (pathName[i] != NULL)
		{
			RUN = 0;
			free(pathName[i]);
			inotify_rm_watch(i, inotifyWd[i]);
		}
	}

	return 0;
}

pthread_t thread_1 = -1;

int FileObserverInit(const char *path)
{
	if (-1 == thread_1)
	{
		pthread_create(&thread_1, NULL, (void * (*)(void *))fileObserver_init, path);
	}
	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_jiangc_fileobserver_FileObserverJni
 * Method:    FileObserverInit
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_jiangc_receiver_FileObserverJni_FileObserverInit(JNIEnv *env, jclass clazz, jstring path)
  {
        const char *str = (*env)->GetStringUTFChars(env, path, 0);
        memset(monitorPath, 0, sizeof(monitorPath));
        memcpy(monitorPath, str, strlen(str));

        /*获取全局的JavaVM以及object*/
        (*env)->GetJavaVM(env, &gl_jvm);
         if (NULL == gl_jvm)
         {
         	LOGE("gl_jvm = NULL");
         }
        gl_class = (*env)->NewGlobalRef(env, clazz);
        LOGE("");
        FileObserverInit(monitorPath);
        (*env)->ReleaseStringUTFChars(env, path, str);
        return 0;
  }

/*
 * Class:     com_jiangc_fileobserver_FileObserverJni
 * Method:    FileObserverDestroy
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_jiangc_receiver_FileObserverJni_FileObserverDestroy(JNIEnv *env, jclass cls){
        (*env)->DeleteGlobalRef(env, gl_class);  //释放全局的object
        FileObserverDestroy();
        return 0;
  }

#ifdef __cplusplus
}
#endif