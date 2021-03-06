/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkVertices.h"

#include "SkAtomics.h"
#include "SkData.h"
#include "SkReadBuffer.h"
#include "SkSafeMath.h"
#include "SkSafeRange.h"
#include "SkTo.h"
#include "SkWriteBuffer.h"
#include <new>

static int32_t gNextID = 1;
static int32_t next_id() {
    int32_t id;
    do {
        id = sk_atomic_inc(&gNextID);
    } while (id == SK_InvalidGenID);
    return id;
}

struct SkVertices::Sizes {
    Sizes(SkVertices::VertexMode mode, int vertexCount, int indexCount, bool hasTexs,
          bool hasColors, bool hasBones) {
        SkSafeMath safe;

        fVSize = safe.mul(vertexCount, sizeof(SkPoint));
        fTSize = hasTexs ? safe.mul(vertexCount, sizeof(SkPoint)) : 0;
        fCSize = hasColors ? safe.mul(vertexCount, sizeof(SkColor)) : 0;
        fBISize = hasBones ? safe.mul(vertexCount, sizeof(BoneIndices)) : 0;
        fBWSize = hasBones ? safe.mul(vertexCount, sizeof(BoneWeights)) : 0;

        fBuilderTriFanISize = 0;
        fISize = safe.mul(indexCount, sizeof(uint16_t));
        if (kTriangleFan_VertexMode == mode) {
            int numFanTris = 0;
            if (indexCount) {
                fBuilderTriFanISize = fISize;
                numFanTris = indexCount - 2;
            } else {
                numFanTris = vertexCount - 2;
                // By forcing this to become indexed we are adding a constraint to the maximum
                // number of vertices.
                if (vertexCount > (SkTo<int>(UINT16_MAX) + 1)) {
                    sk_bzero(this, sizeof(*this));
                    return;
                }
            }
            if (numFanTris <= 0) {
                sk_bzero(this, sizeof(*this));
                return;
            }
            fISize = safe.mul(numFanTris, 3 * sizeof(uint16_t));
        }

        fTotal = safe.add(sizeof(SkVertices),
                 safe.add(fVSize,
                 safe.add(fTSize,
                 safe.add(fCSize,
                 safe.add(fBISize,
                 safe.add(fBWSize,
                          fISize))))));

        if (safe.ok()) {
            fArrays = fTotal - sizeof(SkVertices);  // just the sum of the arrays
        } else {
            sk_bzero(this, sizeof(*this));
        }
    }

    bool isValid() const { return fTotal != 0; }

    size_t fTotal;  // size of entire SkVertices allocation (obj + arrays)
    size_t fArrays; // size of all the arrays (V + T + C + BI + BW + I)
    size_t fVSize;
    size_t fTSize;
    size_t fCSize;
    size_t fBISize;
    size_t fBWSize;
    size_t fISize;

    // For indexed tri-fans this is the number of amount of space fo indices needed in the builder
    // before conversion to indexed triangles (or zero if not indexed or not a triangle fan).
    size_t fBuilderTriFanISize;
};

SkVertices::Builder::Builder(VertexMode mode, int vertexCount, int indexCount,
                             uint32_t builderFlags) {
    bool hasTexs = SkToBool(builderFlags & SkVertices::kHasTexCoords_BuilderFlag);
    bool hasColors = SkToBool(builderFlags & SkVertices::kHasColors_BuilderFlag);
    bool hasBones = SkToBool(builderFlags & SkVertices::kHasBones_BuilderFlag);
    bool isVolatile = !SkToBool(builderFlags & SkVertices::kIsNonVolatile_BuilderFlag);
    this->init(mode, vertexCount, indexCount, isVolatile,
               SkVertices::Sizes(mode, vertexCount, indexCount, hasTexs, hasColors, hasBones));
}

SkVertices::Builder::Builder(VertexMode mode, int vertexCount, int indexCount, bool isVolatile,
                             const SkVertices::Sizes& sizes) {
    this->init(mode, vertexCount, indexCount, isVolatile, sizes);
}

void SkVertices::Builder::init(VertexMode mode, int vertexCount, int indexCount, bool isVolatile,
                               const SkVertices::Sizes& sizes) {
    if (!sizes.isValid()) {
        return; // fVertices will already be null
    }

    void* storage = ::operator new (sizes.fTotal);
    if (sizes.fBuilderTriFanISize) {
        fIntermediateFanIndices.reset(new uint8_t[sizes.fBuilderTriFanISize]);
    }

    fVertices.reset(new (storage) SkVertices);

    // need to point past the object to store the arrays
    char* ptr = (char*)storage + sizeof(SkVertices);

    fVertices->fPositions = (SkPoint*)ptr;                                  ptr += sizes.fVSize;
    fVertices->fTexs = sizes.fTSize ? (SkPoint*)ptr : nullptr;              ptr += sizes.fTSize;
    fVertices->fColors = sizes.fCSize ? (SkColor*)ptr : nullptr;            ptr += sizes.fCSize;
    fVertices->fBoneIndices = sizes.fBISize ? (BoneIndices*) ptr : nullptr; ptr += sizes.fBISize;
    fVertices->fBoneWeights = sizes.fBWSize ? (BoneWeights*) ptr : nullptr; ptr += sizes.fBWSize;
    fVertices->fIndices = sizes.fISize ? (uint16_t*)ptr : nullptr;
    fVertices->fVertexCnt = vertexCount;
    fVertices->fIndexCnt = indexCount;
    fVertices->fIsVolatile = isVolatile;
    fVertices->fMode = mode;

    // We defer assigning fBounds and fUniqueID until detach() is called
}

