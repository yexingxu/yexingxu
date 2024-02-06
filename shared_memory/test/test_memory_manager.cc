#include "memory/chunk_header.hpp"
#include "memory/memory_manager.hpp"
#include "memory/segment.hpp"
// #include "shm/utils.h"
#include "test.hpp"

namespace {
using namespace ::testing;
using namespace shm::memory;
using namespace shm::details;

using UserPayloadOffset_t = ChunkHeader::UserPayloadOffset_t;

class MemoryManager_test : public Test {
 public:
  void SetUp() override {
    rawMemory = malloc(rawMemorySize);
    allocator = new BumpAllocator(rawMemory, rawMemorySize);
    sut = new MemoryManager();
  };
  void TearDown() override {
    delete sut;
    delete allocator;
    free(rawMemory);
  };

  using ChunkStore = std::vector<SharedChunk>;
  ChunkStore getChunksFromSut(const uint32_t numberOfChunks,
                              const ChunkSettings& chunkSettings) {
    ChunkStore chunkStore;
    for (uint32_t i = 0; i < numberOfChunks; ++i) {
      auto chunk = sut->getChunk(chunkSettings);
      if (chunk.has_value()) {
        EXPECT_TRUE(chunk.value());
        chunkStore.push_back(chunk.value());
      } else {
        // GTEST_FAIL() << "getChunk failed with: "
        //              << asStringLiteral(chunk.error());
      }
    }
    return chunkStore;
  }

  static constexpr uint32_t CHUNK_SIZE_32{32U};
  static constexpr uint32_t CHUNK_SIZE_64{64U};
  static constexpr uint32_t CHUNK_SIZE_128{128};
  static constexpr uint32_t CHUNK_SIZE_256{256U};

  BumpAllocator* allocator;
  void* rawMemory;
  size_t rawMemorySize = 1000000;

  MemoryManager* sut;
  Config mempoolconf;

  const ChunkSettings chunkSettings_32{
      ChunkSettings::create(32U, CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT).value()};
  const ChunkSettings chunkSettings_64{
      ChunkSettings::create(64U, CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT).value()};
  const ChunkSettings chunkSettings_128{
      ChunkSettings::create(128U, CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT)
          .value()};
  const ChunkSettings chunkSettings_256{
      ChunkSettings::create(256, CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT).value()};
};

// TEST_F(MemoryManager_test, AddingMempoolNotInTheIncreasingOrderReturnsError)
// {
//   ::testing::Test::RecordProperty("TEST_ID",
//                                   "df439901-8d42-4532-8494-21d5447ef7d7");
//   constexpr uint32_t CHUNK_COUNT{10U};
//   mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
//   mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
//   mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
//   mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
//   optional<PoshError> detectedError;
//   auto errorHandlerGuard =
//       ErrorHandlerMock::setTemporaryErrorHandler<PoshError>(
//           [&detectedError](const PoshError error, const ErrorLevel
//           errorLevel) {
//             detectedError.emplace(error);
//             EXPECT_EQ(errorLevel, ErrorLevel::FATAL);
//           });

//   sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

//   ASSERT_TRUE(detectedError.has_value());
//   EXPECT_EQ(
//       detectedError.value(),
//       PoshError::MEPOO__MEMPOOL_CONFIG_MUST_BE_ORDERED_BY_INCREASING_SIZE);
// }

// TEST_F(MemoryManager_test, WrongCallOfConfigureMemoryManagerReturnsError) {
//   ::testing::Test::RecordProperty("TEST_ID",
//                                   "04cbf769-8721-454b-b038-96dd467ac3c2");
//   constexpr uint32_t CHUNK_COUNT{10U};
//   mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
//   mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
//   sut->configureMemoryManager(mempoolconf, *allocator, *allocator);
//   optional<PoshError> detectedError;
//   auto errorHandlerGuard =
//       ErrorHandlerMock::setTemporaryErrorHandler<PoshError>(
//           [&detectedError](const PoshError error, const ErrorLevel
//           errorLevel) {
//             detectedError.emplace(error);
//             EXPECT_EQ(errorLevel, ErrorLevel::FATAL);
//           });

//   sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

//   ASSERT_TRUE(detectedError.has_value());
//   EXPECT_EQ(
//       detectedError.value(),
//       PoshError::MEPOO__MEMPOOL_ADDMEMPOOL_AFTER_GENERATECHUNKMANAGEMENTPOOL);
// }

TEST_F(
    MemoryManager_test,
    GetMempoolInfoMethodForOutOfBoundaryMempoolIndexReturnsZeroForAllMempoolAttributes) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "897e635e-d6df-46be-a3f0-7373b4116102");
  constexpr uint32_t CHUNK_COUNT{10U};
  mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
  constexpr uint32_t INVALID_MEMPOOL_INDEX = 2U;
  sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

  MemPoolInfo poolInfo = sut->getMemPoolInfo(INVALID_MEMPOOL_INDEX);

  EXPECT_EQ(poolInfo.m_chunkSize, 0U);
  EXPECT_EQ(poolInfo.m_minFreeChunks, 0U);
  EXPECT_EQ(poolInfo.m_numChunks, 0U);
  EXPECT_EQ(poolInfo.m_usedChunks, 0U);
}

