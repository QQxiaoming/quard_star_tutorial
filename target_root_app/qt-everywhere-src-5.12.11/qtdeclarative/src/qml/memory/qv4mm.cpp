/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qv4engine_p.h"
#include "qv4object_p.h"
#include "qv4objectproto_p.h"
#include "qv4mm_p.h"
#include "qv4qobjectwrapper_p.h"
#include "qv4identifiertable_p.h"
#include <QtCore/qalgorithms.h>
#include <QtCore/private/qnumeric_p.h>
#include <QtCore/qloggingcategory.h>
#include <qqmlengine.h>
#include "PageReservation.h"
#include "PageAllocation.h"
#include "PageAllocationAligned.h"
#include "StdLibExtras.h"

#include <QElapsedTimer>
#include <QMap>
#include <QScopedValueRollback>

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include "qv4alloca_p.h"
#include "qv4profiling_p.h"
#include "qv4mapobject_p.h"
#include "qv4setobject_p.h"

//#define MM_STATS

#if !defined(MM_STATS) && !defined(QT_NO_DEBUG)
#define MM_STATS
#endif

#if MM_DEBUG
#define DEBUG qDebug() << "MM:"
#else
#define DEBUG if (1) ; else qDebug() << "MM:"
#endif

#ifdef V4_USE_VALGRIND
#include <valgrind/valgrind.h>
#include <valgrind/memcheck.h>
#endif

#ifdef V4_USE_HEAPTRACK
#include <heaptrack_api.h>
#endif

#if OS(QNX)
#include <sys/storage.h>   // __tls()
#endif

#if USE(PTHREADS) && HAVE(PTHREAD_NP_H)
#include <pthread_np.h>
#endif

Q_LOGGING_CATEGORY(lcGcStats, "qt.qml.gc.statistics")
Q_DECLARE_LOGGING_CATEGORY(lcGcStats)
Q_LOGGING_CATEGORY(lcGcAllocatorStats, "qt.qml.gc.allocatorStats")
Q_DECLARE_LOGGING_CATEGORY(lcGcAllocatorStats)

using namespace WTF;

QT_BEGIN_NAMESPACE

namespace QV4 {

enum {
    MinSlotsGCLimit = QV4::Chunk::AvailableSlots*16,
    GCOverallocation = 200 /* Max overallocation by the GC in % */
};

struct MemorySegment {
    enum {
        NumChunks = 8*sizeof(quint64),
        SegmentSize = NumChunks*Chunk::ChunkSize,
    };

    MemorySegment(size_t size)
    {
        size += Chunk::ChunkSize; // make sure we can get enough 64k aligment memory
        if (size < SegmentSize)
            size = SegmentSize;

        pageReservation = PageReservation::reserve(size, OSAllocator::JSGCHeapPages);
        base = reinterpret_cast<Chunk *>((reinterpret_cast<quintptr>(pageReservation.base()) + Chunk::ChunkSize - 1) & ~(Chunk::ChunkSize - 1));
        nChunks = NumChunks;
        availableBytes = size - (reinterpret_cast<quintptr>(base) - reinterpret_cast<quintptr>(pageReservation.base()));
        if (availableBytes < SegmentSize)
            --nChunks;
    }
    MemorySegment(MemorySegment &&other) {
        qSwap(pageReservation, other.pageReservation);
        qSwap(base, other.base);
        qSwap(allocatedMap, other.allocatedMap);
        qSwap(availableBytes, other.availableBytes);
        qSwap(nChunks, other.nChunks);
    }

    ~MemorySegment() {
        if (base)
            pageReservation.deallocate();
    }

    void setBit(size_t index) {
        Q_ASSERT(index < nChunks);
        quint64 bit = static_cast<quint64>(1) << index;
//        qDebug() << "    setBit" << hex << index << (index & (Bits - 1)) << bit;
        allocatedMap |= bit;
    }
    void clearBit(size_t index) {
        Q_ASSERT(index < nChunks);
        quint64 bit = static_cast<quint64>(1) << index;
//        qDebug() << "    setBit" << hex << index << (index & (Bits - 1)) << bit;
        allocatedMap &= ~bit;
    }
    bool testBit(size_t index) const {
        Q_ASSERT(index < nChunks);
        quint64 bit = static_cast<quint64>(1) << index;
        return (allocatedMap & bit);
    }

    Chunk *allocate(size_t size);
    void free(Chunk *chunk, size_t size) {
        DEBUG << "freeing chunk" << chunk;
        size_t index = static_cast<size_t>(chunk - base);
        size_t end = qMin(static_cast<size_t>(NumChunks), index + (size - 1)/Chunk::ChunkSize + 1);
        while (index < end) {
            Q_ASSERT(testBit(index));
            clearBit(index);
            ++index;
        }

        size_t pageSize = WTF::pageSize();
        size = (size + pageSize - 1) & ~(pageSize - 1);
#if !defined(Q_OS_LINUX) && !defined(Q_OS_WIN)
        // Linux and Windows zero out pages that have been decommitted and get committed again.
        // unfortunately that's not true on other OSes (e.g. BSD based ones), so zero out the
        // memory before decommit, so that we can be sure that all chunks we allocate will be
        // zero initialized.
        memset(chunk, 0, size);
#endif
        pageReservation.decommit(chunk, size);
    }

    bool contains(Chunk *c) const {
        return c >= base && c < base + nChunks;
    }

