#include <vector>
#include <iostream>
#include <queue>
#include <stdlib.h>

using namespace std;


class Solution{
public:
    auto getKth(const vector<int>& a,int k){
        std::priority_queue<int,vector<int>,greater<int>> pq;
        for(int i:a){
            if(pq.size()<k)pq.push(i);
            else if(pq.top()<i){
                pq.pop();
                pq.push(i);
            }
        }
        return pq;
    }

    double median_of_unsorted_array(const vector<int>& a){
        int size = (int)a.size();
        if(size == 0)return 0;
        if(size == 1)return a[0];
        if(size == 2)return (1.0*a[0]+a[1])/2;
       
        auto pq = getKth(a,size/2+1);
        if(size&1==1)return pq.top();
        else {
            int m1 = pq.top();pq.pop();
            int m2 = pq.top();
            return (1.0*m1+m2)/2;
        }

    }
};




int main(int argc,char ** argv){
    if(argc<2)return 0;
    vector<int> a;
    for(int i=1;i<argc;++i){
        a.push_back(atoi(argv[i]));
    }

    cout << Solution().median_of_unsorted_array(a) << endl;

}
