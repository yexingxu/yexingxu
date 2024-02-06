
#include "mock/memory_block_mock.hpp"
#include "mock/memory_provider_mock.hpp"
#include "shm/relative_pointer.hpp"
#include "shm/shared_memory_provider.hpp"
#include "shm/system_call.hpp"
#include "test.hpp"
#include "types/expected.hpp"

namespace {
using namespace ::testing;

using namespace shm::details;

static const char* TEST_SHM_NAME = "FuManchu";

class SharedMemoryProvider_Test : public Test {
 public:
  void SetUp() override {
    /// @note just in the case a test left something behind we remove the shared
    /// memory if it exists
    DISCARD_RESULT(SharedMemory::unlinkIfExist(TEST_SHM_NAME));
  }

  void TearDown() override {}

  bool shmExists() {
    return SharedMemoryObjectBuilder(
               TEST_SHM_NAME, 8, shm::AccessMode::READ_ONLY,
               shm::OpenMode::OPEN_EXISTING, shm::perms::owner_all)
        .create()
        .has_value();
  }

  MemoryBlockMock memoryBlock1;
  MemoryBlockMock memoryBlock2;
};

TEST_F(SharedMemoryProvider_Test, CreateMemory) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "9808f2a5-4cd3-49fe-9a19-6e747183141d");
  SharedMemoryProvider sut(TEST_SHM_NAME, shm::AccessMode::READ_WRITE,
                           shm::OpenMode::PURGE_AND_CREATE);
  ASSERT_FALSE(!sut.addMemoryBlock(&memoryBlock1).has_value());
  uint64_t MEMORY_SIZE{16};
  uint64_t MEMORY_ALIGNMENT{8};
  EXPECT_CALL(memoryBlock1, size()).WillRepeatedly(Return(MEMORY_SIZE));
  EXPECT_CALL(memoryBlock1, alignment())
      .WillRepeatedly(Return(MEMORY_ALIGNMENT));

  EXPECT_THAT(!sut.create().has_value(), Eq(false));

  EXPECT_THAT(shmExists(), Eq(true));

  EXPECT_CALL(memoryBlock1, destroy());
}

TEST_F(SharedMemoryProvider_Test, DestroyMemory) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "f864b99c-373d-4954-ac8b-61acc3c9c555");
  SharedMemoryProvider sut(TEST_SHM_NAME, shm::AccessMode::READ_WRITE,
                           shm::OpenMode::PURGE_AND_CREATE);
  ASSERT_FALSE(!sut.addMemoryBlock(&memoryBlock1).has_value());
  uint64_t MEMORY_SIZE{16};
  uint64_t MEMORY_ALIGNMENT{8};
  EXPECT_CALL(memoryBlock1, size()).WillRepeatedly(Return(MEMORY_SIZE));
  EXPECT_CALL(memoryBlock1, alignment())
      .WillRepeatedly(Return(MEMORY_ALIGNMENT));

  ASSERT_FALSE(!sut.create().has_value());

  EXPECT_CALL(memoryBlock1, destroy());

  ASSERT_FALSE(!sut.destroy().has_value());

  EXPECT_THAT(shmExists(), Eq(false));
}

TEST_F(SharedMemoryProvider_Test,
       CreationFailedWithAlignmentExceedingPageSize) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "6614de7e-0f4c-48ea-bd3c-dd500fa231f2");
  SharedMemoryProvider sut(TEST_SHM_NAME, shm::AccessMode::READ_WRITE,
                           shm::OpenMode::PURGE_AND_CREATE);
  ASSERT_FALSE(!sut.addMemoryBlock(&memoryBlock1).has_value());
  uint64_t MEMORY_SIZE{16};
  uint64_t MEMORY_ALIGNMENT{pageSize() + 8U};
  EXPECT_CALL(memoryBlock1, size()).WillRepeatedly(Return(MEMORY_SIZE));
  EXPECT_CALL(memoryBlock1, alignment())
      .WillRepeatedly(Return(MEMORY_ALIGNMENT));

  auto expectFailed = sut.create();
  ASSERT_THAT(!expectFailed.has_value(), Eq(true));
  ASSERT_THAT(expectFailed.error(),
              Eq(MemoryProviderError::MEMORY_ALIGNMENT_EXCEEDS_PAGE_SIZE));

  EXPECT_THAT(shmExists(), Eq(false));
}

}  // namespace