    PageReservation pageReservation;
    Chunk *base = nullptr;
    quint64 allocatedMap = 0;
    size_t availableBytes = 0;
    uint nChunks = 0;
};

Chunk *MemorySegment::allocate(size_t size)
{
    if (!allocatedMap && size >= SegmentSize) {
        // chunk allocated for one huge allocation
        Q_ASSERT(availableBytes >= size);
        pageReservation.commit(base, size);
        allocatedMap = ~static_cast<quint64>(0);
        return base;
    }
    size_t requiredChunks = (size + sizeof(Chunk) - 1)/sizeof(Chunk);
    uint sequence = 0;
    Chunk *candidate = nullptr;
    for (uint i = 0; i < nChunks; ++i) {
        if (!testBit(i)) {
            if (!candidate)
                candidate = base + i;
            ++sequence;
        } else {
            candidate = nullptr;
            sequence = 0;
        }
        if (sequence == requiredChunks) {
            pageReservation.commit(candidate, size);
            for (uint i = 0; i < requiredChunks; ++i)
                setBit(candidate - base + i);
            DEBUG << "allocated chunk " << candidate << hex << size;
            return candidate;
        }
    }
    return nullptr;
}

struct ChunkAllocator {
    ChunkAllocator() {}

    size_t requiredChunkSize(size_t size) {
        size += Chunk::HeaderSize; // space required for the Chunk header
        size_t pageSize = WTF::pageSize();
        size = (size + pageSize - 1) & ~(pageSize - 1); // align to page sizes
        if (size < Chunk::ChunkSize)
            size = Chunk::ChunkSize;
        return size;
    }

    Chunk *allocate(size_t size = 0);
    void free(Chunk *chunk, size_t size = 0);

