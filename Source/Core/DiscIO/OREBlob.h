// Copyright 2026 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <memory>
#include <optional>
#include <string>

#include "Common/CommonTypes.h"
#include "DiscIO/Blob.h"

struct smelt_ingot;

namespace DiscIO
{
// "ORE" and a layout digit, byteswapped to little endian: libsmelt refuses a
// layout it does not know by name
static constexpr u32 ORE_MAGIC_MASK = 0x00FFFFFF;
static constexpr u32 ORE_MAGIC = 0x0045524F;

bool IsOREMagic(u32 magic);

// An ORE, oxyromon's disc format, read through libsmelt as the disc image it
// was made from.
class OREFileReader final : public BlobReader
{
public:
  static std::unique_ptr<OREFileReader> Create(const std::string& path);
  ~OREFileReader() override;

  BlobType GetBlobType() const override { return BlobType::ORE; }
  std::unique_ptr<BlobReader> CopyReader() const override;

  u64 GetRawSize() const override { return m_raw_size; }
  u64 GetDataSize() const override { return m_size; }
  DataSizeType GetDataSizeType() const override { return DataSizeType::Accurate; }

  u64 GetBlockSize() const override { return 0; }
  bool HasFastRandomAccessInBlock() const override { return true; }
  std::string GetCompressionMethod() const override { return {}; }
  std::optional<int> GetCompressionLevel() const override { return std::nullopt; }

  bool Read(u64 offset, u64 nbytes, u8* out_ptr) override;

private:
  OREFileReader(smelt_ingot* ingot, u64 size, u64 raw_size);

  smelt_ingot* m_ingot;
  u64 m_size;
  u64 m_raw_size;
};

}  // namespace DiscIO
