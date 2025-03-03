#include "searcher.hpp"
//如果想在命令行调试就用这个代码
const std::string path = "data/output/raw.txt";
int main()
{
    ns_searcher::Searcher *sea = new ns_searcher::Searcher();
    sea->InitSearcher(path);
    std::string query;
    std::string result;
    while (1)
    { 
        std::cout << "Please Enter Key# ";
        std::getline(std::cin, query);
        sea->Search(query, &result);
        std::cout << result << std::endl;
    }
    return 0;
}