TEST_F(MemoryManager_test,
       GetNumberOfMemPoolsMethodReturnsTheNumberOfMemPools) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "e3c05153-add7-4098-8045-88960970d2a7");
  constexpr uint32_t CHUNK_COUNT{10U};
  mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});

  sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

  EXPECT_EQ(sut->getNumberOfMemPools(), 3U);
}

// TEST_F(MemoryManager_test,
// GetChunkMethodWithNoMemPoolInMemConfigReturnsError) {
//   ::testing::Test::RecordProperty("TEST_ID",
//                                   "dff31ea2-8ae0-4786-8c97-633af59c287d");
//   optional<PoshError> detectedError;
//   auto errorHandlerGuard =
//       ErrorHandlerMock::setTemporaryErrorHandler<PoshError>(
//           [&detectedError](const PoshError error, const ErrorLevel
//           errorLevel) {
//             detectedError.emplace(error);
//             EXPECT_EQ(errorLevel, ErrorLevel::SEVERE);
//           });

//   constexpr uint32_t USER_PAYLOAD_SIZE{15U};
//   auto chunkSettingsResult = ChunkSettings::create(
//       USER_PAYLOAD_SIZE, CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
//   ASSERT_FALSE(chunkSettingsResult.has_error());
//   auto& chunkSettings = chunkSettingsResult.value();

//   constexpr auto EXPECTED_ERROR{MemoryManager::Error::NO_MEMPOOLS_AVAILABLE};
//   sut->getChunk(chunkSettings)
//       .and_then([&](auto&) {
//         GTEST_FAIL() << "getChunk should fail with '" << EXPECTED_ERROR
//                      << "' but did not fail";
//       })
//       .or_else([&](const auto& error) { EXPECT_EQ(error, EXPECTED_ERROR); });

//   ASSERT_TRUE(detectedError.has_value());
//   EXPECT_EQ(detectedError.value(),
//             PoshError::MEPOO__MEMPOOL_GETCHUNK_CHUNK_WITHOUT_MEMPOOL);
// }

// TEST_F(
//     MemoryManager_test,
//     GetChunkMethodWithChunkSizeGreaterThanAvailableChunkSizeInMemPoolConfigReturnsError)
//     {
//   ::testing::Test::RecordProperty("TEST_ID",
//                                   "d2104b74-5092-484d-bb9d-554996dffbc5");
//   constexpr uint32_t CHUNK_COUNT{10U};
//   mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
//   mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
//   mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
//   sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

//   optional<PoshError> detectedError;
//   auto errorHandlerGuard =
//       ErrorHandlerMock::setTemporaryErrorHandler<PoshError>(
//           [&detectedError](const PoshError error, const ErrorLevel
//           errorLevel) {
//             detectedError.emplace(error);
//             EXPECT_EQ(errorLevel, ErrorLevel::SEVERE);
//           });

//   constexpr uint32_t USER_PAYLOAD_SIZE{200U};
//   auto chunkSettingsResult = ChunkSettings::create(
//       USER_PAYLOAD_SIZE, CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
//   ASSERT_FALSE(chunkSettingsResult.has_error());
//   auto& chunkSettings = chunkSettingsResult.value();

//   constexpr auto EXPECTED_ERROR{
//       MemoryManager::Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE};
//   sut->getChunk(chunkSettings)
//       .and_then([&](auto&) {
//         GTEST_FAIL() << "getChunk should fail with '" << EXPECTED_ERROR
//                      << "' but did not fail";
//       })
//       .or_else([&](const auto& error) { EXPECT_EQ(error, EXPECTED_ERROR); });

