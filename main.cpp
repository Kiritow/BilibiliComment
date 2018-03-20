#include <cstdlib>
#include <string>
#include <tuple>
#include <iostream>
#include <fstream>
#include <functional>
#include "json.hpp"
#include "NetworkWrapper.h"

using namespace std;

string FetchVideoComment(int VideoID,int Page)
{
	HTTPConnection t;
	t.setURL("https://api.bilibili.com/x/v2/reply?jsonp=jsonp&pn="s +
		to_string(Page) + "&type=1&oid=" + to_string(VideoID) + "&sort=0");
	t.setDataOutputBuffer(nullptr, 0);
	t.perform();
	string ret = static_cast<const char*>(t.getDataOutputBuffer());
	return ret;
}

tuple<int,int> GetCommentPageCount(const string& raw_comment)
{
    nlohmann::json jobj=nlohmann::json::parse(raw_comment);
    auto jobjDataIter=jobj.find("data");
    auto jobjData_PageIter=jobjDataIter->find("page");
    auto jobjData_Page_CountIter=jobjData_PageIter->find("count");
    auto jobjData_Page_SizeIter=jobjData_PageIter->find("size");
    return make_tuple(*jobjData_Page_CountIter,*jobjData_Page_SizeIter);
}

void ParseComment(const string& raw_comment,vector<string>& vec)
{
    nlohmann::json jobj=nlohmann::json::parse(raw_comment);
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

int ProcessVideo(int avid,const string& savefile)
{
	string str=FetchVideoComment(avid, 1);
    int cnt,cntperpage;
    tie(cnt,cntperpage)=GetCommentPageCount(str);

    int needPage=cnt/cntperpage+(cnt%cntperpage!=0?1:0);

    cout<<"Totally "<<cnt<<" Comments in "<<needPage<<" pages."<<endl;

    vector<string> vec;
    ParseComment(str,vec);

    for(int i=2;i<=needPage;i++)
    {
		cout << "Fetching page: " << i << endl;
		ParseComment(FetchVideoComment(avid, i),vec);
    }

    ofstream ofs(savefile);

    for(auto& str:vec)
    {
        ofs<<str<<endl;
    }

	return 0;
}

int main()
{
	ProcessVideo(5158497, "out.txt");
	return 0;
}
