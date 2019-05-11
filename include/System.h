#include <iostream>
#include <memory>
#include <sstream>
#include <queue>
#include <functional>

#include "json/json.h"
#include "utils.h"
#include "Camera.h"
#include "Detector.h"
#include "Websocket_server.h"
#include "http_client.h"
#include "Serial.h"
#include "Log.hpp"

class System
{

struct WorkInfo
{
    int64_t id;
    int page;
    bool reprint_flag = false;
};

struct OriginInfo{
    string location;
    int quantity;
};


private:
    // 批次信息
    std::deque<std::string>     work_name_list_;
    std::deque<int>         work_count_list_;
    std::deque<std::string>     batch_origin_list_;
    std::deque<int>         batch_count_list_;

    std::vector<WorkInfo>       work_list_;
    std::vector<OriginInfo>     origin_list_;

    std::queue<string> unsolved_list_;

    // 服务器
    std::string client_host_;
    int         server_port_;
    int         client_port_;
    std::string result_url_;

    // 当前作业信息
    int64_t cur_id_;        // 当前的JobId
    int cur_page_;          // 当前所检测页
    int cur_index_;         // 当前的列表下标
    int totla_page_;        // 总页数

    // 线程
    pthread_t post_thread_id_ = 3;
	pthread_t server_thread_id_ = 4;
    pthread_mutex_t slv_list_mutex_;
    pthread_mutex_t result_mutex_;
    pthread_mutex_t work_list_mutex_;
    
    Json::Value* result_root_;

    Serial result_serial_;
    Camera* camera;
    Detector* detector;
    std::shared_ptr<WebsocketServer> server_;

    int readNetParam();

    int downloadFileList(std::vector<string> file_list);

    std::string getJsonWorkList();
    std::string getJsonOriginList();

    void postResult();
    void startServer();
    
    static void* postResultWrapper(void *arg);
    static void* startServerWrapper(void *arg);
    bool handle_signal(std::string url, std::string body, mg_connection *c, OnRspCallback rsp_callback);
    bool handle_result(std::string url, std::string body, mg_connection *c, OnRspCallback rsp_callback);
    bool handle_setTask(std::string url, std::string body, mg_connection *c, OnRspCallback rsp_callback);
    bool handle_start(std::string url, std::string body, mg_connection *c, OnRspCallback rsp_callback);
    bool handle_changeMod(std::string url, std::string body, mg_connection *c, OnRspCallback rsp_callback);
    bool handle_softTrigger(std::string url, std::string body, mg_connection *c, OnRspCallback rsp_callback);
    bool handle_websocketConnect();

    void updateState();                                             // 主线程每个运行周期更新当前页数和当前检测作业在列表中的位置
    void popWorkList();                                             // 丢弃过久的列表元素
    void informCurState();                                          // 通知当前正在处理的作业信息
    void informWorkList();                                          // 当列表发生变化时进行处理,通知网页

public:
    System();
    System(int first);
    ~System();

    void init();
    void run();
    
    int getCurInfo(std::string filename, int64_t &id);
    int setCurWork(int index, int64_t jobid, int64_t curpage);     // 修改当前作业

};

