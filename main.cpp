#include <cstdlib>
#include <cstring>
#include <string>
#include <tuple>
#include <iostream>
#include <fstream>
#include <functional>
#include "json.hpp"

using namespace std;

void FetchVideoComment(int VideoID,int Page,const string& SaveFile)
{
    string command="C:\\curl -k \"https://api.bilibili.com/x/v2/reply?jsonp=jsonp&pn="s+
        to_string(Page)+"&type=1&oid="+to_string(VideoID)+"&sort=0\" > "+SaveFile;
    cout<<"Executing: "<<command<<endl;
    system(command.c_str());
}

tuple<int,int> GetCommentPageCount(const string& SavedFile)
{
    ifstream ifs(SavedFile);
    string line;
    getline(ifs,line);
    nlohmann::json jobj=nlohmann::json::parse(line);
    auto jobjDataIter=jobj.find("data");
    auto jobjData_PageIter=jobjDataIter->find("page");
    auto jobjData_Page_CountIter=jobjData_PageIter->find("count");
    auto jobjData_Page_SizeIter=jobjData_PageIter->find("size");
    return make_tuple(*jobjData_Page_CountIter,*jobjData_Page_SizeIter);
}

void GetCommentFromFile(const string& SavedFile,vector<string>& vec)
{
    ifstream ifs(SavedFile);
    string line;
    getline(ifs,line);
    nlohmann::json jobj=nlohmann::json::parse(line);
    auto jobjDataIter=jobj.find("data");
    auto jobjData_RepliesIter=jobjDataIter->find("replies");
    /// Check if Replies is empty
    if(jobjData_RepliesIter==jobjDataIter->end() ||
       jobjData_RepliesIter->empty() )
    {
        /// no reply.
        return;
    }
    else
    {
        for(auto jobjData_Replies_StepIter=jobjData_RepliesIter->begin();
        jobjData_Replies_StepIter!=jobjData_RepliesIter->end();
        ++jobjData_Replies_StepIter)
        {
            auto thisReplyContentIter=jobjData_Replies_StepIter->find("content");
            auto thisReplyContent_MessageIter=thisReplyContentIter->find("message");
            auto thisReplyMemberIter=jobjData_Replies_StepIter->find("member");
            auto thisReplyMember_UNameIter=thisReplyMemberIter->find("uname");

            ostringstream ostr;
            ostr<<(*thisReplyMember_UNameIter)<<": "<<(*thisReplyContent_MessageIter);
            vec.push_back(ostr.str());

            auto thisReplySubReply=jobjData_Replies_StepIter->find("replies");
            if(thisReplySubReply==jobjData_Replies_StepIter->end() ||
               thisReplySubReply->empty())
            {
                /// No SubReply.
            }
            else
            {
                /// Push SubReply
                for(auto thisReplySubReply_StepIter=thisReplySubReply->begin();
                thisReplySubReply_StepIter!=thisReplySubReply->end();
                ++thisReplySubReply_StepIter)
                {
                    auto thisSubReplyContentIter=thisReplySubReply_StepIter->find("content");
                    auto thisSubReplyContent_MessageIter=thisSubReplyContentIter->find("message");
                    auto thisSubReplyMemberIter=thisReplySubReply_StepIter->find("member");
                    auto thisSubReplyMember_UNameIter=thisSubReplyMemberIter->find("uname");

                    ostringstream ostr;
                    ostr<<(*thisSubReplyMember_UNameIter)<<": "<<(*thisSubReplyContent_MessageIter);
                    vec.push_back(ostr.str());
                }
            }
        }
    }
}

int main()
{
    int avid=5158497;


    FetchVideoComment(avid,1,"save1.txt");
    int cnt,cntperpage;
    tie(cnt,cntperpage)=GetCommentPageCount("save1.txt");

    int needPage=cnt/cntperpage+(cnt%cntperpage!=0?1:0);

    cout<<"Totally "<<cnt<<" Comments. "<<endl;

    vector<string> vec;
    GetCommentFromFile("save1.txt",vec);

    for(int i=2;i<=needPage;i++)
    {
        FetchVideoComment(avid,i,"save"s+to_string(i)+".txt");
        GetCommentFromFile("save"s+to_string(i)+".txt",vec);
    }

    ofstream ofs("out.txt");

    for(auto& str:vec)
    {
        ofs<<str<<endl;
    }

    return 0;
}
