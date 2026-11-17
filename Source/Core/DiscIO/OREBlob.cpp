// Copyright 2026 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "DiscIO/OREBlob.h"

#include <smelt.h>

#include "Common/FileUtil.h"
#include "Common/Logging/Log.h"

namespace DiscIO
{
bool IsOREMagic(u32 magic)
{
  const u8 layout = static_cast<u8>(magic >> 24);
  return (magic & ORE_MAGIC_MASK) == ORE_MAGIC && layout >= '1' && layout <= '9';
}

OREFileReader::OREFileReader(smelt_ingot* ingot, u64 size, u64 raw_size)
    : m_ingot(ingot), m_size(size), m_raw_size(raw_size)
{
}

OREFileReader::~OREFileReader()
{
  smelt_close(m_ingot);
}

// shortcut: opened by path, not through the frontend's VFS, as libsmelt reads
// files itself; a VFS-only frontend needs a reader callback in its ABI
std::unique_ptr<OREFileReader> OREFileReader::Create(const std::string& path)
{
  smelt_ingot* ingot = nullptr;
  u64 size = 0;
  if (smelt_open(path.c_str(), nullptr, &ingot) != SMELT_OK ||
      smelt_logical_size(ingot, &size) != SMELT_OK)
  {
    ERROR_LOG_FMT(DISCIO, "ORE {}: {}", path, smelt_last_error());
    smelt_close(ingot);
    return nullptr;
  }
  return std::unique_ptr<OREFileReader>(new OREFileReader(ingot, size, File::GetSize(path)));
}

// Another ingot over the same ORE, as each reader belongs to one thread
std::unique_ptr<BlobReader> OREFileReader::CopyReader() const
{
  smelt_ingot* ingot = nullptr;
  if (smelt_clone(m_ingot, &ingot) != SMELT_OK)
  {
    ERROR_LOG_FMT(DISCIO, "ORE clone: {}", smelt_last_error());
    return nullptr;
  }
  return std::unique_ptr<OREFileReader>(new OREFileReader(ingot, m_size, m_raw_size));
}

bool OREFileReader::Read(u64 offset, u64 nbytes, u8* out_ptr)
{
  if (smelt_read(m_ingot, offset, out_ptr, nbytes) != SMELT_OK)
  {
    ERROR_LOG_FMT(DISCIO, "ORE read of {} bytes at {}: {}", nbytes, offset, smelt_last_error());
    return false;
  }
  return true;
}

}  // namespace DiscIO
