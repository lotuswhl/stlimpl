#pragma once

namespace lotus{

template<typename InputIterator,typename Tp>
Tp accumulate(InputIterator beg,InputIterator end,Tp init){
    for(;beg!=end;++beg){
        init += *beg;
    }
    return init;
}

template<typename InputIterator,typename Tp,typename BiOp>
Tp accumulate(InputIterator beg,InputIterator end,Tp init,BiOp op){
    for(;beg != end;++beg){
        op(init,*beg);
    }
    return init;
}


}