sk_sp<SkVertices> SkVertices::Builder::detach() {
    if (fVertices) {
        fVertices->fBounds.set(fVertices->fPositions, fVertices->fVertexCnt);
        if (fVertices->fMode == kTriangleFan_VertexMode) {
            if (fIntermediateFanIndices.get()) {
                SkASSERT(fVertices->fIndexCnt);
                auto tempIndices = this->indices();
                for (int t = 0; t < fVertices->fIndexCnt - 2; ++t) {
                    fVertices->fIndices[3 * t + 0] = tempIndices[0];
                    fVertices->fIndices[3 * t + 1] = tempIndices[t + 1];
                    fVertices->fIndices[3 * t + 2] = tempIndices[t + 2];
                }
                fVertices->fIndexCnt = 3 * (fVertices->fIndexCnt - 2);
            } else {
                SkASSERT(!fVertices->fIndexCnt);
                for (int t = 0; t < fVertices->fVertexCnt - 2; ++t) {
                    fVertices->fIndices[3 * t + 0] = 0;
                    fVertices->fIndices[3 * t + 1] = SkToU16(t + 1);
                    fVertices->fIndices[3 * t + 2] = SkToU16(t + 2);
                }
                fVertices->fIndexCnt = 3 * (fVertices->fVertexCnt - 2);
            }
            fVertices->fMode = kTriangles_VertexMode;
        }
        fVertices->fUniqueID = next_id();
        return std::move(fVertices);        // this will null fVertices after the return
    }
    return nullptr;
}

int SkVertices::Builder::vertexCount() const {
    return fVertices ? fVertices->vertexCount() : 0;
}

int SkVertices::Builder::indexCount() const {
    return fVertices ? fVertices->indexCount() : 0;
}

bool SkVertices::Builder::isVolatile() const {
    return fVertices ? fVertices->isVolatile() : true;
}

SkPoint* SkVertices::Builder::positions() {
    return fVertices ? const_cast<SkPoint*>(fVertices->positions()) : nullptr;
}

SkPoint* SkVertices::Builder::texCoords() {
    return fVertices ? const_cast<SkPoint*>(fVertices->texCoords()) : nullptr;
}

SkColor* SkVertices::Builder::colors() {
    return fVertices ? const_cast<SkColor*>(fVertices->colors()) : nullptr;
}

SkVertices::BoneIndices* SkVertices::Builder::boneIndices() {
    return fVertices ? const_cast<BoneIndices*>(fVertices->boneIndices()) : nullptr;
}

SkVertices::BoneWeights* SkVertices::Builder::boneWeights() {
    return fVertices ? const_cast<BoneWeights*>(fVertices->boneWeights()) : nullptr;
}

