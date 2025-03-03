#include"searcher.hpp"
#include"cpp-httplib/httplib.h"
const std::string root_path="./wwwroot";//web根目录的路径
const std::string path="./data/output/raw.txt";
int main()
{
    ns_searcher::Searcher sea;
    sea.InitSearcher(path);
    httplib::Server svr;
    svr.set_base_dir(root_path.c_str());//设置默认界面（没有Get方法时），一般是web根目录下的index.html，这里已经准备好了
    svr.Get("/s",[&sea](const httplib::Request &req,httplib::Response&res){
        if(!req.has_param("word"))
        {
            res.set_content("必须要有搜索关键字!","text/plain; charset=utf-8");
            return;
        }
        std::string word=req.get_param_value("word");
        LOG(1,"用户在搜索： "+word);
        std::string json_string;
        sea.Search(word,&json_string);
        res.set_content(json_string,"appliaction/json");
    });
    LOG(1,"服务器启动成功");
    svr.listen("0.0.0.0",5566);//表示监听任意IP，端口为5566的请求
    return 0;
}