//   ASSERT_TRUE(detectedError.has_value());
//   EXPECT_EQ(detectedError.value(),
//             PoshError::MEPOO__MEMPOOL_GETCHUNK_CHUNK_IS_TOO_LARGE);
// }

// TEST_F(MemoryManager_test,
//        GetChunkMethodWhenNoFreeChunksInMemPoolConfigReturnsError) {
//   ::testing::Test::RecordProperty("TEST_ID",
//                                   "0f201458-040e-43b1-a51b-698c2957ca7c");
//   constexpr uint32_t CHUNK_COUNT{1U};
//   constexpr uint32_t PAYLOAD_SIZE{100U};
//   mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
//   sut->configureMemoryManager(mempoolconf, *allocator, *allocator);
//   auto chunkSettingsResult =
//       ChunkSettings::create(PAYLOAD_SIZE,
//       CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
//   ASSERT_FALSE(chunkSettingsResult.has_error());
//   auto& chunkSettings = chunkSettingsResult.value();
//   auto chunkStore = getChunksFromSut(CHUNK_COUNT, chunkSettings);

//   optional<PoshError> detectedError;
//   auto errorHandlerGuard =
//       ErrorHandlerMock::setTemporaryErrorHandler<PoshError>(
//           [&detectedError](const PoshError error, const ErrorLevel
//           errorLevel) {
//             detectedError.emplace(error);
//             EXPECT_EQ(errorLevel, ErrorLevel::MODERATE);
//           });

//   constexpr auto EXPECTED_ERROR{MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS};
//   sut->getChunk(chunkSettings)
//       .and_then([&](auto&) {
//         GTEST_FAIL() << "getChunk should fail with '" << EXPECTED_ERROR
//                      << "' but did not fail";
//       })
//       .or_else([&](const auto& error) { EXPECT_EQ(error, EXPECTED_ERROR); });

//   ASSERT_TRUE(detectedError.has_value());
//   EXPECT_EQ(detectedError.value(),
//             PoshError::MEPOO__MEMPOOL_GETCHUNK_POOL_IS_RUNNING_OUT_OF_CHUNKS);
// }

TEST_F(MemoryManager_test,
       VerifyGetChunkMethodWhenTheRequestedChunkIsAvailableInMemPoolConfig) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "a5069a9d-ae2f-4466-ae13-53b0794dd292");
  constexpr uint32_t CHUNK_COUNT{10U};
  mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
  sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

  constexpr uint32_t USER_PAYLOAD_SIZE{50U};
  auto chunkSettingsResult = ChunkSettings::create(
      USER_PAYLOAD_SIZE, CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
  ASSERT_TRUE(chunkSettingsResult.has_value());
  auto& chunkSettings = chunkSettingsResult.value();

  auto chunk = sut->getChunk(chunkSettings);
  if (chunk.has_value()) {
    EXPECT_TRUE(chunk);
  } else {
    GTEST_FAIL() << "getChunk failed with: " << asStringLiteral(chunk.error());
  }
}

TEST_F(MemoryManager_test, getChunkSingleMemPoolAllChunks) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "6623841b-baf0-4636-a5d4-b21e4678b7e8");
  constexpr uint32_t CHUNK_COUNT{100U};

  mempoolconf.addMemPool({128, CHUNK_COUNT});
  sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

  constexpr uint32_t USER_PAYLOAD_SIZE{50U};
  auto chunkSettingsResult = ChunkSettings::create(
      USER_PAYLOAD_SIZE, CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
  ASSERT_TRUE(chunkSettingsResult.has_value());
  auto& chunkSettings = chunkSettingsResult.value();

  auto chunkStore = getChunksFromSut(CHUNK_COUNT, chunkSettings);

  EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, CHUNK_COUNT);
}

TEST_F(MemoryManager_test, getChunkSingleMemPoolToMuchChunks) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "8af072c7-425b-4820-bc28-0c1e6bad0441");
  constexpr uint32_t CHUNK_COUNT{100U};

  mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
  sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

  auto chunkStore = getChunksFromSut(CHUNK_COUNT, chunkSettings_128);

  constexpr auto EXPECTED_ERROR{MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS};
  auto chunk = sut->getChunk(chunkSettings_128);
  if (chunk.has_value()) {
    GTEST_FAIL() << "getChunk should fail with '"
                 << asStringLiteral(EXPECTED_ERROR) << "' but did not fail";
  } else {
    EXPECT_EQ(chunk.error(), EXPECTED_ERROR);
  }
}

