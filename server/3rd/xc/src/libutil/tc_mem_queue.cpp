#include "util/tc_mem_queue.h"
#include <string.h>
#include <cassert>
#include <limits>
#include <string>
#include <iostream>

namespace xutil
{

XC_MemQueue::XC_MemQueue()
:_pctrlBlock(NULL)
,_paddr(NULL)
,_size(0)
{
}

void XC_MemQueue::create(void *pAddr, size_t iSize)
{
    assert(pAddr);
    assert(iSize != 0);

    _size           = iSize;
    _pctrlBlock     = (CONTROL_BLOCK*)pAddr;
    _pstModifyHead  = static_cast<tagModifyHead*>((void*)((char*)pAddr + sizeof(CONTROL_BLOCK)));
    _paddr          = (void *)((char *)_pctrlBlock + sizeof(CONTROL_BLOCK) + sizeof(tagModifyHead));

    _pctrlBlock->iMemSize   = iSize;
    _pctrlBlock->iTopIndex  = 0;
    _pctrlBlock->iBotIndex  = 0;
    _pctrlBlock->iPopCount  = 0;
    _pctrlBlock->iPushCount = 0;
}

void XC_MemQueue::connect(void *pAddr, size_t iSize)
{
    assert(pAddr);

    _pctrlBlock     = (CONTROL_BLOCK*)pAddr;

    assert(iSize == _pctrlBlock->iMemSize);

    _size           = _pctrlBlock->iMemSize;
    _pstModifyHead  = static_cast<tagModifyHead*>((void*)((char*)pAddr + sizeof(CONTROL_BLOCK)));
    _paddr          = (void *)((char *)_pctrlBlock + sizeof(CONTROL_BLOCK) + sizeof(tagModifyHead));
}

bool XC_MemQueue::isFull(size_t iSize)
{
    doUpdate();

    size_t iTopIndex = _pctrlBlock->iTopIndex;
    size_t iBotIndex = _pctrlBlock->iBotIndex;

    if(iTopIndex < iBotIndex)
    {
        return (iTopIndex + sizeof(size_t) + iSize >= iBotIndex);
    }
    else if(iTopIndex > iBotIndex)
    {
        return iTopIndex + sizeof(size_t) + iSize >= iBotIndex + queueSize();
    }

    return false;
}

bool XC_MemQueue::isEmpty()
{
    doUpdate();
    return (_pctrlBlock->iTopIndex == _pctrlBlock->iBotIndex);
}

size_t XC_MemQueue::queueSize()
{
    doUpdate();
    return _size - sizeof(CONTROL_BLOCK) - sizeof(tagModifyHead);
}

size_t XC_MemQueue::elementCount()
{
    doUpdate();
    size_t iPushCount = _pctrlBlock->iPushCount;
    size_t iPopCount  = _pctrlBlock->iPopCount;

    if(iPushCount >= iPopCount)
    {
        return iPushCount - iPopCount;
    }

    return numeric_limits<size_t>::max() - iPopCount + iPushCount;
}

bool XC_MemQueue::pop_front(string &sOut)
{
    if(isEmpty())
    {
        return false;
    }

    doUpdate();

    //大小没有分隔
    if(_pctrlBlock->iBotIndex + sizeof(size_t) <= queueSize())
    {
        size_t iSize;
        memcpy((char*)&iSize, (const char*)_paddr + _pctrlBlock->iBotIndex, sizeof(size_t));

        sOut.reserve(iSize);

        //文件没有分隔
        if(_pctrlBlock->iBotIndex + sizeof(size_t) + iSize <= queueSize())
        {
            sOut.assign((char*)_paddr + _pctrlBlock->iBotIndex + sizeof(size_t), iSize);

            update(&_pctrlBlock->iBotIndex, _pctrlBlock->iBotIndex + sizeof(size_t) + iSize);

        }
        else
        {
            //前面的数据长度
            size_t iLeftSize = queueSize() -  _pctrlBlock->iBotIndex - sizeof(size_t);

            sOut.assign((char*)_paddr + _pctrlBlock->iBotIndex + sizeof(size_t), iLeftSize);
            sOut.append((char*)_paddr, iSize - iLeftSize);

            update(&_pctrlBlock->iBotIndex, iSize - iLeftSize);
        }
    }
    else
    {
        size_t iSize;

        size_t iLeftSize = queueSize() -  _pctrlBlock->iBotIndex;

        memcpy((char*)&iSize, (const char*)_paddr + _pctrlBlock->iBotIndex, iLeftSize);
        memcpy((char*)&iSize + iLeftSize, (const char*)_paddr, sizeof(size_t) - iLeftSize);

        sOut.assign((char*)_paddr + sizeof(size_t) - iLeftSize, iSize);

        update(&_pctrlBlock->iBotIndex, iSize + sizeof(size_t) - iLeftSize);
    }

    update(&_pctrlBlock->iPopCount, _pctrlBlock->iPopCount + 1);

    doUpdate(true);

    return true;
}

bool XC_MemQueue::push_back(const string &sIn)
{
    return push_back(sIn.c_str(),sIn.length());
}

bool XC_MemQueue::push_back(const char *pvIn, size_t iSize)
{
    if(isFull(iSize))
    {
        return false;
    }

    doUpdate();

    //没有跨越共享内存边界
    if(_pctrlBlock->iTopIndex + sizeof(size_t) + iSize <= queueSize())
    {
        memcpy((char*)_paddr + _pctrlBlock->iTopIndex, (const char*)&iSize, sizeof(size_t));
        memcpy((char*)_paddr + _pctrlBlock->iTopIndex + sizeof(size_t), (const char*)pvIn, iSize);

        update(&_pctrlBlock->iTopIndex, _pctrlBlock->iTopIndex + sizeof(size_t) + iSize);

    }
    //跨越了共享内存边界
    else
    {
        //共享内存剩下的大小
        size_t iLeftSize = queueSize() - _pctrlBlock->iTopIndex;

        //iSize跨越了共享内存边界
        if(iLeftSize < sizeof(size_t))
        {
            if(iLeftSize > 0)
            {
                memcpy((char*)_paddr + _pctrlBlock->iTopIndex, (const char*)&iSize, iLeftSize);
            }

            memcpy((char*)_paddr, (const char*)&iSize + iLeftSize, sizeof(size_t) - iLeftSize);
            memcpy((char*)_paddr + sizeof(size_t) - iLeftSize, (const char*)pvIn, iSize);

            update(&_pctrlBlock->iTopIndex, iSize + sizeof(size_t) - iLeftSize);

        }
        else
        {
            //iSize没有跨越共享内存边界
            memcpy((char*)_paddr + _pctrlBlock->iTopIndex, (const char*)&iSize, sizeof(size_t));

            size_t iLeftSize1 = iLeftSize - sizeof(size_t);
            if(iLeftSize1 > 0)
            {
                memcpy((char*)_paddr + _pctrlBlock->iTopIndex + sizeof(size_t), (const char*)pvIn, iLeftSize1);
            }

            memcpy((char*)_paddr, (const char*)pvIn + iLeftSize1, iSize - iLeftSize1);

            update(&_pctrlBlock->iTopIndex, iSize - iLeftSize1);

        }
    }

    update(&_pctrlBlock->iPushCount, _pctrlBlock->iPushCount + 1);

    doUpdate(true);

    return true;
}

void XC_MemQueue::doUpdate(bool bUpdate)
{
	if(bUpdate)
	{
		_pstModifyHead->_cModifyStatus = 2;
	}

	//==1, copy过程中, 程序失败, 需要清除状态
	if(_pstModifyHead->_cModifyStatus == 1)
	{
		_pstModifyHead->_iNowIndex        = 0;
		for(size_t i = 0; i < sizeof(_pstModifyHead->_stModifyData) / sizeof(tagModifyData); i++)
		{
			_pstModifyHead->_stModifyData[i]._iModifyAddr       = 0;
			_pstModifyHead->_stModifyData[i]._cBytes            = 0;
			_pstModifyHead->_stModifyData[i]._iModifyValue      = 0;
		}
		_pstModifyHead->_cModifyStatus    = 0;
	}
	//==2, 已经修改成功, 但是没有copy到内存中, 需要更新到内存中
	else if(_pstModifyHead->_cModifyStatus == 2)
	{
		for(size_t i = 0; i < _pstModifyHead->_iNowIndex; i++)
		{
			if(_pstModifyHead->_stModifyData[i]._cBytes == sizeof(size_t))
			{
				*(size_t*)((char*)_pctrlBlock + _pstModifyHead->_stModifyData[i]._iModifyAddr) = _pstModifyHead->_stModifyData[i]._iModifyValue;
			}
			else if(_pstModifyHead->_stModifyData[i]._cBytes == sizeof(bool))
			{
				*(bool*)((char*)_pctrlBlock + _pstModifyHead->_stModifyData[i]._iModifyAddr) = (bool)_pstModifyHead->_stModifyData[i]._iModifyValue;
			}
			else
			{
				assert(true);
			}
		}
		_pstModifyHead->_iNowIndex        = 0;
		_pstModifyHead->_cModifyStatus    = 0;
	}
	//==0, 正常状态
	else if(_pstModifyHead->_cModifyStatus == 0)
	{
		return;
	}
}

void XC_MemQueue::update(void* iModifyAddr, size_t iModifyValue)
{
	_pstModifyHead->_cModifyStatus = 1;
	_pstModifyHead->_stModifyData[_pstModifyHead->_iNowIndex]._iModifyAddr  = (char*)iModifyAddr - (char*)_pctrlBlock;
	_pstModifyHead->_stModifyData[_pstModifyHead->_iNowIndex]._iModifyValue = iModifyValue;
	_pstModifyHead->_stModifyData[_pstModifyHead->_iNowIndex]._cBytes       = sizeof(iModifyValue);
	_pstModifyHead->_iNowIndex++;

	assert(_pstModifyHead->_iNowIndex < sizeof(_pstModifyHead->_stModifyData) / sizeof(tagModifyData));
}

void XC_MemQueue::update(void* iModifyAddr, bool bModifyValue)
{
	_pstModifyHead->_cModifyStatus = 1;
	_pstModifyHead->_stModifyData[_pstModifyHead->_iNowIndex]._iModifyAddr  = (char*)iModifyAddr - (char*)_pctrlBlock;
	_pstModifyHead->_stModifyData[_pstModifyHead->_iNowIndex]._iModifyValue = bModifyValue;
	_pstModifyHead->_stModifyData[_pstModifyHead->_iNowIndex]._cBytes       = sizeof(bModifyValue);
	_pstModifyHead->_iNowIndex++;

	assert(_pstModifyHead->_iNowIndex < sizeof(_pstModifyHead->_stModifyData) / sizeof(tagModifyData));
}

}

