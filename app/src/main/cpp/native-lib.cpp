#include <jni.h>
#include <string>
#include "native-lib.h"


int m_child = 0;
int m_parent = -1;
int user_id = 0;

const char *PATH = "/data/data/com.mg.axechen.socketprocess/my.sock";

extern "C"
JNIEXPORT void JNICALL
Java_com_mg_axechen_socketprocess_Wathcer_createWatcher(JNIEnv *env, jobject instance,
                                                        jint userId) {
    // TODO
    user_id = userId;
    create_child();


}


JNIEXPORT void JNICALL
Java_com_mg_axechen_socketprocess_Wathcer_connectToMonitor(JNIEnv *env, jobject instance) {

    int sockfd;
    while (1) {
        sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
        if (sockfd < 0) {
            continue;
        }
        struct sockaddr_un addr;
        // 先将客户端清零
        memset(&addr, 0, sizeof(sockaddr_un));
        addr.sun_family = AF_LOCAL;
        strcpy(addr.sun_path, PATH);
        if (connect(sockfd, (const sockaddr *) &addr, sizeof(sockaddr_un)) < 0) {
            // 连接失败
            close(sockfd);
            sleep(1);
            continue;
        }
        // 连接成功
        LOGE("连接服务端成功");
        m_parent = sockfd;
        break;
    }


}

void create_child() {
    pid_t pid = fork();
//
    if (pid < 0) {

    } else if (pid > 0) {
//父进程
    } else if (pid == 0) {
        LOGE("子进程开启 ");
//        开启线程轮询
        child_do_work();
    }
}

void child_do_work() {
    // 1、建立socket
    if (child_create_channel()) {
        child_listen_msg();
    }
    // 2、读取消息
}

void child_listen_msg() {
    fd_set rfds;
    while (1) {
        // 清空端口号
        FD_ZERO(&rfds);
        FD_SET(m_child, &rfds);
        //设置超时时间
        struct timeval time_out = {3, 0};
        int r = select(m_child + 1, &rfds, NULL, NULL, &time_out);
        if (r > 0) {
            char pkg[256] = {0};
            // 确保读到的内容
            if (FD_ISSET(m_child, &rfds)) {
                // 客户端写到内容
                read(m_child, pkg, sizeof(pkg));
                // 重启父进程
                LOGE("apk 死亡时发来的信息，重启客户端必要服务");
                LOGE("重启父进程");
                execlp("am", "am", "startservice", "--user", user_id,
                       "com.dongnao.signalprocess/com.dongnao.signalprocess.ProcessService",
                       (char *) NULL);
                break;
            }
        }
    }
}

int child_create_channel() {
    // 创建socket
    int listenfb = socket(AF_LOCAL, SOCK_STREAM, 0);
    // 取消之前进程链接的文件
    unlink(PATH);
    // 绑定端口号
    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(sockaddr_un));

    addr.sun_family = AF_LOCAL;

    if (bind(listenfb, (const sockaddr *) &addr, sizeof(addr)) < 0) {
        LOGE("绑定失败！");
        return 0;
    }
    listen(listenfb, 5);
    int connfd = 0;
    while (1) {
        // 不断接受客户端请求的数据
        // 等待客户端连接
        connfd = accept(listenfb, NULL, NULL);
        if (connfd < 0) {
            if (errno == EINTR) {
                // 连接失败
                continue;
            } else {
                LOGE("读取错误");
            }
        }

        LOGE("进程连接上");
        // apk连接上socket
        m_child = connfd;
        break;
    }

    return 1;
}