TEST_F(MemoryManager_test, freeChunkSingleMemPoolFullToEmptyToFull) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "0cbb56ae-c5a3-46b6-bfab-abbf91b0ed64");
  constexpr uint32_t CHUNK_COUNT{100U};

  // chunks are freed when they go out of scope
  {
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    auto chunkStore = getChunksFromSut(CHUNK_COUNT, chunkSettings_128);

    EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, CHUNK_COUNT);
  }

  EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, 0U);

  auto chunkStore = getChunksFromSut(CHUNK_COUNT, chunkSettings_128);

  EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, CHUNK_COUNT);
}

TEST_F(MemoryManager_test, getChunkMultiMemPoolSingleChunk) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "b22f804d-2e12-40c3-8bec-f9a0ab375e98");
  constexpr uint32_t CHUNK_COUNT{10U};

  mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});

  sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

  for (const auto& chunkSettings : {chunkSettings_32, chunkSettings_64,
                                    chunkSettings_128, chunkSettings_256}) {
    auto chunk = sut->getChunk(chunkSettings);

    if (chunk.has_value()) {
      EXPECT_TRUE(chunk);
    } else {
      GTEST_FAIL() << "getChunk failed with: "
                   << asStringLiteral(chunk.error());
    }
  }
}

TEST_F(MemoryManager_test, getChunkMultiMemPoolAllChunks) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "cdb65377-7579-4c76-9931-9552071fff5c");
  constexpr uint32_t CHUNK_COUNT{100U};

  mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
  sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

  auto chunkStore_32 = getChunksFromSut(CHUNK_COUNT, chunkSettings_32);
  auto chunkStore_64 = getChunksFromSut(CHUNK_COUNT, chunkSettings_64);
  auto chunkStore_128 = getChunksFromSut(CHUNK_COUNT, chunkSettings_128);
  auto chunkStore_256 = getChunksFromSut(CHUNK_COUNT, chunkSettings_256);

  EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(CHUNK_COUNT));
  EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(CHUNK_COUNT));
  EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(CHUNK_COUNT));
  EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(CHUNK_COUNT));
}

TEST_F(MemoryManager_test, getChunkMultiMemPoolTooMuchChunks) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "975e1347-9b39-4fda-a95a-0725b04f7d7d");
  constexpr uint32_t CHUNK_COUNT{100U};

  mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
  sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

  auto chunkStore_32 = getChunksFromSut(CHUNK_COUNT, chunkSettings_32);
  auto chunkStore_64 = getChunksFromSut(CHUNK_COUNT, chunkSettings_64);
  auto chunkStore_128 = getChunksFromSut(CHUNK_COUNT, chunkSettings_128);
  auto chunkStore_256 = getChunksFromSut(CHUNK_COUNT, chunkSettings_256);

  constexpr auto EXPECTED_ERROR{MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS};

  for (const auto& chunkSettings : {chunkSettings_32, chunkSettings_64,
                                    chunkSettings_128, chunkSettings_256}) {
    auto chunk = sut->getChunk(chunkSettings);
    if (chunk.has_value()) {
      GTEST_FAIL() << "getChunk for payload size "
                   << chunkSettings.userPayloadSize() << " should fail with '"
                   << asStringLiteral(EXPECTED_ERROR) << "' but did not fail";
    } else {
      EXPECT_EQ(chunk.error(), EXPECTED_ERROR);
    }
  }
}

TEST_F(MemoryManager_test,
       emptyMemPoolDoesNotResultInAcquiringChunksFromOtherMemPools) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "c2ff3937-6cdf-43ad-bec6-5203521a8f57");
  constexpr uint32_t CHUNK_COUNT{100};

  mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
  mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
  sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

  auto chunkStore = getChunksFromSut(CHUNK_COUNT, chunkSettings_64);

  constexpr auto EXPECTED_ERROR{MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS};
  auto chunk = sut->getChunk(chunkSettings_64);
  if (chunk.has_value()) {
    GTEST_FAIL() << "getChunk should fail with '"
                 << asStringLiteral(EXPECTED_ERROR) << "' but did not fail";
  } else {
    EXPECT_EQ(chunk.error(), EXPECTED_ERROR);
  }

  EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(0U));
  EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(CHUNK_COUNT));
  EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(0U));
  EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(0U));
}