    std::vector<MemorySegment> memorySegments;
};

Chunk *ChunkAllocator::allocate(size_t size)
{
    size = requiredChunkSize(size);
    for (auto &m : memorySegments) {
        if (~m.allocatedMap) {
            Chunk *c = m.allocate(size);
            if (c)
                return c;
        }
    }

    // allocate a new segment
    memorySegments.push_back(MemorySegment(size));
    Chunk *c = memorySegments.back().allocate(size);
    Q_ASSERT(c);
    return c;
}

void ChunkAllocator::free(Chunk *chunk, size_t size)
{
    size = requiredChunkSize(size);
    for (auto &m : memorySegments) {
        if (m.contains(chunk)) {
            m.free(chunk, size);
            return;
        }
    }
    Q_ASSERT(false);
}

#ifdef DUMP_SWEEP
QString binary(quintptr n) {
    QString s = QString::number(n, 2);
    while (s.length() < 64)
        s.prepend(QChar::fromLatin1('0'));
    return s;
}
#define SDUMP qDebug
#else
QString binary(quintptr) { return QString(); }
#define SDUMP if (1) ; else qDebug
#endif

// Stores a classname -> freed count mapping.
typedef QHash<const char*, int> MMStatsHash;
Q_GLOBAL_STATIC(MMStatsHash, freedObjectStatsGlobal)

// This indirection avoids sticking QHash code in each of the call sites, which
// shaves off some instructions in the case that it's unused.
static void increaseFreedCountForClass(const char *className)
{
    (*freedObjectStatsGlobal())[className]++;
}

//bool Chunk::sweep(ClassDestroyStatsCallback classCountPtr)
bool Chunk::sweep(ExecutionEngine *engine)
{
    bool hasUsedSlots = false;
    SDUMP() << "sweeping chunk" << this;
    HeapItem *o = realBase();
    bool lastSlotFree = false;
    for (uint i = 0; i < Chunk::EntriesInBitmap; ++i) {
#if WRITEBARRIER(none)
        Q_ASSERT((grayBitmap[i] | blackBitmap[i]) == blackBitmap[i]); // check that we don't have gray only objects
#endif
        quintptr toFree = objectBitmap[i] ^ blackBitmap[i];
        Q_ASSERT((toFree & objectBitmap[i]) == toFree); // check all black objects are marked as being used
        quintptr e = extendsBitmap[i];
        SDUMP() << "   index=" << i;
        SDUMP() << "        toFree      =" << binary(toFree);
        SDUMP() << "        black       =" << binary(blackBitmap[i]);
        SDUMP() << "        object      =" << binary(objectBitmap[i]);
        SDUMP() << "        extends     =" << binary(e);
        if (lastSlotFree)
            e &= (e + 1); // clear all lowest extent bits
        while (toFree) {
            uint index = qCountTrailingZeroBits(toFree);
            quintptr bit = (static_cast<quintptr>(1) << index);

            toFree ^= bit; // mask out freed slot
            //            DEBUG << "       index" << hex << index << toFree;

            // remove all extends slots that have been freed
            // this is a bit of bit trickery.
            quintptr mask = (bit << 1) - 1; // create a mask of 1's to the right of and up to the current bit
            quintptr objmask = e | mask; // or'ing mask with e gives all ones until the end of the current object
            quintptr result = objmask + 1;
            Q_ASSERT(qCountTrailingZeroBits(result) - index != 0); // ensure we freed something
            result |= mask; // ensure we don't clear stuff to the right of the current object
            e &= result;

            HeapItem *itemToFree = o + index;
            Heap::Base *b = *itemToFree;
            const VTable *v = b->internalClass->vtable;
//            if (Q_UNLIKELY(classCountPtr))
//                classCountPtr(v->className);
            if (v->destroy) {
                v->destroy(b);
                b->_checkIsDestroyed();
            }
#ifdef V4_USE_HEAPTRACK
            heaptrack_report_free(itemToFree);
#endif
        }
        Q_V4_PROFILE_DEALLOC(engine, qPopulationCount((objectBitmap[i] | extendsBitmap[i])
                                                      - (blackBitmap[i] | e)) * Chunk::SlotSize,
                             Profiling::SmallItem);
        objectBitmap[i] = blackBitmap[i];
        grayBitmap[i] = 0;
        hasUsedSlots |= (blackBitmap[i] != 0);
        extendsBitmap[i] = e;
        lastSlotFree = !((objectBitmap[i]|extendsBitmap[i]) >> (sizeof(quintptr)*8 - 1));
        SDUMP() << "        new extends =" << binary(e);
        SDUMP() << "        lastSlotFree" << lastSlotFree;
        Q_ASSERT((objectBitmap[i] & extendsBitmap[i]) == 0);
        o += Chunk::Bits;
    }
    //    DEBUG << "swept chunk" << this << "freed" << slotsFreed << "slots.";
    return hasUsedSlots;
}

void Chunk::freeAll(ExecutionEngine *engine)
{
    //    DEBUG << "sweeping chunk" << this << (*freeList);
    HeapItem *o = realBase();
    for (uint i = 0; i < Chunk::EntriesInBitmap; ++i) {
        quintptr toFree = objectBitmap[i];
        quintptr e = extendsBitmap[i];
        //        DEBUG << hex << "   index=" << i << toFree;
        while (toFree) {
            uint index = qCountTrailingZeroBits(toFree);
            quintptr bit = (static_cast<quintptr>(1) << index);

            toFree ^= bit; // mask out freed slot
            //            DEBUG << "       index" << hex << index << toFree;

            // remove all extends slots that have been freed
            // this is a bit of bit trickery.
            quintptr mask = (bit << 1) - 1; // create a mask of 1's to the right of and up to the current bit
            quintptr objmask = e | mask; // or'ing mask with e gives all ones until the end of the current object
            quintptr result = objmask + 1;
            Q_ASSERT(qCountTrailingZeroBits(result) - index != 0); // ensure we freed something
            result |= mask; // ensure we don't clear stuff to the right of the current object
            e &= result;

            HeapItem *itemToFree = o + index;
            Heap::Base *b = *itemToFree;
            if (b->internalClass->vtable->destroy) {
                b->internalClass->vtable->destroy(b);
                b->_checkIsDestroyed();
            }
#ifdef V4_USE_HEAPTRACK
            heaptrack_report_free(itemToFree);
#endif
        }
        Q_V4_PROFILE_DEALLOC(engine, (qPopulationCount(objectBitmap[i]|extendsBitmap[i])
                             - qPopulationCount(e)) * Chunk::SlotSize, Profiling::SmallItem);
        objectBitmap[i] = 0;
        grayBitmap[i] = 0;
        extendsBitmap[i] = e;
        o += Chunk::Bits;
    }
    //    DEBUG << "swept chunk" << this << "freed" << slotsFreed << "slots.";
}

void Chunk::resetBlackBits()
{
    memset(blackBitmap, 0, sizeof(blackBitmap));
}

void Chunk::collectGrayItems(MarkStack *markStack)
{
    //    DEBUG << "sweeping chunk" << this << (*freeList);
    HeapItem *o = realBase();
    for (uint i = 0; i < Chunk::EntriesInBitmap; ++i) {
#if WRITEBARRIER(none)
        Q_ASSERT((grayBitmap[i] | blackBitmap[i]) == blackBitmap[i]); // check that we don't have gray only objects
#endif
        quintptr toMark = blackBitmap[i] & grayBitmap[i]; // correct for a Steele type barrier
        Q_ASSERT((toMark & objectBitmap[i]) == toMark); // check all black objects are marked as being used
        //        DEBUG << hex << "   index=" << i << toFree;
        while (toMark) {
            uint index = qCountTrailingZeroBits(toMark);
            quintptr bit = (static_cast<quintptr>(1) << index);

            toMark ^= bit; // mask out marked slot
            //            DEBUG << "       index" << hex << index << toFree;

            HeapItem *itemToFree = o + index;
            Heap::Base *b = *itemToFree;
            Q_ASSERT(b->inUse());
            markStack->push(b);
        }
        grayBitmap[i] = 0;
        o += Chunk::Bits;
    }
    //    DEBUG << "swept chunk" << this << "freed" << slotsFreed << "slots.";

}

void Chunk::sortIntoBins(HeapItem **bins, uint nBins)
{
//    qDebug() << "sortIntoBins:";
    HeapItem *base = realBase();
#if QT_POINTER_SIZE == 8
    const int start = 0;
#else
    const int start = 1;
#endif
#ifndef QT_NO_DEBUG
    uint freeSlots = 0;
    uint allocatedSlots = 0;
#endif
    for (int i = start; i < EntriesInBitmap; ++i) {
        quintptr usedSlots = (objectBitmap[i]|extendsBitmap[i]);
#if QT_POINTER_SIZE == 8
        if (!i)
            usedSlots |= (static_cast<quintptr>(1) << (HeaderSize/SlotSize)) - 1;
#endif
#ifndef QT_NO_DEBUG
        allocatedSlots += qPopulationCount(usedSlots);
//        qDebug() << hex << "   i=" << i << "used=" << usedSlots;
#endif
        while (1) {
            uint index = qCountTrailingZeroBits(usedSlots + 1);
            if (index == Bits)
                break;
            uint freeStart = i*Bits + index;
            usedSlots &= ~((static_cast<quintptr>(1) << index) - 1);
            while (!usedSlots) {
                if (++i < EntriesInBitmap) {
                    usedSlots = (objectBitmap[i]|extendsBitmap[i]);
                } else {
                    Q_ASSERT(i == EntriesInBitmap);
                    // Overflows to 0 when counting trailing zeroes above in next iteration.
                    // Then, all the bits are zeroes and we break.
                    usedSlots = std::numeric_limits<quintptr>::max();
                    break;
                }
#ifndef QT_NO_DEBUG
                allocatedSlots += qPopulationCount(usedSlots);
//                qDebug() << hex << "   i=" << i << "used=" << usedSlots;
#endif
            }
            HeapItem *freeItem = base + freeStart;

            index = qCountTrailingZeroBits(usedSlots);
            usedSlots |= (quintptr(1) << index) - 1;
            uint freeEnd = i*Bits + index;
            uint nSlots = freeEnd - freeStart;
#ifndef QT_NO_DEBUG
//            qDebug() << hex << "   got free slots from" << freeStart << "to" << freeEnd << "n=" << nSlots << "usedSlots=" << usedSlots;
            freeSlots += nSlots;
#endif
            Q_ASSERT(freeEnd > freeStart && freeEnd <= NumSlots);
            freeItem->freeData.availableSlots = nSlots;
            uint bin = qMin(nBins - 1, nSlots);
            freeItem->freeData.next = bins[bin];
            bins[bin] = freeItem;
        }
    }
#ifndef QT_NO_DEBUG
    Q_ASSERT(freeSlots + allocatedSlots == (EntriesInBitmap - start) * 8 * sizeof(quintptr));
#endif
}

HeapItem *BlockAllocator::allocate(size_t size, bool forceAllocation) {
    Q_ASSERT((size % Chunk::SlotSize) == 0);
    size_t slotsRequired = size >> Chunk::SlotSizeShift;

    if (allocationStats)
        ++allocationStats[binForSlots(slotsRequired)];

    HeapItem **last;

    HeapItem *m;

    if (slotsRequired < NumBins - 1) {
        m = freeBins[slotsRequired];
        if (m) {
            freeBins[slotsRequired] = m->freeData.next;
            goto done;
        }
    }

    if (nFree >= slotsRequired) {
        // use bump allocation
        Q_ASSERT(nextFree);
        m = nextFree;
        nextFree += slotsRequired;
        nFree -= slotsRequired;
        goto done;
    }

    //        DEBUG << "No matching bin found for item" << size << bin;
    // search last bin for a large enough item
    last = &freeBins[NumBins - 1];
    while ((m = *last)) {
        if (m->freeData.availableSlots >= slotsRequired) {
            *last = m->freeData.next; // take it out of the list

            size_t remainingSlots = m->freeData.availableSlots - slotsRequired;
            //                DEBUG << "found large free slots of size" << m->freeData.availableSlots << m << "remaining" << remainingSlots;
            if (remainingSlots == 0)
                goto done;

            HeapItem *remainder = m + slotsRequired;
            if (remainingSlots > nFree) {
                if (nFree) {
                    size_t bin = binForSlots(nFree);
                    nextFree->freeData.next = freeBins[bin];
                    nextFree->freeData.availableSlots = nFree;
                    freeBins[bin] = nextFree;
                }
                nextFree = remainder;
                nFree = remainingSlots;
            } else {
                remainder->freeData.availableSlots = remainingSlots;
                size_t binForRemainder = binForSlots(remainingSlots);
                remainder->freeData.next = freeBins[binForRemainder];
                freeBins[binForRemainder] = remainder;
            }
            goto done;
        }
        last = &m->freeData.next;
    }

    if (slotsRequired < NumBins - 1) {
        // check if we can split up another slot
        for (size_t i = slotsRequired + 1; i < NumBins - 1; ++i) {
            m = freeBins[i];
            if (m) {
                freeBins[i] = m->freeData.next; // take it out of the list
//                qDebug() << "got item" << slotsRequired << "from slot" << i;
                size_t remainingSlots = i - slotsRequired;
                Q_ASSERT(remainingSlots < NumBins - 1);
                HeapItem *remainder = m + slotsRequired;
                remainder->freeData.availableSlots = remainingSlots;
                remainder->freeData.next = freeBins[remainingSlots];
                freeBins[remainingSlots] = remainder;
                goto done;
            }
        }
    }

    if (!m) {
        if (!forceAllocation)
            return nullptr;
        Chunk *newChunk = chunkAllocator->allocate();
        Q_V4_PROFILE_ALLOC(engine, Chunk::DataSize, Profiling::HeapPage);
        chunks.push_back(newChunk);
        nextFree = newChunk->first();
        nFree = Chunk::AvailableSlots;
        m = nextFree;
        nextFree += slotsRequired;
        nFree -= slotsRequired;
    }

done:
    m->setAllocatedSlots(slotsRequired);
    Q_V4_PROFILE_ALLOC(engine, slotsRequired * Chunk::SlotSize, Profiling::SmallItem);
#ifdef V4_USE_HEAPTRACK
    heaptrack_report_alloc(m, slotsRequired * Chunk::SlotSize);
#endif
    //        DEBUG << "   " << hex << m->chunk() << m->chunk()->objectBitmap[0] << m->chunk()->extendsBitmap[0] << (m - m->chunk()->realBase());
    return m;
}

void BlockAllocator::sweep()
{
    nextFree = nullptr;
    nFree = 0;
    memset(freeBins, 0, sizeof(freeBins));

//    qDebug() << "BlockAlloc: sweep";
    usedSlotsAfterLastSweep = 0;

    auto firstEmptyChunk = std::partition(chunks.begin(), chunks.end(), [this](Chunk *c) {
        return c->sweep(engine);
    });

    std::for_each(chunks.begin(), firstEmptyChunk, [this](Chunk *c) {
        c->sortIntoBins(freeBins, NumBins);
        usedSlotsAfterLastSweep += c->nUsedSlots();
    });

    // only free the chunks at the end to avoid that the sweep() calls indirectly
    // access freed memory
    std::for_each(firstEmptyChunk, chunks.end(), [this](Chunk *c) {
        Q_V4_PROFILE_DEALLOC(engine, Chunk::DataSize, Profiling::HeapPage);
        chunkAllocator->free(c);
    });

    chunks.erase(firstEmptyChunk, chunks.end());
}

void BlockAllocator::freeAll()
{
    for (auto c : chunks)
        c->freeAll(engine);
    for (auto c : chunks) {
        Q_V4_PROFILE_DEALLOC(engine, Chunk::DataSize, Profiling::HeapPage);
        chunkAllocator->free(c);
    }
}

void BlockAllocator::resetBlackBits()
{
    for (auto c : chunks)
        c->resetBlackBits();
}

void BlockAllocator::collectGrayItems(MarkStack *markStack)
{
    for (auto c : chunks)
        c->collectGrayItems(markStack);

}

HeapItem *HugeItemAllocator::allocate(size_t size) {
    MemorySegment *m = nullptr;
    Chunk *c = nullptr;
    if (size >= MemorySegment::SegmentSize/2) {
        // too large to handle through the ChunkAllocator, let's get our own memory segement
        size += Chunk::HeaderSize; // space required for the Chunk header
        size_t pageSize = WTF::pageSize();
        size = (size + pageSize - 1) & ~(pageSize - 1); // align to page sizes
        m = new MemorySegment(size);
        c = m->allocate(size);
    } else {
        c = chunkAllocator->allocate(size);
    }
    Q_ASSERT(c);
    chunks.push_back(HugeChunk{m, c, size});
    Chunk::setBit(c->objectBitmap, c->first() - c->realBase());
    Q_V4_PROFILE_ALLOC(engine, size, Profiling::LargeItem);
#ifdef V4_USE_HEAPTRACK
    heaptrack_report_alloc(c, size);
#endif
    return c->first();
}

static void freeHugeChunk(ChunkAllocator *chunkAllocator, const HugeItemAllocator::HugeChunk &c, ClassDestroyStatsCallback classCountPtr)
{
    HeapItem *itemToFree = c.chunk->first();
    Heap::Base *b = *itemToFree;
    const VTable *v = b->internalClass->vtable;
    if (Q_UNLIKELY(classCountPtr))
        classCountPtr(v->className);

    if (v->destroy) {
        v->destroy(b);
        b->_checkIsDestroyed();
    }
    if (c.segment) {
        // own memory segment
        c.segment->free(c.chunk, c.size);
        delete c.segment;
    } else {
        chunkAllocator->free(c.chunk, c.size);
    }
#ifdef V4_USE_HEAPTRACK
    heaptrack_report_free(c.chunk);
#endif
}

void HugeItemAllocator::sweep(ClassDestroyStatsCallback classCountPtr)
{
    auto isBlack = [this, classCountPtr] (const HugeChunk &c) {
        bool b = c.chunk->first()->isBlack();
        Chunk::clearBit(c.chunk->blackBitmap, c.chunk->first() - c.chunk->realBase());
        if (!b) {
            Q_V4_PROFILE_DEALLOC(engine, c.size, Profiling::LargeItem);
            freeHugeChunk(chunkAllocator, c, classCountPtr);
        }
        return !b;
    };

    auto newEnd = std::remove_if(chunks.begin(), chunks.end(), isBlack);
    chunks.erase(newEnd, chunks.end());
}

void HugeItemAllocator::resetBlackBits()
{
    for (auto c : chunks)
        Chunk::clearBit(c.chunk->blackBitmap, c.chunk->first() - c.chunk->realBase());
}

void HugeItemAllocator::collectGrayItems(MarkStack *markStack)
{
    for (auto c : chunks)
        // Correct for a Steele type barrier
        if (Chunk::testBit(c.chunk->blackBitmap, c.chunk->first() - c.chunk->realBase()) &&
            Chunk::testBit(c.chunk->grayBitmap, c.chunk->first() - c.chunk->realBase())) {
            HeapItem *i = c.chunk->first();
            Heap::Base *b = *i;
            b->mark(markStack);
        }
}

void HugeItemAllocator::freeAll()
{
    for (auto &c : chunks) {
        Q_V4_PROFILE_DEALLOC(engine, c.size, Profiling::LargeItem);
        freeHugeChunk(chunkAllocator, c, nullptr);
    }
}


MemoryManager::MemoryManager(ExecutionEngine *engine)
    : engine(engine)
    , chunkAllocator(new ChunkAllocator)
    , blockAllocator(chunkAllocator, engine)
    , icAllocator(chunkAllocator, engine)
    , hugeItemAllocator(chunkAllocator, engine)
    , m_persistentValues(new PersistentValueStorage(engine))
    , m_weakValues(new PersistentValueStorage(engine))
    , unmanagedHeapSizeGCLimit(MinUnmanagedHeapSizeGCLimit)
    , aggressiveGC(!qEnvironmentVariableIsEmpty("QV4_MM_AGGRESSIVE_GC"))
    , gcStats(lcGcStats().isDebugEnabled())
    , gcCollectorStats(lcGcAllocatorStats().isDebugEnabled())
{
#ifdef V4_USE_VALGRIND
    VALGRIND_CREATE_MEMPOOL(this, 0, true);
#endif
    memset(statistics.allocations, 0, sizeof(statistics.allocations));
    if (gcStats)
        blockAllocator.allocationStats = statistics.allocations;
}

Heap::Base *MemoryManager::allocString(std::size_t unmanagedSize)
{
    const size_t stringSize = align(sizeof(Heap::String));
#ifdef MM_STATS
    lastAllocRequestedSlots = stringSize >> Chunk::SlotSizeShift;
    ++allocationCount;
#endif
    unmanagedHeapSize += unmanagedSize;

    HeapItem *m = allocate(&blockAllocator, stringSize);
    memset(m, 0, stringSize);
    return *m;
}

Heap::Base *MemoryManager::allocData(std::size_t size)
{
#ifdef MM_STATS
    lastAllocRequestedSlots = size >> Chunk::SlotSizeShift;
    ++allocationCount;
#endif

    Q_ASSERT(size >= Chunk::SlotSize);
    Q_ASSERT(size % Chunk::SlotSize == 0);

    HeapItem *m = allocate(&blockAllocator, size);
    memset(m, 0, size);
    return *m;
}

Heap::Object *MemoryManager::allocObjectWithMemberData(const QV4::VTable *vtable, uint nMembers)
{
    uint size = (vtable->nInlineProperties + vtable->inlinePropertyOffset)*sizeof(Value);
    Q_ASSERT(!(size % sizeof(HeapItem)));

    Heap::Object *o;
    if (nMembers <= vtable->nInlineProperties) {
        o = static_cast<Heap::Object *>(allocData(size));
    } else {
        // Allocate both in one go through the block allocator
        nMembers -= vtable->nInlineProperties;
        std::size_t memberSize = align(sizeof(Heap::MemberData) + (nMembers - 1)*sizeof(Value));
        size_t totalSize = size + memberSize;
        Heap::MemberData *m;
        if (totalSize > Chunk::DataSize) {
            o = static_cast<Heap::Object *>(allocData(size));
            m = hugeItemAllocator.allocate(memberSize)->as<Heap::MemberData>();
        } else {
            HeapItem *mh = reinterpret_cast<HeapItem *>(allocData(totalSize));
            Heap::Base *b = *mh;
            o = static_cast<Heap::Object *>(b);
            mh += (size >> Chunk::SlotSizeShift);
            m = mh->as<Heap::MemberData>();
            Chunk *c = mh->chunk();
            size_t index = mh - c->realBase();
            Chunk::setBit(c->objectBitmap, index);
            Chunk::clearBit(c->extendsBitmap, index);
        }
        o->memberData.set(engine, m);
        m->internalClass.set(engine, engine->internalClasses(EngineBase::Class_MemberData));
        Q_ASSERT(o->memberData->internalClass);
        m->values.alloc = static_cast<uint>((memberSize - sizeof(Heap::MemberData) + sizeof(Value))/sizeof(Value));
        m->values.size = o->memberData->values.alloc;
        m->init();
//        qDebug() << "    got" << o->memberData << o->memberData->size;
    }
//    qDebug() << "allocating object with memberData" << o << o->memberData.operator->();
    return o;
}

static uint markStackSize = 0;

MarkStack::MarkStack(ExecutionEngine *engine)
    : engine(engine)
{
    base = (Heap::Base **)engine->gcStack->base();
    top = base;
    limit = base + ExecutionEngine::GCStackLimit/sizeof(Heap::Base)*3/4;
}

void MarkStack::drain()
{
    while (top > base) {
        Heap::Base *h = pop();
        ++markStackSize;
        Q_ASSERT(h); // at this point we should only have Heap::Base objects in this area on the stack. If not, weird things might happen.
        h->internalClass->vtable->markObjects(h, this);
    }
}

void MemoryManager::collectRoots(MarkStack *markStack)
{
    engine->markObjects(markStack);

//    qDebug() << "   mark stack after engine->mark" << (engine->jsStackTop - markBase);

    collectFromJSStack(markStack);

//    qDebug() << "   mark stack after js stack collect" << (engine->jsStackTop - markBase);
    m_persistentValues->mark(markStack);

//    qDebug() << "   mark stack after persistants" << (engine->jsStackTop - markBase);

    // Preserve QObject ownership rules within JavaScript: A parent with c++ ownership
    // keeps all of its children alive in JavaScript.

    // Do this _after_ collectFromStack to ensure that processing the weak
    // managed objects in the loop down there doesn't make then end up as leftovers
    // on the stack and thus always get collected.
    for (PersistentValueStorage::Iterator it = m_weakValues->begin(); it != m_weakValues->end(); ++it) {
        QObjectWrapper *qobjectWrapper = (*it).as<QObjectWrapper>();
        if (!qobjectWrapper)
            continue;
        QObject *qobject = qobjectWrapper->object();
        if (!qobject)
            continue;
        bool keepAlive = QQmlData::keepAliveDuringGarbageCollection(qobject);

        if (!keepAlive) {
            if (QObject *parent = qobject->parent()) {
                while (parent->parent())
                    parent = parent->parent();

                keepAlive = QQmlData::keepAliveDuringGarbageCollection(parent);
            }
        }

        if (keepAlive)
            qobjectWrapper->mark(markStack);

        if (markStack->top >= markStack->limit)
            markStack->drain();
    }
}

void MemoryManager::mark()
{
    markStackSize = 0;

    MarkStack markStack(engine);
    collectRoots(&markStack);

    markStack.drain();
}

void MemoryManager::sweep(bool lastSweep, ClassDestroyStatsCallback classCountPtr)
{
    for (PersistentValueStorage::Iterator it = m_weakValues->begin(); it != m_weakValues->end(); ++it) {
        Managed *m = (*it).managed();
        if (!m || m->markBit())
            continue;
        // we need to call destroyObject on qobjectwrappers now, so that they can emit the destroyed
        // signal before we start sweeping the heap
        if (QObjectWrapper *qobjectWrapper = (*it).as<QObjectWrapper>())
            qobjectWrapper->destroyObject(lastSweep);
    }

    // remove objects from weak maps and sets
    Heap::MapObject *map = weakMaps;
    Heap::MapObject **lastMap = &weakMaps;
    while (map) {
        if (map->isMarked()) {
            map->removeUnmarkedKeys();
            *lastMap = map;
            lastMap = &map->nextWeakMap;
        }
        map = map->nextWeakMap;
    }

    Heap::SetObject *set = weakSets;
    Heap::SetObject **lastSet = &weakSets;
    while (set) {
        if (set->isMarked()) {
            set->removeUnmarkedKeys();
            *lastSet = set;
            lastSet = &set->nextWeakSet;
        }
        set = set->nextWeakSet;
    }

    // onDestruction handlers may have accessed other QObject wrappers and reset their value, so ensure
    // that they are all set to undefined.
    for (PersistentValueStorage::Iterator it = m_weakValues->begin(); it != m_weakValues->end(); ++it) {
        Managed *m = (*it).managed();
        if (!m || m->markBit())
            continue;
        (*it) = Value::undefinedValue();
    }

    // Now it is time to free QV4::QObjectWrapper Value, we must check the Value's tag to make sure its object has been destroyed
    const int pendingCount = m_pendingFreedObjectWrapperValue.count();
    if (pendingCount) {
        QVector<Value *> remainingWeakQObjectWrappers;
        remainingWeakQObjectWrappers.reserve(pendingCount);
        for (int i = 0; i < pendingCount; ++i) {
            Value *v = m_pendingFreedObjectWrapperValue.at(i);
            if (v->isUndefined() || v->isEmpty())
                PersistentValueStorage::free(v);
            else
                remainingWeakQObjectWrappers.append(v);
        }
        m_pendingFreedObjectWrapperValue = remainingWeakQObjectWrappers;
    }

    if (MultiplyWrappedQObjectMap *multiplyWrappedQObjects = engine->m_multiplyWrappedQObjects) {
        for (MultiplyWrappedQObjectMap::Iterator it = multiplyWrappedQObjects->begin(); it != multiplyWrappedQObjects->end();) {
            if (!it.value().isNullOrUndefined())
                it = multiplyWrappedQObjects->erase(it);
            else
                ++it;
        }
    }


    if (!lastSweep) {
        engine->identifierTable->sweep();
        blockAllocator.sweep(/*classCountPtr*/);
        hugeItemAllocator.sweep(classCountPtr);
        icAllocator.sweep(/*classCountPtr*/);
    }
}

bool MemoryManager::shouldRunGC() const
{
    size_t total = blockAllocator.totalSlots() + icAllocator.totalSlots();
    if (total > MinSlotsGCLimit && usedSlotsAfterLastFullSweep * GCOverallocation < total * 100)
        return true;
    return false;
}

size_t dumpBins(BlockAllocator *b, bool printOutput = true)
{
    const QLoggingCategory &stats = lcGcAllocatorStats();
    size_t totalSlotMem = 0;
    if (printOutput)
        qDebug(stats) << "Slot map:";
    for (uint i = 0; i < BlockAllocator::NumBins; ++i) {
        uint nEntries = 0;
        HeapItem *h = b->freeBins[i];
        while (h) {
            ++nEntries;
            totalSlotMem += h->freeData.availableSlots;
            h = h->freeData.next;
        }
        if (printOutput)
            qDebug(stats) << "    number of entries in slot" << i << ":" << nEntries;
    }
    SDUMP() << "    large slot map";
    HeapItem *h = b->freeBins[BlockAllocator::NumBins - 1];
    while (h) {
        SDUMP() << "        " << hex << (quintptr(h)/32) << h->freeData.availableSlots;
        h = h->freeData.next;
    }

    if (printOutput)
        qDebug(stats) << "  total mem in bins" << totalSlotMem*Chunk::SlotSize;
    return totalSlotMem*Chunk::SlotSize;
}

void MemoryManager::runGC()
{
    if (gcBlocked) {
//        qDebug() << "Not running GC.";
        return;
    }

    QScopedValueRollback<bool> gcBlocker(gcBlocked, true);
//    qDebug() << "runGC";

    if (gcStats) {
        statistics.maxReservedMem = qMax(statistics.maxReservedMem, getAllocatedMem());
        statistics.maxAllocatedMem = qMax(statistics.maxAllocatedMem, getUsedMem() + getLargeItemsMem());
    }

    if (!gcCollectorStats) {
        mark();
        sweep();
    } else {
        bool triggeredByUnmanagedHeap = (unmanagedHeapSize > unmanagedHeapSizeGCLimit);
        size_t oldUnmanagedSize = unmanagedHeapSize;

        const size_t totalMem = getAllocatedMem();
        const size_t usedBefore = getUsedMem();
        const size_t largeItemsBefore = getLargeItemsMem();

        const QLoggingCategory &stats = lcGcAllocatorStats();
        qDebug(stats) << "========== GC ==========";
#ifdef MM_STATS
        qDebug(stats) << "    Triggered by alloc request of" << lastAllocRequestedSlots << "slots.";
        qDebug(stats) << "    Allocations since last GC" << allocationCount;
        allocationCount = 0;
#endif
        size_t oldChunks = blockAllocator.chunks.size();
        qDebug(stats) << "Allocated" << totalMem << "bytes in" << oldChunks << "chunks";
        qDebug(stats) << "Fragmented memory before GC" << (totalMem - usedBefore);
        dumpBins(&blockAllocator);

        QElapsedTimer t;
        t.start();
        mark();
        qint64 markTime = t.nsecsElapsed()/1000;
        t.restart();
        sweep(false, increaseFreedCountForClass);
        const size_t usedAfter = getUsedMem();
        const size_t largeItemsAfter = getLargeItemsMem();
        qint64 sweepTime = t.nsecsElapsed()/1000;

        if (triggeredByUnmanagedHeap) {
            qDebug(stats) << "triggered by unmanaged heap:";
            qDebug(stats) << "   old unmanaged heap size:" << oldUnmanagedSize;
            qDebug(stats) << "   new unmanaged heap:" << unmanagedHeapSize;
            qDebug(stats) << "   unmanaged heap limit:" << unmanagedHeapSizeGCLimit;
        }
        size_t memInBins = dumpBins(&blockAllocator);
        qDebug(stats) << "Marked object in" << markTime << "us.";
        qDebug(stats) << "   " << markStackSize << "objects marked";
        qDebug(stats) << "Sweeped object in" << sweepTime << "us.";

        // sort our object types by number of freed instances
        MMStatsHash freedObjectStats;
        std::swap(freedObjectStats, *freedObjectStatsGlobal());
        typedef std::pair<const char*, int> ObjectStatInfo;
        std::vector<ObjectStatInfo> freedObjectsSorted;
        freedObjectsSorted.reserve(freedObjectStats.count());
        for (auto it = freedObjectStats.constBegin(); it != freedObjectStats.constEnd(); ++it) {
            freedObjectsSorted.push_back(std::make_pair(it.key(), it.value()));
        }
        std::sort(freedObjectsSorted.begin(), freedObjectsSorted.end(), [](const ObjectStatInfo &a, const ObjectStatInfo &b) {
            return a.second > b.second && strcmp(a.first, b.first) < 0;
        });

        qDebug(stats) << "Used memory before GC:" << usedBefore;
        qDebug(stats) << "Used memory after GC:" << usedAfter;
        qDebug(stats) << "Freed up bytes      :" << (usedBefore - usedAfter);
        qDebug(stats) << "Freed up chunks     :" << (oldChunks - blockAllocator.chunks.size());
        size_t lost = blockAllocator.allocatedMem() - memInBins - usedAfter;
        if (lost)
            qDebug(stats) << "!!!!!!!!!!!!!!!!!!!!! LOST MEM:" << lost << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
        if (largeItemsBefore || largeItemsAfter) {
            qDebug(stats) << "Large item memory before GC:" << largeItemsBefore;
            qDebug(stats) << "Large item memory after GC:" << largeItemsAfter;
            qDebug(stats) << "Large item memory freed up:" << (largeItemsBefore - largeItemsAfter);
        }

        for (auto it = freedObjectsSorted.cbegin(); it != freedObjectsSorted.cend(); ++it) {
            qDebug(stats).noquote() << QString::fromLatin1("Freed JS type: %1 (%2 instances)").arg(QString::fromLatin1(it->first), QString::number(it->second));
        }

        qDebug(stats) << "======== End GC ========";
    }

    if (gcStats)
        statistics.maxUsedMem = qMax(statistics.maxUsedMem, getUsedMem() + getLargeItemsMem());

    if (aggressiveGC) {
        // ensure we don't 'loose' any memory
        Q_ASSERT(blockAllocator.allocatedMem()
                 == blockAllocator.usedMem() + dumpBins(&blockAllocator, false));
    }

    usedSlotsAfterLastFullSweep = blockAllocator.usedSlotsAfterLastSweep + icAllocator.usedSlotsAfterLastSweep;

    // reset all black bits
    blockAllocator.resetBlackBits();
    hugeItemAllocator.resetBlackBits();
    icAllocator.resetBlackBits();
}

size_t MemoryManager::getUsedMem() const
{
    return blockAllocator.usedMem() + icAllocator.usedMem();
}

size_t MemoryManager::getAllocatedMem() const
{
    return blockAllocator.allocatedMem() + icAllocator.allocatedMem() + hugeItemAllocator.usedMem();
}

size_t MemoryManager::getLargeItemsMem() const
{
    return hugeItemAllocator.usedMem();
}

void MemoryManager::registerWeakMap(Heap::MapObject *map)
{
    map->nextWeakMap = weakMaps;
    weakMaps = map;
}

void MemoryManager::registerWeakSet(Heap::SetObject *set)
{
    set->nextWeakSet = weakSets;
    weakSets = set;
}

MemoryManager::~MemoryManager()
{
    delete m_persistentValues;

    dumpStats();

    sweep(/*lastSweep*/true);
    blockAllocator.freeAll();
    hugeItemAllocator.freeAll();
    icAllocator.freeAll();

    delete m_weakValues;
#ifdef V4_USE_VALGRIND
    VALGRIND_DESTROY_MEMPOOL(this);
#endif
    delete chunkAllocator;
}


void MemoryManager::dumpStats() const
{
    if (!gcStats)
        return;

    const QLoggingCategory &stats = lcGcStats();
    qDebug(stats) << "Qml GC memory allocation statistics:";
    qDebug(stats) << "Total memory allocated:" << statistics.maxReservedMem;
    qDebug(stats) << "Max memory used before a GC run:" << statistics.maxAllocatedMem;
    qDebug(stats) << "Max memory used after a GC run:" << statistics.maxUsedMem;
    qDebug(stats) << "Requests for different item sizes:";
    for (int i = 1; i < BlockAllocator::NumBins - 1; ++i)
        qDebug(stats) << "     <" << (i << Chunk::SlotSizeShift) << " bytes: " << statistics.allocations[i];
    qDebug(stats) << "     >=" << ((BlockAllocator::NumBins - 1) << Chunk::SlotSizeShift) << " bytes: " << statistics.allocations[BlockAllocator::NumBins - 1];
}

void MemoryManager::collectFromJSStack(MarkStack *markStack) const
{
    Value *v = engine->jsStackBase;
    Value *top = engine->jsStackTop;
    while (v < top) {
        Managed *m = v->managed();
        if (m) {
            Q_ASSERT(m->inUse());
            // Skip pointers to already freed objects, they are bogus as well
            m->mark(markStack);
            if (markStack->top >= markStack->limit)
                markStack->drain();
        }
        ++v;
    }
}

} // namespace QV4

QT_END_NAMESPACE