uint16_t* SkVertices::Builder::indices() {
    if (!fVertices) {
        return nullptr;
    }
    if (fIntermediateFanIndices) {
        return reinterpret_cast<uint16_t*>(fIntermediateFanIndices.get());
    }
    return const_cast<uint16_t*>(fVertices->indices());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkVertices> SkVertices::MakeCopy(VertexMode mode, int vertexCount,
                                       const SkPoint pos[], const SkPoint texs[],
                                       const SkColor colors[],
                                       const BoneIndices boneIndices[],
                                       const BoneWeights boneWeights[],
                                       int indexCount, const uint16_t indices[],
                                       bool isVolatile) {
    SkASSERT((!boneIndices && !boneWeights) || (boneIndices && boneWeights));
    Sizes sizes(mode,
                vertexCount,
                indexCount,
                texs != nullptr,
                colors != nullptr,
                boneIndices != nullptr);
    if (!sizes.isValid()) {
        return nullptr;
    }

    Builder builder(mode, vertexCount, indexCount, isVolatile, sizes);
    SkASSERT(builder.isValid());

    sk_careful_memcpy(builder.positions(), pos, sizes.fVSize);
    sk_careful_memcpy(builder.texCoords(), texs, sizes.fTSize);
    sk_careful_memcpy(builder.colors(), colors, sizes.fCSize);
    sk_careful_memcpy(builder.boneIndices(), boneIndices, sizes.fBISize);
    sk_careful_memcpy(builder.boneWeights(), boneWeights, sizes.fBWSize);
    size_t isize = (mode == kTriangleFan_VertexMode) ? sizes.fBuilderTriFanISize : sizes.fISize;
    sk_careful_memcpy(builder.indices(), indices, isize);

    return builder.detach();
}

size_t SkVertices::approximateSize() const {
    Sizes sizes(fMode,
                fVertexCnt,
                fIndexCnt,
                this->hasTexCoords(),
                this->hasColors(),
                this->hasBones());
    SkASSERT(sizes.isValid());
    return sizeof(SkVertices) + sizes.fArrays;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// storage = packed | vertex_count | index_count | pos[] | texs[] | colors[] | boneIndices[] |
//           boneWeights[] | indices[]
//         = header + arrays

#define kMode_Mask          0x0FF
#define kHasTexs_Mask       0x100
#define kHasColors_Mask     0x200
#define kHasBones_Mask      0x400
#define kIsNonVolatile_Mask 0x800
#define kHeaderSize         (3 * sizeof(uint32_t))

void SkVertices::encode(SkWriteBuffer& buffer) const {
    // packed has room for addtional flags in the future (e.g. versioning)
    uint32_t packed = static_cast<uint32_t>(fMode);
    SkASSERT((packed & ~kMode_Mask) == 0);  // our mode fits in the mask bits
    if (this->hasTexCoords()) {
        packed |= kHasTexs_Mask;
    }
    if (this->hasColors()) {
        packed |= kHasColors_Mask;
    }
    if (this->hasBones()) {
        packed |= kHasBones_Mask;
    }
    if (!this->isVolatile()) {
        packed |= kIsNonVolatile_Mask;
    }

    Sizes sizes(fMode,
                fVertexCnt,
                fIndexCnt,
                this->hasTexCoords(),
                this->hasColors(),
                this->hasBones());
    SkASSERT(sizes.isValid());
    SkASSERT(!sizes.fBuilderTriFanISize);
    buffer.writeUInt(packed);
    buffer.writeInt(fVertexCnt);
    buffer.writeInt(fIndexCnt);
    buffer.writeByteArray(fPositions, sizes.fVSize);
    buffer.writeByteArray(fTexs, sizes.fTSize);
    buffer.writeByteArray(fColors, sizes.fCSize);
    buffer.writeByteArray(fBoneIndices, sizes.fBISize);
    buffer.writeByteArray(fBoneWeights, sizes.fBWSize);
    // if index-count is odd, we won't be 4-bytes aligned, so we call the pad version
    buffer.writeByteArray(fIndices, sizes.fISize);
}

sk_sp<SkVertices> SkVertices::Decode(SkReadBuffer& buffer) {
    if (buffer.isVersionLT(SkReadBuffer::kVerticesUseReadBuffer_Version)) {
        // Old versions used an embedded blob that was serialized with SkWriter32/SkReader32.
        // We don't support loading those, but skip over the vertices to keep the buffer valid.
        auto data = buffer.readByteArrayAsData();
        (void)data;
         return nullptr;
    }

    auto decode = [](SkReadBuffer &buffer) -> sk_sp<SkVertices> {
        SkSafeRange safe;

        const uint32_t packed = buffer.readUInt();
        const int vertexCount = safe.checkGE(buffer.readInt(), 0);
        const int indexCount = safe.checkGE(buffer.readInt(), 0);
        const VertexMode mode = safe.checkLE<VertexMode>(packed & kMode_Mask,
                                                     SkVertices::kLast_VertexMode);
        if (!safe) {
            return nullptr;
        }
        const bool hasTexs = SkToBool(packed & kHasTexs_Mask);
        const bool hasColors = SkToBool(packed & kHasColors_Mask);
        const bool hasBones = SkToBool(packed & kHasBones_Mask);
        const bool isVolatile = !SkToBool(packed & kIsNonVolatile_Mask);
        Sizes sizes(mode, vertexCount, indexCount, hasTexs, hasColors, hasBones);
        if (!sizes.isValid()) {
            return nullptr;
        }

        Builder builder(mode, vertexCount, indexCount, isVolatile, sizes);
        if (!builder.isValid()) {
            return nullptr;
        }
        buffer.readByteArray(builder.positions(), sizes.fVSize);
        buffer.readByteArray(builder.texCoords(), sizes.fTSize);
        buffer.readByteArray(builder.colors(), sizes.fCSize);
        buffer.readByteArray(builder.boneIndices(), sizes.fBISize);
        buffer.readByteArray(builder.boneWeights(), sizes.fBWSize);
        size_t isize = (mode == kTriangleFan_VertexMode) ? sizes.fBuilderTriFanISize : sizes.fISize;
        buffer.readByteArray(builder.indices(), isize);
        if (!buffer.isValid()) {
            return nullptr;
        }
        if (indexCount > 0) {
            // validate that the indicies are in range
            SkASSERT(indexCount == builder.indexCount());
            const uint16_t* indices = builder.indices();
            for (int i = 0; i < indexCount; ++i) {
                if (indices[i] >= (unsigned)vertexCount) {
                    return nullptr;
                }
            }
        }
        return builder.detach();
    };

    if (auto verts = decode(buffer)) {
        return verts;
    }

    buffer.validate(false);
    return nullptr;
}

void SkVertices::operator delete(void* p)
{
    ::operator delete(p);
}