TEST_F(MemoryManager_test, freeChunkMultiMemPoolFullToEmptyToFull) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "0eddc5b5-e28f-43df-9da7-2c12014284a5");
  constexpr uint32_t CHUNK_COUNT{100U};

  // chunks are freed when they go out of scope
  {
    std::vector<SharedChunk> chunkStore;

    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    auto chunkStore_32 = getChunksFromSut(CHUNK_COUNT, chunkSettings_32);
    auto chunkStore_64 = getChunksFromSut(CHUNK_COUNT, chunkSettings_64);
    auto chunkStore_128 = getChunksFromSut(CHUNK_COUNT, chunkSettings_128);
    auto chunkStore_256 = getChunksFromSut(CHUNK_COUNT, chunkSettings_256);

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(CHUNK_COUNT));
  }

  EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(0U));
  EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(0U));
  EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(0U));
  EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(0U));

  auto chunkStore_32 = getChunksFromSut(CHUNK_COUNT, chunkSettings_32);
  auto chunkStore_64 = getChunksFromSut(CHUNK_COUNT, chunkSettings_64);
  auto chunkStore_128 = getChunksFromSut(CHUNK_COUNT, chunkSettings_128);
  auto chunkStore_256 = getChunksFromSut(CHUNK_COUNT, chunkSettings_256);

  EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(CHUNK_COUNT));
  EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(CHUNK_COUNT));
  EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(CHUNK_COUNT));
  EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(CHUNK_COUNT));
}

TEST_F(MemoryManager_test, getChunkWithUserPayloadSizeZeroShouldNotFail) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "9fbfe1ff-9d59-449b-b164-433bbb031125");
  constexpr uint32_t USER_PAYLOAD_SIZE{0U};
  auto chunkSettingsResult = ChunkSettings::create(
      USER_PAYLOAD_SIZE, CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
  ASSERT_TRUE(chunkSettingsResult.has_value());
  auto& chunkSettings = chunkSettingsResult.value();

  mempoolconf.addMemPool({32, 10});
  sut->configureMemoryManager(mempoolconf, *allocator, *allocator);
  auto chunk = sut->getChunk(chunkSettings);

  if (chunk.has_value()) {
    EXPECT_TRUE(chunk);
  } else {
    GTEST_FAIL() << "getChunk failed with: " << asStringLiteral(chunk.error());
  }
}

// TEST_F(MemoryManager_test, addMemPoolWithChunkCountZeroShouldFail) {
//   ::testing::Test::RecordProperty("TEST_ID",
//                                   "be653b65-a2d1-42eb-98b5-d161c6ba7c08");
//   mempoolconf.addMemPool({32, 0});

//   IOX_EXPECT_FATAL_FAILURE<HoofsError>(
//       [&] { sut->configureMemoryManager(mempoolconf, *allocator, *allocator);
//       }, HoofsError::EXPECTS_ENSURES_FAILED);
// }

TEST(MemoryManagerEnumString_test, asStringLiteralConvertsEnumValuesToStrings) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "5f6c3942-0af5-4c48-b44c-7268191dbac5");
  using Error = MemoryManager::Error;

  // each bit corresponds to an enum value and must be set to true on test
  uint64_t testedEnumValues{0U};
  uint64_t loopCounter{0U};
  for (const auto& sut : {Error::NO_MEMPOOLS_AVAILABLE,
                          Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE,
                          Error::MEMPOOL_OUT_OF_CHUNKS}) {
    auto enumString = asStringLiteral(sut);

    switch (sut) {
      case Error::NO_MEMPOOLS_AVAILABLE:
        EXPECT_THAT(enumString,
                    StrEq("MemoryManager::Error::NO_MEMPOOLS_AVAILABLE"));
        break;
      case Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE:
        EXPECT_THAT(
            enumString,
            StrEq("MemoryManager::Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE"));
        break;
      case Error::MEMPOOL_OUT_OF_CHUNKS:
        EXPECT_THAT(enumString,
                    StrEq("MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS"));
        break;
      default:
        break;
    }

    testedEnumValues |= 1U << static_cast<uint64_t>(sut);
    ++loopCounter;
  }

  uint64_t expectedTestedEnumValues = (1U << loopCounter) - 1;
  EXPECT_EQ(testedEnumValues, expectedTestedEnumValues);
}

// TEST(MemoryManagerEnumString_test, LogStreamConvertsEnumValueToString) {
//   ::testing::Test::RecordProperty("TEST_ID",
//                                   "4a3539e5-5465-4352-b2b7-a850e104c173");

//   auto sut = MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS;

//   ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
//   EXPECT_THAT(loggerMock.logs[0].message, StrEq(asStringLiteral(sut)));
// }

}  // namespace