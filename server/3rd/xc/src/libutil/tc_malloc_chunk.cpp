#include "util/tc_malloc_chunk.h"

namespace xutil
{
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	SizeMap Static::_sizemap;

	size_t SizeMap::AlignmentForSize(size_t size)
	{
		size_t alignment = kAlignment;

		if (size > kMaxSize)
		{
			alignment = kPageSize;
		}
		else if (size >= 128)
		{
			alignment = (1 << LgFloor(size)) / 8;
		}
		else if (size >= 16)
		{
			alignment = 16;
		}

		if (alignment > kPageSize)
		{
			alignment = kPageSize;
		}

		return alignment;
	}

	int SizeMap::NumMoveSize(size_t size)
	{
		if (size == 0)
			return 0;
		int num = static_cast<int>(64.0 * 1024.0 / size);
		if (num < 2)
			num = 2;
		if (num > 32)
			num = 32;
		return num;
	}

	void SizeMap::Init()
	{
		if (ClassIndex(0) < 0) 
		{
			assert(false);
		}
		if (ClassIndex(kMaxSize) >= sizeof(class_array_)) 
		{
			assert(false);
		}

		size_t sc = 1;
		size_t alignment = kAlignment;
		for (size_t size = kAlignment; size <= kMaxSize; size += alignment) 
		{
			alignment = AlignmentForSize(size);	

			size_t blocks_to_move = NumMoveSize(size) / 4;
			size_t psize = 0;
			do 
			{
			  psize += kPageSize;

			  while ((psize % size) > (psize >> 3)) 
			  {
				psize += kPageSize;
			  }

			} while ((psize / size) < (blocks_to_move));
			const size_t my_pages = psize >> kPageShift;

			if (sc > 1 && my_pages == class_to_pages_[sc-1]) 
			{
				const size_t my_objects = (my_pages << kPageShift) / size;
				const size_t prev_objects = (class_to_pages_[sc-1] << kPageShift) / class_to_size_[sc-1];
				if (my_objects == prev_objects) 
				{
					class_to_size_[sc-1] = size;
					continue;
				}
			}

			class_to_pages_[sc] = my_pages;
			class_to_size_[sc] = size;
			sc++;
		}
		if (sc != kNumClasses) 
		{
			assert(false);
		}

		int next_size = 0;
		for (size_t c = 1; c < kNumClasses; c++) 
		{
			const int max_size_in_class = class_to_size_[c];
			for (int s = next_size; s <= max_size_in_class; s += kAlignment) 
			{
				class_array_[ClassIndex(s)] = c;
			}
			next_size = max_size_in_class + kAlignment;
		}

		for (size_t size = 0; size <= kMaxSize; size++) 
		{
			const size_t sc = SizeClass(size);
			if (sc <= 0 || sc >= kNumClasses) 
			{
				assert(false);
			}
			if (sc > 1 && size <= class_to_size_[sc-1]) 
			{
				assert(false);
			}
			const size_t s = class_to_size_[sc];
			if (size > s) 
			{
				assert(false);
			}
			if (s == 0) 
			{
				assert(false);
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void XC_SpanAllocator::init(void *pAddr)
	{
		_pHead = static_cast<tagSpanAlloc*>(pAddr);
		_pData = (unsigned char*)((char*)_pHead + sizeof(tagSpanAlloc));
	}

	void XC_SpanAllocator::create(void* pAddr, size_t iSpanSize, size_t iSpanCount)
	{
		assert(iSpanSize > sizeof(size_t));

		init(pAddr);

		_pHead->_iSpanSize             = iSpanSize;
		_pHead->_iSpanCount            = iSpanCount;
		_pHead->_firstAvailableSpan    = 0;
		_pHead->_spanAvailable         = iSpanCount;

		memset(_pData, 0x00, iSpanCount*iSpanSize);

		unsigned char *pt               = _pData;
		for(size_t i = 0; i != iSpanCount; pt += iSpanSize)
		{
			++i;
			memcpy(pt, &i, sizeof(size_t));
		}
	}

	void XC_SpanAllocator::connect(void *pAddr)
	{
		init(pAddr);
	}

	void* XC_SpanAllocator::allocate()
	{
		if(!isSpanAvailable()) return NULL;

		unsigned char *result        = _pData + (_pHead->_firstAvailableSpan * _pHead->_iSpanSize);

		--_pHead->_spanAvailable;

		_pHead->_firstAvailableSpan = *((size_t *)result);

		memset(result, 0x00, sizeof(_pHead->_iSpanSize));

		return result;
	}

	void XC_SpanAllocator::deallocate(void *pAddr)
	{
		assert(pAddr >= _pData);

		unsigned char* prelease = static_cast<unsigned char*>(pAddr);

		assert((prelease - _pData) % _pHead->_iSpanSize == 0);

		memset(pAddr, 0x00, _pHead->_iSpanSize);

		*((size_t *)prelease) = _pHead->_firstAvailableSpan;

		_pHead->_firstAvailableSpan = static_cast<size_t>((prelease - _pData)/_pHead->_iSpanSize);

		assert(_pHead->_firstAvailableSpan == (prelease - _pData)/_pHead->_iSpanSize);

		++_pHead->_spanAvailable;
	}

	void XC_SpanAllocator::rebuild()
	{
		assert(_pHead);

		_pHead->_firstAvailableSpan    = 0;
		_pHead->_spanAvailable         = _pHead->_iSpanCount;

		memset(_pData, 0x00, _pHead->_iSpanCount *_pHead->_iSpanSize);

		unsigned char *pt               = _pData;
		for(size_t i = 0; i != _pHead->_iSpanCount; pt+= _pHead->_iSpanSize)
		{
			++i;
			memcpy(pt, &i, sizeof(size_t));
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void XC_Page::DLL_Init(XC_Span* list, size_t iIndex)
	{
		list->next = iIndex;
		list->prev = iIndex;
	}


	void XC_Page::DLL_Remove(XC_Span *span)
	{
		XC_Span *_prev = reinterpret_cast<XC_Span*>(reinterpret_cast<size_t>(_pShmFlagHead) + span->prev);
		XC_Span *_next = reinterpret_cast<XC_Span*>(reinterpret_cast<size_t>(_pShmFlagHead) + span->next);

		_prev->next = span->next;
		_next->prev = span->prev;
		span->prev = 0;
		span->next = 0;	
	}

	void XC_Page::DLL_Prepend(XC_Span *list, XC_Span *span)
	{
		size_t iBeginAddr = reinterpret_cast<size_t>(_pShmFlagHead);
		XC_Span *_next = reinterpret_cast<XC_Span*>(iBeginAddr + list->next);
		
		span->next = list->next;
		span->prev = reinterpret_cast<size_t>(list) - iBeginAddr;
		_next->prev = reinterpret_cast<size_t>(span) - iBeginAddr;
		list->next = reinterpret_cast<size_t>(span) - iBeginAddr;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	XC_Page::XC_Span* XC_Page::SearchFreeAndLargeLists(size_t n)
	{
		for (size_t s = n; s < kMaxPages; ++s)
		{
			XC_Span* ll = &_pFree[s];
			if (!DLL_IsEmpty(ll, reinterpret_cast<size_t>(ll) - reinterpret_cast<size_t>(_pShmFlagHead)))
			{
				return Carve(reinterpret_cast<XC_Span*>(reinterpret_cast<size_t>(_pShmFlagHead) + ll->next), n);
			}
		}
		return AllocLarge(n);
	}


	XC_Page::XC_Span* XC_Page::New(size_t n)
	{
		XC_Span* result = SearchFreeAndLargeLists(n);
		if (result != NULL)
			return result;

		if ( _pShmFlagHead->_iShmFlag == 0)
		{
			if (!UseShmMem())
			{
				return NULL;
			}
		}

		return SearchFreeAndLargeLists(n);
	}

	XC_Page::XC_Span* XC_Page::AllocLarge(size_t n)
	{
		XC_Span *best = NULL;
		size_t iBeginAddr = reinterpret_cast<size_t>(_pShmFlagHead);

		XC_Span* span = reinterpret_cast<XC_Span*>(iBeginAddr + _pLarge->next);
		
		for (; span != _pLarge; span = reinterpret_cast<XC_Span*>(iBeginAddr + span->next))
		{
			if (span->length >= n)
			{
				if ((best == NULL) || (span->length < best->length) || ((span->length == best->length) && (span->start < best->start)))
				{
					best = span;
				}
			}
		}
		return best == NULL ? NULL : Carve(best, n);
	}

	XC_Page::XC_Span* XC_Page::Carve(XC_Span *span, size_t n)
	{
		unsigned int old_location = span->location;
		bool   flag			= _pShmFlagHead->_bShmProtectedArea;
		size_t extra = span->length - n;

		if (extra >= kMaxPages)
		{
			XC_Span* leftover = static_cast<XC_Span*>(_spanAlloc.allocate());
			if (flag)
			{
				update((void*)(&(leftover->start)), span->start);
				update((void*)(&(leftover->length)), n);
				update((void*)(&(leftover->location)), static_cast<unsigned int>(XC_Span::ON_FREELIST));//update((void*)(&(leftover->location)), XC_Span::IN_USE);
				
				update((void*)(&(_pPageMap[span->start])), reinterpret_cast<size_t>(leftover) - reinterpret_cast<size_t>(_spanAlloc.getHead()));
				if (n > 1)
				{
					update((void*)(&(_pPageMap[span->start + n - 1])), reinterpret_cast<size_t>(leftover) - reinterpret_cast<size_t>(_spanAlloc.getHead()));
				}

				update((void*)(&(span->start)), span->start + n);
				update((void*)(&(span->length)), extra);

				update((void*)(&(_pPageMap[span->start + n])), reinterpret_cast<size_t>(span) - reinterpret_cast<size_t>(_spanAlloc.getHead()));
				if (extra > 1)
				{
					update((void*)(&(_pPageMap[span->start + n + extra - 1])), reinterpret_cast<size_t>(span) - reinterpret_cast<size_t>(_spanAlloc.getHead()));
				}
				doUpdate(true);
			}
			else
			{
				leftover->start  = span->start;
				leftover->length = n;
				leftover->location = XC_Span::IN_USE;
				RecordSpan(leftover);

				span->start = span->start + n;
				span->length = extra;
				RecordSpan(span);
			}
			return leftover;
		}
		else
		{
			XC_Span *_prev = reinterpret_cast<XC_Span*>(reinterpret_cast<size_t>(_pShmFlagHead) + span->prev);
			XC_Span *_next = reinterpret_cast<XC_Span*>(reinterpret_cast<size_t>(_pShmFlagHead) + span->next);

			if (flag)
			{
				update((void*)(&(_prev->next)), span->next);
				update((void*)(&(_next->prev)), span->prev);
				update((void*)(&(span->prev)), static_cast<size_t>(0));
				update((void*)(&(span->next)), static_cast<size_t>(0));
				update((void*)(&(span->location)), static_cast<unsigned int>(XC_Span::ON_FREELIST));
			}
			else
			{
				_prev->next = span->next;
				_next->prev = span->prev;
				span->prev = 0;
				span->next = 0;
				span->location = XC_Span::IN_USE;
			}

			if (extra > 0)
			{
				XC_Span* leftover = static_cast<XC_Span*>(_spanAlloc.allocate());
				if (flag)
				{
					update((void*)(&(leftover->start)), span->start + n);
					update((void*)(&(leftover->length)), extra);
					update((void*)(&(leftover->location)), old_location);
					
					update((void*)(&(_pPageMap[span->start + n])), reinterpret_cast<size_t>(leftover) - reinterpret_cast<size_t>(_spanAlloc.getHead()));
					if (extra > 1)
					{
						update((void*)(&(_pPageMap[span->start + n + extra - 1])), reinterpret_cast<size_t>(leftover) - reinterpret_cast<size_t>(_spanAlloc.getHead()));
					}
				}
				else
				{
					leftover->start  = span->start + n;
					leftover->length = extra;
					leftover->location = old_location;
					RecordSpan(leftover);
				}
				
				XC_Span* list = &_pFree[extra];

				size_t iBeginAddr = reinterpret_cast<size_t>(_pShmFlagHead);
				XC_Span *_pnext = reinterpret_cast<XC_Span*>(iBeginAddr + list->next);

				if (flag)
				{
					update((void*)(&(leftover->next)), list->next);
					update((void*)(&(leftover->prev)), reinterpret_cast<size_t>(list) - iBeginAddr);
					update((void*)(&(_pnext->prev)), reinterpret_cast<size_t>(leftover) - iBeginAddr);
					update((void*)(&(list->next)), reinterpret_cast<size_t>(leftover) - iBeginAddr);
					update((void*)(&(span->length)), n);

					update((void*)(&(_pPageMap[span->start + n -1])), reinterpret_cast<size_t>(span) - reinterpret_cast<size_t>(_spanAlloc.getHead()));
				}
				else
				{
					leftover->next = list->next;
					leftover->prev = reinterpret_cast<size_t>(list) - iBeginAddr;
					_pnext->prev = reinterpret_cast<size_t>(leftover) - iBeginAddr;
					list->next = reinterpret_cast<size_t>(leftover) - iBeginAddr;
					span->length = n;
					Set(span->start + n - 1, reinterpret_cast<size_t>(span) - reinterpret_cast<size_t>(_spanAlloc.getHead()));
				}	
				
			}
			if (flag)
			{
				doUpdate(true);
			}
		}
		return span;
	}

	void XC_Page::PrependToFreeList(XC_Span* span)
	{
		XC_Span* list = (span->length < kMaxPages) ? &_pFree[span->length] : _pLarge;
		DLL_Prepend(list, span);
	}

	void XC_Page::Delete(XC_Span *span)
	{
		bool flag = _pShmFlagHead->_bShmProtectedArea;

		if(flag)
		{
			size_t p = span->start;
			size_t n = span->length;
			size_t location = XC_Span::ON_FREELIST;
			size_t iBeginAddr = reinterpret_cast<size_t>(_pShmFlagHead);
			size_t spanBeginAddr = 0;
			XC_Span* pPrev = NULL;
			XC_Span* pNext = NULL;
			bool     prevMaxFlag = false;
			bool	 prevMinFlag = true;

			if ( p > 0)
			{
				pPrev = GetDescriptor(p-1);
				if (pPrev != NULL && pPrev->location == location)
				{
					const size_t len = pPrev->length;
					spanBeginAddr = reinterpret_cast<size_t>(_spanAlloc.getHead());

					if (len >= kMaxPages)
					{
						p -= len;
						n += len;

						update((void*)(&(pPrev->length)), n);
						update((void*)(&(_pPageMap[p + n - 1])), reinterpret_cast<size_t>(pPrev) - spanBeginAddr);
						doUpdate(true);
						
						_spanAlloc.deallocate(static_cast<void*>(span));
						span = pPrev;
						prevMaxFlag = true;
						prevMinFlag = false;
					}
					else
					{
						if (pPrev->prev != 0 && pPrev->next != 0)
						{
							XC_Span *_prev = reinterpret_cast<XC_Span*>(iBeginAddr + pPrev->prev);
							XC_Span *_next = reinterpret_cast<XC_Span*>(iBeginAddr + pPrev->next);

							update((void*)(&(_prev->next)), pPrev->next);
							update((void*)(&(_next->prev)), pPrev->prev);
							update((void*)(&(pPrev->prev)), static_cast<size_t>(0));
							update((void*)(&(pPrev->next)), static_cast<size_t>(0));
						}
						
						p -= len;
						n += len;
						
						update((void*)(&(span->start)), p);
						update((void*)(&(span->length)), n);

						update((void*)(&(_pPageMap[p])), reinterpret_cast<size_t>(span) - spanBeginAddr);
						doUpdate(true);
						_spanAlloc.deallocate(static_cast<void*>(pPrev));
						prevMinFlag = true;
					}
				}
			}
			if ( p+n < _pShmFlagHead->_iShmPageMapNum)
			{
				pNext = GetDescriptor(p+n);
				if (pNext != NULL && pNext->location == location)
				{
					const size_t len = pNext->length;
					spanBeginAddr = reinterpret_cast<size_t>(_spanAlloc.getHead());

					if (n >= kMaxPages && len >= kMaxPages)
					{
						if (prevMaxFlag)
						{
							XC_Span *_prev = reinterpret_cast<XC_Span*>(iBeginAddr + span->prev);
							XC_Span *_next = reinterpret_cast<XC_Span*>(iBeginAddr + span->next);

							update((void*)(&(_prev->next)), span->next);
							update((void*)(&(_next->prev)), span->prev);
							update((void*)(&(span->prev)), static_cast<size_t>(0));
							update((void*)(&(span->next)), static_cast<size_t>(0));

							update((void*)(&(pNext->start)), p);
							update((void*)(&(pNext->length)), n + len);

							update((void*)(&(_pPageMap[p])), reinterpret_cast<size_t>(pNext) - spanBeginAddr);

							doUpdate(true);
							_spanAlloc.deallocate(static_cast<void*>(span));
							prevMinFlag = false;
						}
						else
						{
							update((void*)(&(pNext->start)), p);
							update((void*)(&(pNext->length)), n + len);

							update((void*)(&(_pPageMap[p])), reinterpret_cast<size_t>(pNext) - spanBeginAddr);
							doUpdate(true);
							_spanAlloc.deallocate(static_cast<void*>(span));
							prevMinFlag = false;
						}
					}
					else if (n < kMaxPages && len >= kMaxPages)
					{
						update((void*)(&(pNext->start)), p);
						update((void*)(&(pNext->length)), len + n);

						update((void*)(&(_pPageMap[p])), reinterpret_cast<size_t>(pNext) - spanBeginAddr);
						doUpdate(true);
						
						_spanAlloc.deallocate(static_cast<void*>(span));
						prevMinFlag = false;
					}
					else if (n >= kMaxPages && len < kMaxPages)
					{
						if (prevMaxFlag)
						{
							if (pNext->prev != 0 && pNext->next != 0)
							{
								XC_Span *_prev = reinterpret_cast<XC_Span*>(iBeginAddr + pNext->prev);
								XC_Span *_next = reinterpret_cast<XC_Span*>(iBeginAddr + pNext->next);

								update((void*)(&(_prev->next)), pNext->next);
								update((void*)(&(_next->prev)), pNext->prev);
								update((void*)(&(pNext->prev)), static_cast<size_t>(0));
								update((void*)(&(pNext->next)), static_cast<size_t>(0));
							}

							update((void*)(&(span->length)), n + len);
							n += len;

							update((void*)(&(_pPageMap[p+n-1])), reinterpret_cast<size_t>(span) - spanBeginAddr);
							doUpdate(true);

							_spanAlloc.deallocate(static_cast<void*>(pNext));
							prevMinFlag = false;
						}
						else
						{
							if (pNext->prev != 0 && pNext->next != 0)
							{
								XC_Span *_prev = reinterpret_cast<XC_Span*>(iBeginAddr + pNext->prev);
								XC_Span *_next = reinterpret_cast<XC_Span*>(iBeginAddr + pNext->next);

								update((void*)(&(_prev->next)), pNext->next);
								update((void*)(&(_next->prev)), pNext->prev);
								update((void*)(&(pNext->prev)), static_cast<size_t>(0));
								update((void*)(&(pNext->next)), static_cast<size_t>(0));
							}

							update((void*)(&(span->length)), n + len);
							n += len;

							update((void*)(&(_pPageMap[p+n-1])), reinterpret_cast<size_t>(span) - spanBeginAddr);

							XC_Span* list = (n < kMaxPages) ? &_pFree[n] : _pLarge;		
				
							XC_Span *_pnext = reinterpret_cast<XC_Span*>(iBeginAddr + list->next);
							
							update((void*)(&(span->next)), list->next);
							update((void*)(&(span->prev)), reinterpret_cast<size_t>(list) - iBeginAddr);
							update((void*)(&(_pnext->prev)), reinterpret_cast<size_t>(span) - iBeginAddr);
							update((void*)(&(list->next)), reinterpret_cast<size_t>(span) - iBeginAddr);

				
							doUpdate(true);
							_spanAlloc.deallocate(static_cast<void*>(pNext));
							prevMinFlag = false;
						}			
					}
					else
					{
						if (pNext->prev != 0 && pNext->next != 0)
						{
							XC_Span *_prev = reinterpret_cast<XC_Span*>(iBeginAddr + pNext->prev);
							XC_Span *_next = reinterpret_cast<XC_Span*>(iBeginAddr + pNext->next);

							update((void*)(&(_prev->next)), pNext->next);
							update((void*)(&(_next->prev)), pNext->prev);
							update((void*)(&(pNext->prev)), static_cast<size_t>(0));
							update((void*)(&(pNext->next)), static_cast<size_t>(0));
						}

						update((void*)(&(span->length)), n + len);
						n += len;

						update((void*)(&(_pPageMap[p+n-1])), reinterpret_cast<size_t>(span) - spanBeginAddr);

						XC_Span* list = (n < kMaxPages) ? &_pFree[n] : _pLarge;		
				
						XC_Span *_pnext = reinterpret_cast<XC_Span*>(iBeginAddr + list->next);
						
						update((void*)(&(span->next)), list->next);
						update((void*)(&(span->prev)), reinterpret_cast<size_t>(list) - iBeginAddr);
						update((void*)(&(_pnext->prev)), reinterpret_cast<size_t>(span) - iBeginAddr);
						update((void*)(&(list->next)), reinterpret_cast<size_t>(span) - iBeginAddr);
						doUpdate(true);

						_spanAlloc.deallocate(static_cast<void*>(pNext));
						prevMinFlag = false;
					}
					
				}
			}

			if (prevMinFlag)
			{
				XC_Span* list = (n < kMaxPages) ? &_pFree[n] : _pLarge;		
				
				XC_Span *_next = reinterpret_cast<XC_Span*>(iBeginAddr + list->next);
				
				update((void*)(&(span->next)), list->next);
				update((void*)(&(span->prev)), reinterpret_cast<size_t>(list) - iBeginAddr);
				update((void*)(&(_next->prev)), reinterpret_cast<size_t>(span) - iBeginAddr);
				update((void*)(&(list->next)), reinterpret_cast<size_t>(span) - iBeginAddr);

				doUpdate(true);
			}

		}
		else
		{
			span->sizeclass = 0;
			span->location  = XC_Span::ON_FREELIST;
			MergeIntoFreeList(span);
		}
	}

	void XC_Page::MergeIntoFreeList(XC_Span* span)
	{
		const size_t p = span->start;
		const size_t n = span->length;

		if ( p > 0)
		{
			XC_Span* pPrev = GetDescriptor(p-1);
			if (pPrev != NULL && pPrev->location == span->location)
			{
				const size_t len = pPrev->length;
				DLL_Remove(pPrev);

				_spanAlloc.deallocate(static_cast<void*>(pPrev));
				span->start -= len;
				span->length += len;
				Set(span->start, reinterpret_cast<size_t>(span) - reinterpret_cast<size_t>(_spanAlloc.getHead()));
			}
		}
		if ( p+n < _pShmFlagHead->_iShmPageMapNum)
		{
			XC_Span* pNext = GetDescriptor(p+n);
			if (pNext != NULL && pNext->location == span->location)
			{
				const size_t len = pNext->length;
				DLL_Remove(pNext);

				_spanAlloc.deallocate(static_cast<void*>(pNext));
				span->length += len;
				Set(span->start + span->length - 1, reinterpret_cast<size_t>(span) - reinterpret_cast<size_t>(_spanAlloc.getHead()));
			}
		}

		PrependToFreeList(span);
	}

	void XC_Page::RegisterSizeClass(XC_Span *span, size_t sc)
	{
		span->sizeclass = sc;
		for (size_t i = 1; i < span->length - 1; ++i)
		{
			Set(span->start + i, reinterpret_cast<size_t>(span) - reinterpret_cast<size_t>(_spanAlloc.getHead()));
		}
	}

	XC_Page::XC_Span* XC_Page::GetDescriptor(size_t pPageId)
	{
		size_t end    = _pShmFlagHead->_iShmPageMapNum;

		if (pPageId >= 0 && pPageId < end)
		{
			return reinterpret_cast<XC_Span*>(reinterpret_cast<char*>(_spanAlloc.getHead()) + Get(pPageId));
		}
		else
		{
			return NULL;
		}
	}

	bool XC_Page::UseShmMem()
	{
		size_t iBeginAddr	= reinterpret_cast<size_t>(_pShmFlagHead);
		size_t    ptr		= reinterpret_cast<size_t>(_pData);
		size_t    size		= _pShmFlagHead->_iShmPageNum;
		const size_t p		= (ptr - iBeginAddr - _pShmFlagHead->_iShmPageAddr) >> kPageShift;

		if (Ensure(p, size))
		{
			XC_Span* span = static_cast<XC_Span*>(_spanAlloc.allocate());
			
			span->start = p;
			span->length = size;
			RecordSpan(span);

			span->sizeclass = 0;
			span->location  = XC_Span::ON_FREELIST;

			XC_Span* list = (size < kMaxPages) ? &_pFree[size] : _pLarge;

			XC_Span *_next = reinterpret_cast<XC_Span*>(iBeginAddr + list->next);
			
			span->next = list->next;
			span->prev = reinterpret_cast<size_t>(list) - iBeginAddr;
			_next->prev = reinterpret_cast<size_t>(span) - iBeginAddr;
			list->next = reinterpret_cast<size_t>(span) - iBeginAddr;
		
			_pShmFlagHead->_iShmFlag = 1;
			return true;
		}
		else
		{
			return false;
		}
	}

	void XC_Page::RecordSpan(XC_Span *span)
	{
		Set(span->start, reinterpret_cast<size_t>(span) - reinterpret_cast<size_t>(_spanAlloc.getHead()));
		if (span->length > 1)
		{
			Set(span->start + span->length - 1, reinterpret_cast<size_t>(span) - reinterpret_cast<size_t>(_spanAlloc.getHead()));
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void* XC_Page::fetchFromSpansSafe(size_t iClassSize, size_t &iAllocSize, size_t &iPageId, size_t &iIndex)
	{
		void *t = FetchFromSpans(iClassSize, iAllocSize, iPageId, iIndex);

		if (!t)
		{
			if (Populate(iClassSize) == 0)
			{
				t = FetchFromSpans(iClassSize, iAllocSize, iPageId, iIndex);
			}
			else
			{
				return NULL;
			}
		}
		return t;
	}

	void* XC_Page::FetchFromSpans(size_t iClassSize, size_t &iAllocSize, size_t &iPageId, size_t &iIndex)
	{
		size_t iBeginAddr = reinterpret_cast<size_t>(_pShmFlagHead);
		if (DLL_IsEmpty(&_pCenterCache[iClassSize].nonempty, reinterpret_cast<size_t>(&_pCenterCache[iClassSize].nonempty) - iBeginAddr))
			return NULL;

		XC_Span* span = reinterpret_cast<XC_Span*>(iBeginAddr + _pCenterCache[iClassSize].nonempty.next);

		void* result = reinterpret_cast<void*>(iBeginAddr + span->objects);
		size_t _size = Static::sizemap()->ByteSizeForClass(iClassSize);
		iPageId = span->start;
		iIndex   = (span->objects - (_pShmFlagHead->_iShmPageAddr + (iPageId << kPageShift))) / _size;
		size_t last = (span->length << kPageShift) / _size;

		if ((iIndex + 1) != last)
		{
			iAllocSize = _size;
		}
		else
		{
			iAllocSize = (span->length << kPageShift) - iIndex * _size;
		}

		bool flag     = _pShmFlagHead->_bShmProtectedArea;
		size_t p	  = *(reinterpret_cast<size_t*>(result));

		if (flag)
		{
			update((void*)(&(span->objects)), p);
		}
		else
		{
			span->objects = p;
		}

		if (p == 0)
		{
			XC_Span *_prev = reinterpret_cast<XC_Span*>(reinterpret_cast<size_t>(_pShmFlagHead) + span->prev);
			XC_Span *_next = reinterpret_cast<XC_Span*>(reinterpret_cast<size_t>(_pShmFlagHead) + span->next);

			if (flag)
			{
				update((void*)(&(_prev->next)), span->next);
				update((void*)(&(_next->prev)), span->prev);
				update((void*)(&(span->prev)), static_cast<size_t>(0));
				update((void*)(&(span->next)), static_cast<size_t>(0));
			}
			else
			{
				_prev->next = span->next;
				_next->prev = span->prev;
				span->prev = 0;
				span->next = 0;
			}

			XC_Span *list  = &(_pCenterCache[iClassSize].empty);
			_next = reinterpret_cast<XC_Span*>(iBeginAddr + list->next);
			if (flag)
			{
				update((void*)(&(span->next)), list->next);
				update((void*)(&(span->prev)), reinterpret_cast<size_t>(list) - iBeginAddr);
				update((void*)(&(_next->prev)), reinterpret_cast<size_t>(span) - iBeginAddr);
				update((void*)(&(list->next)), reinterpret_cast<size_t>(span) - iBeginAddr);
			}
			else
			{
				span->next = list->next;
				span->prev = reinterpret_cast<size_t>(list) - iBeginAddr;
				_next->prev = reinterpret_cast<size_t>(span) - iBeginAddr;
				list->next = reinterpret_cast<size_t>(span) - iBeginAddr;
			}
		}

		if (flag)
		{
			update((void*)(&(span->refcount)), span->refcount + 1);
			doUpdate(true);
		}
		else
		{
			++(span->refcount);
		}

		return result;
	}

	int XC_Page::Populate(size_t iClassSize)
	{
		const size_t npages = Static::sizemap()->class_to_pages(iClassSize);
		bool      flag		= _pShmFlagHead->_bShmProtectedArea;

		XC_Span* span = New(npages);

		if (span)
		{
			if (flag)
			{
				update((void*)(&(span->location)), static_cast<unsigned int>(XC_Span::IN_USE));
				update((void*)(&(span->sizeclass)), static_cast<unsigned int>(iClassSize));
				for (size_t i = 1; i < span->length - 1; ++i)
				{
					update((void*)(&(_pPageMap[span->start + i])), reinterpret_cast<size_t>(span) - reinterpret_cast<size_t>(_spanAlloc.getHead()));
				}
			}
			else
			{
				RegisterSizeClass(span, iClassSize);
			}
			
		}
		else
		{
			return -1;
		}

		size_t iBeginAddr = reinterpret_cast<size_t>(_pShmFlagHead);
		
		size_t* tail  = &(span->objects);
		char*  ptr  = reinterpret_cast<char*>(iBeginAddr + _pShmFlagHead->_iShmPageAddr + (span->start << kPageShift));
		const  size_t size = Static::sizemap()->ByteSizeForClass(iClassSize);
		size_t num  = ((span->length) << kPageShift) / size;
		char*  _ptr = ptr + size * (num - 1);
		size_t* temp = reinterpret_cast<size_t*>(_ptr);

		*temp = 0;
		*tail = reinterpret_cast<size_t>(_ptr) - iBeginAddr;

		span->refcount = 0;

		while (num > 1)
		{
			_ptr -= size;
			temp = reinterpret_cast<size_t*>(_ptr);
			*temp = *tail;
			*tail = reinterpret_cast<size_t>(_ptr) - iBeginAddr;
			--num;
		}

		XC_Span *list = &(_pCenterCache[iClassSize].nonempty);
		XC_Span *_next = reinterpret_cast<XC_Span*>(iBeginAddr + list->next);
		if (flag)
		{
			update((void*)(&(span->next)), list->next);
			update((void*)(&(span->prev)), reinterpret_cast<size_t>(list) - iBeginAddr);
			update((void*)(&(_next->prev)), reinterpret_cast<size_t>(span) - iBeginAddr);
			update((void*)(&(list->next)), reinterpret_cast<size_t>(span) - iBeginAddr);

			doUpdate(true);
		}
		else
		{
			span->next = list->next;
			span->prev = reinterpret_cast<size_t>(list) - iBeginAddr;
			_next->prev = reinterpret_cast<size_t>(span) - iBeginAddr;
			list->next = reinterpret_cast<size_t>(span) - iBeginAddr;
		}

		return 0;
	}

	void XC_Page::releaseToSpans(size_t iPageId, size_t iIndex)
	{
		XC_Span* span = GetDescriptor(iPageId);
		assert(span != NULL);
		const size_t _size_class = span->sizeclass;

		assert(_size_class > 0);
		size_t iBeginAddr = reinterpret_cast<size_t>(_pShmFlagHead);
		bool   flag	      = _pShmFlagHead->_bShmProtectedArea;
		unsigned int ref  = span->refcount - 1;
		size_t spanObject = span->objects;

		if (ref == 0)
		{
			XC_Span *_prev = reinterpret_cast<XC_Span*>(iBeginAddr + span->prev);
			XC_Span *_next = reinterpret_cast<XC_Span*>(iBeginAddr + span->next);

			if (flag)
			{
				update((void*)(&(span->refcount)), ref);
				update((void*)(&(_prev->next)), span->next);
				update((void*)(&(_next->prev)), span->prev);
				update((void*)(&(span->prev)), static_cast<size_t>(0));
				update((void*)(&(span->next)), static_cast<size_t>(0));
				update((void*)(&(span->sizeclass)), static_cast<unsigned int>(0));
				update((void*)(&(span->location)), static_cast<unsigned int>(XC_Span::ON_FREELIST));
				doUpdate(true);
			}
			else
			{
				--(span->refcount);
				_prev->next = span->next;
				_next->prev = span->prev;
				span->prev = 0;
				span->next = 0;			
			}

			Delete(span);
			return ;
		}
		else
		{
			void*  ptr  = reinterpret_cast<void*>(iBeginAddr + _pShmFlagHead->_iShmPageAddr + (iPageId << kPageShift) + iIndex *  Static::sizemap()->ByteSizeForClass(_size_class));
			if (flag)
			{
				update(ptr, span->objects);
				update((void*)(&(span->objects)), reinterpret_cast<size_t>(ptr) - iBeginAddr);
				update((void*)(&(span->refcount)), ref);
			}
			else
			{
				*(reinterpret_cast<size_t*>(ptr)) = span->objects;
				span->objects = reinterpret_cast<size_t>(ptr) - iBeginAddr;
				--(span->refcount);
			}
		}

		if (spanObject == 0)
		{
			XC_Span *_prev = reinterpret_cast<XC_Span*>(iBeginAddr + span->prev);
			XC_Span *_next = reinterpret_cast<XC_Span*>(iBeginAddr + span->next);

			if (flag)
			{
				update((void*)(&(_prev->next)), span->next);
				update((void*)(&(_next->prev)), span->prev);
				update((void*)(&(span->prev)), static_cast<size_t>(0));
				update((void*)(&(span->next)), static_cast<size_t>(0));
			}
			else
			{
				_prev->next = span->next;
				_next->prev = span->prev;
				span->prev = 0;
				span->next = 0;
			}

			XC_Span *list  = &(_pCenterCache[_size_class].nonempty);
			_next = reinterpret_cast<XC_Span*>(iBeginAddr + list->next);
			if (flag)
			{
				update((void*)(&(span->next)), list->next);
				update((void*)(&(span->prev)), reinterpret_cast<size_t>(list) - iBeginAddr);
				update((void*)(&(_next->prev)), reinterpret_cast<size_t>(span) - iBeginAddr);
				update((void*)(&(list->next)), reinterpret_cast<size_t>(span) - iBeginAddr);
			}
			else
			{
				span->next = list->next;
				span->prev = reinterpret_cast<size_t>(list) - iBeginAddr;
				_next->prev = reinterpret_cast<size_t>(span) - iBeginAddr;
				list->next = reinterpret_cast<size_t>(span) - iBeginAddr;
			}
		}

		if (flag)
		{
			doUpdate(true);
		}	

	}

	void  XC_Page::releaseToSpans(void* pObject)
	{
		size_t iBeginAddr       = reinterpret_cast<size_t>(_pShmFlagHead);
		const size_t _page_id	= (reinterpret_cast<size_t>(pObject) - (iBeginAddr + _pShmFlagHead->_iShmPageAddr)) >> kPageShift;

		XC_Span* span			= GetDescriptor(_page_id);
		assert(span != NULL);

		const size_t _size_class = span->sizeclass;
		assert(_size_class > 0);

		bool      flag	  = _pShmFlagHead->_bShmProtectedArea;
		unsigned int ref  = span->refcount - 1;
		size_t spanObject = span->objects;

		if (ref == 0)
		{
			XC_Span *_prev = reinterpret_cast<XC_Span*>(iBeginAddr + span->prev);
			XC_Span *_next = reinterpret_cast<XC_Span*>(iBeginAddr + span->next);

			if (flag)
			{
				update((void*)(&(span->refcount)), ref);
				update((void*)(&(_prev->next)), span->next);
				update((void*)(&(_next->prev)), span->prev);
				update((void*)(&(span->prev)), static_cast<size_t>(0));
				update((void*)(&(span->next)), static_cast<size_t>(0));
				update((void*)(&(span->sizeclass)), static_cast<unsigned int>(0));
				update((void*)(&(span->location)), static_cast<unsigned int>(XC_Span::ON_FREELIST));
				doUpdate(true);
			}
			else
			{
				--(span->refcount);
				_prev->next = span->next;
				_next->prev = span->prev;
				span->prev = 0;
				span->next = 0;
				
			}

			Delete(span);
			return ;
		}
		else
		{
			if (flag)
			{
				update(pObject, span->objects);
				update((void*)(&(span->objects)), reinterpret_cast<size_t>(pObject) - iBeginAddr);
				update((void*)(&(span->refcount)), ref);
			}
			else
			{
				*(reinterpret_cast<size_t*>(pObject)) = span->objects;
				span->objects = reinterpret_cast<size_t>(pObject) - iBeginAddr;
				--(span->refcount);
			}			
		}

		if (spanObject == 0)
		{
			XC_Span *_prev = reinterpret_cast<XC_Span*>(iBeginAddr + span->prev);
			XC_Span *_next = reinterpret_cast<XC_Span*>(iBeginAddr + span->next);

			if (flag)
			{
				update((void*)(&(_prev->next)), span->next);
				update((void*)(&(_next->prev)), span->prev);
				update((void*)(&(span->prev)), static_cast<size_t>(0));
				update((void*)(&(span->next)), static_cast<size_t>(0));
			}
			else
			{
				_prev->next = span->next;
				_next->prev = span->prev;
				span->prev = 0;
				span->next = 0;
			}

			XC_Span *list  = &(_pCenterCache[_size_class].nonempty);
			_next = reinterpret_cast<XC_Span*>(iBeginAddr + list->next);
			if (flag)
			{
				update((void*)(&(span->next)), list->next);
				update((void*)(&(span->prev)), reinterpret_cast<size_t>(list) - iBeginAddr);
				update((void*)(&(_next->prev)), reinterpret_cast<size_t>(span) - iBeginAddr);
				update((void*)(&(list->next)), reinterpret_cast<size_t>(span) - iBeginAddr);
			}
			else
			{
				span->next = list->next;
				span->prev = reinterpret_cast<size_t>(list) - iBeginAddr;
				_next->prev = reinterpret_cast<size_t>(span) - iBeginAddr;
				list->next = reinterpret_cast<size_t>(span) - iBeginAddr;
			}
		}

		if (flag)
		{
			doUpdate(true);
		}
		
	}

	void* XC_Page::getAbsolute(size_t iPageId, size_t iIndex)
	{
		XC_Span* span = GetDescriptor(iPageId);
		assert(span != NULL);
		const size_t _size_class = span->sizeclass;
		assert(_size_class > 0);

		return reinterpret_cast<void*>(reinterpret_cast<size_t>(_pShmFlagHead) + _pShmFlagHead->_iShmPageAddr + (iPageId << kPageShift) + iIndex *  Static::sizemap()->ByteSizeForClass(_size_class));
	}

	void XC_Page::doUpdate(bool bUpdate)
	{
		if(bUpdate)
		{
			_pModifyHead->_cModifyStatus = 2;
		}

		if(_pModifyHead->_cModifyStatus == 1)
		{
			_pModifyHead->_iNowIndex        = 0;
			for(size_t i = 0; i < sizeof(_pModifyHead->_stModifyData) / sizeof(tagModifyData); i++)
			{
				_pModifyHead->_stModifyData[i]._iModifyAddr       = 0;
				_pModifyHead->_stModifyData[i]._cBytes            = 0;
				_pModifyHead->_stModifyData[i]._iModifyValue      = 0;
			}
			_pModifyHead->_cModifyStatus    = 0;
		}
		else if(_pModifyHead->_cModifyStatus == 2)
		{
			for(size_t i = 0; i < _pModifyHead->_iNowIndex; i++)
			{
				if(_pModifyHead->_stModifyData[i]._cBytes == sizeof(size_t))
				{
					*(reinterpret_cast<size_t*>(reinterpret_cast<size_t>(_pShmFlagHead) + _pModifyHead->_stModifyData[i]._iModifyAddr)) = _pModifyHead->_stModifyData[i]._iModifyValue;
				}
				else if(_pModifyHead->_stModifyData[i]._cBytes == sizeof(unsigned int))
				{
					*(reinterpret_cast<unsigned int*>(reinterpret_cast<size_t>(_pShmFlagHead) + _pModifyHead->_stModifyData[i]._iModifyAddr)) = static_cast<unsigned int>(_pModifyHead->_stModifyData[i]._iModifyValue);
				}
				else
				{
					assert(true);
				}
			}
			_pModifyHead->_iNowIndex        = 0;
			_pModifyHead->_cModifyStatus    = 0;
		}
		else if(_pModifyHead->_cModifyStatus == 0)
		{
			return;
		}

	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void XC_Page::init(void *pAddr)
	{
		_pShmFlagHead	= static_cast<tagShmFlag*>(pAddr);

		_pModifyHead    = static_cast<tagModifyHead*>((void*)((char*)_pShmFlagHead + sizeof(tagShmFlag)));

		_pCenterCache   = static_cast<XC_CenterList*>((void*)((char*)_pModifyHead + sizeof(tagModifyHead)));

		_pLarge         = static_cast<XC_Span*>((void*)((char*)_pCenterCache + sizeof(XC_CenterList) * kNumClasses));

		_pFree          = static_cast<XC_Span*>((void*)((char*)_pLarge + sizeof(XC_Span)));
	}

	void XC_Page::initPage(void *pAddr)
	{
		void * span_begin_ptr				= static_cast<void*>((char*)_pFree + sizeof(XC_Span) * kMaxPages);
		size_t begin						= reinterpret_cast<size_t>(pAddr);
		size_t end							= reinterpret_cast<size_t>(span_begin_ptr);

		_pSpanMemHead						= static_cast<XC_SpanAllocator*>((void*)((char*)span_begin_ptr));
		_pShmFlagHead->_iShmSpanAddr		= end - begin;

		size_t _iCapacitySize				= _pShmFlagHead->_iShmMemSize - (end - begin);
		size_t _iPagesNum					= (_iCapacitySize - XC_SpanAllocator::getHeadSize() - sizeof(XC_Span) * 20) / (sizeof(XC_Span) + sizeof(size_t) + kPageSize);

		size_t*    pagemap_ptr				= reinterpret_cast<size_t*>((char*)_pSpanMemHead + XC_SpanAllocator::getHeadSize() + sizeof(XC_Span) * (_iPagesNum + 20));
		end									= reinterpret_cast<size_t>(pagemap_ptr);
		_pPageMap							= pagemap_ptr;
		_pShmFlagHead->_iShmPageMapAddr		= end - begin;

		void*     data_ptr					= reinterpret_cast<void*>((char*)_pPageMap + sizeof(size_t) * _iPagesNum);
		end									= reinterpret_cast<size_t>(data_ptr);
		_pData								= data_ptr;
		_pShmFlagHead->_iShmPageAddr		= end - begin;

		memset(_pPageMap, 0, sizeof(size_t)*_iPagesNum);
		_spanAlloc.create((void*)((char*)_pSpanMemHead), sizeof(XC_Span), (_iPagesNum + 20));
		_pShmFlagHead->_iShmSpanNum		= _iPagesNum + 20;
		_pShmFlagHead->_iShmPageMapNum		= _iPagesNum;
		_pShmFlagHead->_iShmPageNum		= _iPagesNum;
		
	}

	void XC_Page::create(void *pAddr, size_t iShmMemSize, bool bProtectedArea)
	{
		init(pAddr);
		_pShmFlagHead->_iShmFlag			= 0;
		_pShmFlagHead->_bShmProtectedArea	= bProtectedArea;
		_pShmFlagHead->_iShmMemSize			= iShmMemSize;

		size_t iBeginAddr                   = reinterpret_cast<size_t>(pAddr);

		memset((void*)_pModifyHead, 0, sizeof(tagModifyHead));

		for (size_t i = 0; i < kNumClasses; ++i)
		{
			_pCenterCache[i].size_class = i;
			DLL_Init(&_pCenterCache[i].empty, reinterpret_cast<size_t>(&_pCenterCache[i].empty) - iBeginAddr);
			DLL_Init(&_pCenterCache[i].nonempty, reinterpret_cast<size_t>(&_pCenterCache[i].nonempty) - iBeginAddr);
		}

		DLL_Init(_pLarge, reinterpret_cast<size_t>(_pLarge) - iBeginAddr);

		for (size_t i = 0; i < kMaxPages; ++i)
		{
			DLL_Init(&_pFree[i], reinterpret_cast<size_t>(&_pFree[i]) - iBeginAddr);
		}

		initPage(pAddr);
	}

	void XC_Page::connect(void *pAddr)
	{
		init(pAddr);
		size_t iBeginAddr = reinterpret_cast<size_t>(pAddr);

		_pSpanMemHead	= reinterpret_cast<XC_SpanAllocator*>(iBeginAddr + _pShmFlagHead->_iShmSpanAddr);
		_spanAlloc.connect((void*)((char*)_pSpanMemHead));
		_pPageMap       = reinterpret_cast<size_t*>(iBeginAddr + _pShmFlagHead->_iShmPageMapAddr);
		_pData          = reinterpret_cast<void*>(iBeginAddr + _pShmFlagHead->_iShmPageAddr);
	}

	void XC_Page::rebuild()
	{
		size_t iBeginAddr        = reinterpret_cast<size_t>(_pShmFlagHead);

		_pShmFlagHead->_iShmFlag = 0;

		memset((void*)_pModifyHead, 0, sizeof(tagModifyHead));

		for (size_t i = 0; i < kNumClasses; ++i)
		{
			_pCenterCache[i].size_class = i;
			DLL_Init(&_pCenterCache[i].empty, reinterpret_cast<size_t>(&_pCenterCache[i].empty) - iBeginAddr);
			DLL_Init(&_pCenterCache[i].nonempty, reinterpret_cast<size_t>(&_pCenterCache[i].nonempty) - iBeginAddr);
		}

		DLL_Init(_pLarge, reinterpret_cast<size_t>(_pLarge) - iBeginAddr);

		for (size_t i = 0; i < kMaxPages; ++i)
		{
			DLL_Init(&_pFree[i], reinterpret_cast<size_t>(&_pFree[i]) - iBeginAddr);
		}

		_pSpanMemHead	= reinterpret_cast<XC_SpanAllocator*>(iBeginAddr + _pShmFlagHead->_iShmSpanAddr);
		_spanAlloc.rebuild();
		_pPageMap       = reinterpret_cast<size_t*>(iBeginAddr + _pShmFlagHead->_iShmPageMapAddr);
		memset(_pPageMap, 0, sizeof(size_t)*_pShmFlagHead->_iShmPageMapNum);
		_pData          = reinterpret_cast<void*>(iBeginAddr + _pShmFlagHead->_iShmPageAddr);
	}

	size_t XC_Page::getPageNumber()
	{
		return _pShmFlagHead->_iShmPageNum;
	}

	size_t XC_Page::getPageMemSize()
	{
		return _pShmFlagHead->_iShmPageNum << kPageShift;
	}

	size_t XC_Page::getPageMemEnd()
	{
		return reinterpret_cast<size_t>(_pShmFlagHead) + _pShmFlagHead->_iShmMemSize;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void XC_MallocChunkAllocator::init(void *pAddr)
	{
		_pHead	= static_cast<tagChunkAllocatorHead*>(pAddr);
		_pChunk = (char*)_pHead + sizeof(tagChunkAllocatorHead);
	}

	void XC_MallocChunkAllocator::create(void *pAddr, size_t iSize, bool bProtectedArea)
	{
		assert(iSize > (sizeof(tagChunkAllocatorHead) + XC_Page::getMinMemSize()));
		init(pAddr);

		_pHead->_bProtectedArea = bProtectedArea;
		_pHead->_iSize			= iSize;
		_pHead->_iTotalSize		= iSize;
		_pHead->_iNext			= 0;

		size_t iChunkCapacity	= iSize - sizeof(tagChunkAllocatorHead);

		_page.create(_pChunk, iChunkCapacity, bProtectedArea);
	}

	void XC_MallocChunkAllocator::_connect(void *pAddr)
	{
		clear();

		init(pAddr);

		_page.connect(_pChunk);

		if(_pHead->_iNext == 0)
		{
			return;
		}

		assert(_pHead->_iNext == _pHead->_iSize);
		assert(_nallocator == NULL);

		tagChunkAllocatorHead  *pNextHead = (tagChunkAllocatorHead   *)((char*)_pHead + _pHead->_iNext);
		_nallocator = new XC_MallocChunkAllocator();
		_nallocator->connect(pNextHead);
	}

	void XC_MallocChunkAllocator::connect(void *pAddr)
	{
		_connect(pAddr);
		doUpdate();
		
	}

	void XC_MallocChunkAllocator::rebuild()
	{
		_page.rebuild();

		if(_nallocator)
		{
			_nallocator->rebuild();
		}
	}

	XC_MallocChunkAllocator * XC_MallocChunkAllocator::lastAlloc()
	{
		if(_nallocator == NULL)
			return NULL;

		XC_MallocChunkAllocator *p = _nallocator;

		while(p && p->_nallocator)
		{
			p = p->_nallocator;
		}
	    
		return p;
	}

	void XC_MallocChunkAllocator::append(void *pAddr, size_t iSize)
	{
		connect(pAddr);

		assert(iSize > _pHead->_iTotalSize);

		void *pAppendAddr = (char*)pAddr + _pHead->_iTotalSize;

		XC_MallocChunkAllocator *p = new XC_MallocChunkAllocator();
		p->create(pAppendAddr, iSize - _pHead->_iTotalSize, _pHead->_bProtectedArea);

		XC_MallocChunkAllocator *palloc = lastAlloc();
		if (palloc)
		{
			palloc->_pHead->_iNext = (char*)pAppendAddr - (char*)palloc->_pHead;
			palloc->_nallocator    = p;
		}
		else
		{
			_pHead->_iNext = (char*)pAppendAddr - (char*)_pHead;
			_nallocator    = p;
		}
		assert(_pHead->_iNext == _pHead->_iSize);

		_pHead->_iTotalSize = iSize;
	}

	void* XC_MallocChunkAllocator::allocate(size_t iNeedSize, size_t &iAllocSize)
	{
		size_t iPageId = 0;
		size_t iIndex   = 0;

		return allocate(iNeedSize, iAllocSize, iPageId, iIndex);
		
	}

	void  XC_MallocChunkAllocator::deallocate(void* pAddr)
	{
		if (_nallocator)
		{
			if (reinterpret_cast<size_t>(pAddr) > _page.getPageMemEnd())
			{
				_nallocator->deallocate(pAddr);
			}
			else
			{
				_page.releaseToSpans(pAddr);
			}

		}
		else
		{
			_page.releaseToSpans(pAddr);
		}
	}

	void* XC_MallocChunkAllocator::allocate(size_t iNeedSize, size_t &iAllocSize, size_t &iPageId, size_t &iIndex)
	{
		if (iNeedSize > kMaxSize)
		{
			iNeedSize = kMaxSize;
			//return NULL;
		}
		size_t iClassSize = Static::sizemap()->SizeClass(iNeedSize);
		void* p = NULL;
		
		p = _page.fetchFromSpansSafe(iClassSize, iAllocSize, iPageId, iIndex);

		if (p != NULL)
		{
			return p;
		}

		for (size_t i = iClassSize + 1; i < kNumClasses; ++i)
		{
			p = _page.fetchFromSpansSafe(i, iAllocSize, iPageId, iIndex);

			if (p != NULL)
			{
				return p;
			}
		}

		for (size_t i = iClassSize - 1; i > 0; --i)
		{
			p = _page.fetchFromSpansSafe(i, iAllocSize, iPageId, iIndex);

			if (p != NULL)
			{
				return p;
			}
		}
		
		if (_nallocator)
		{
			size_t prev = _page.getPageNumber();
			p = _nallocator->allocate(iNeedSize, iAllocSize, iPageId, iIndex);
			iPageId += prev;
			if (p != NULL)
			{
				return p;
			}

		}

		return NULL;
	}

	void XC_MallocChunkAllocator::deallocate(size_t iPageId, size_t iIndex)
	{
		if (_nallocator)
		{
			size_t prev = _page.getPageNumber();
			if (iPageId >= prev)
			{
				_nallocator->deallocate(iPageId - prev, iIndex);
			}
			else
			{
				_page.releaseToSpans(iPageId, iIndex);
			}
		}
		else
		{
			_page.releaseToSpans(iPageId, iIndex);
		}
	}

	void* XC_MallocChunkAllocator::getAbsolute(size_t iPageId, size_t iIndex)
	{
		if(_nallocator == NULL)
		{
			return _page.getAbsolute(iPageId, iIndex);
		}
		else
		{
			size_t prev = _page.getPageNumber();
			if (iPageId > prev)
			{
				return _nallocator->getAbsolute(iPageId - prev, iIndex);
			}
			else
			{
				return _page.getAbsolute(iPageId, iIndex);
			}

		}
		return NULL;
	}

	size_t XC_MallocChunkAllocator::getAllCapacity()
	{	

		if(_nallocator == NULL)
			return _page.getPageMemSize();

		size_t _iCapacity = _page.getPageMemSize();

		XC_MallocChunkAllocator *p = _nallocator;

		while(p)
		{
			_iCapacity += p->_page.getPageMemSize();
			p = p->_nallocator;
		}
	    
		return _iCapacity;
	}

	void   XC_MallocChunkAllocator::getSingleCapacity(vector<size_t> &vec_capacity)
	{
		vec_capacity.clear();

		if (_nallocator == NULL)
		{
			vec_capacity.push_back(_page.getPageMemSize());
			return ;
		}

		XC_MallocChunkAllocator *p = _nallocator;

		while(p)
		{
			vec_capacity.push_back(p->_page.getPageMemSize());
			p = p->_nallocator;
		}
	}

	void XC_MallocChunkAllocator::doUpdate(bool bUpdate)
	{
		_page.doUpdate(bUpdate);

		if (_nallocator)
		{
			_nallocator->_page.doUpdate(bUpdate);
		}
	}
}
