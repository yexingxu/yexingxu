

#include <gtest/gtest.h>
#include <sys/mman.h>
#include <unistd.h>

#include "shm/file_access.hpp"
#include "shm/shared_memory.hpp"
#include "test.hpp"

namespace shm_test {

using namespace testing;
using namespace shm::details;

class SharedMemory_Test : public Test {
 public:
  void SetUp() override {
    auto result = SharedMemory::unlinkIfExist(SUT_SHM_NAME);
    ASSERT_FALSE(!result.has_value());
  }

  void TearDown() override {}

  // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays) test
  // only
  static constexpr const char SUT_SHM_NAME[] = "ignatz";

  static tl::expected<SharedMemory, SharedMemoryError> createSut(
      const SharedMemory::Name_t& name, const shm::OpenMode openMode) {
    return SharedMemoryBuilder(name, shm::AccessMode::READ_WRITE, openMode,
                               shm::perms::owner_all, 128)
        .create();
  }

  static std::unique_ptr<int, std::function<void(int*)>> createRawSharedMemory(
      const SharedMemory::Name_t& name) {
    // NOLINTBEGIN(hicpp-signed-bitwise) enum types defined by POSIX are
    // required

    auto open_res =
        shm_open((std::string("/") + name.c_str()).c_str(), O_RDWR | O_CREAT,
                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (open_res == SharedMemory::INVALID_HANDLE) {
      return std::unique_ptr<int, std::function<void(int*)>>();
    }
    return std::unique_ptr<int, std::function<void(int*)>>(
        new int(open_res), [=](const int* fd) {
          cleanupSharedMemory(name);
          close(*fd);
          delete fd;
        });
  }

  static bool cleanupSharedMemory(const SharedMemory::Name_t& name) {
    auto result = SharedMemory::unlinkIfExist(name);
    if (!result.has_value()) {
      EXPECT_TRUE(false);
      return false;
    }
    return *result;
  }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays) test
// only
constexpr const char SharedMemory_Test::SUT_SHM_NAME[];

TEST_F(SharedMemory_Test, CTorWithValidArguments) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "158f1ee6-cc8c-4e80-a288-6e23a74cd66e");
  auto sut = createSut(SUT_SHM_NAME, shm::OpenMode::PURGE_AND_CREATE);
  EXPECT_THAT(!sut.has_value(), Eq(false));
}

TEST_F(SharedMemory_Test, CTorWithInvalidMessageQueueNames) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "76ed82b1-eef7-4a5a-8794-b333c679e726");
  EXPECT_THAT(!createSut("", shm::OpenMode::PURGE_AND_CREATE).has_value(),
              Eq(true));
  EXPECT_THAT(
      !createSut("/ignatz", shm::OpenMode::PURGE_AND_CREATE).has_value(),
      Eq(true));
}

TEST_F(SharedMemory_Test, CTorWithInvalidArguments) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "53c66249-4f3a-4220-9cc0-001be53546d3");
  auto sut = createSut("/schlomo", shm::OpenMode::OPEN_EXISTING);
  EXPECT_THAT(!sut.has_value(), Eq(true));
}

TEST_F(SharedMemory_Test, MoveCTorWithValidValues) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "2844c9c5-856e-4b51-890d-1418f79f1a80");

  auto sut = createSut(SUT_SHM_NAME, shm::OpenMode::PURGE_AND_CREATE);
  ASSERT_FALSE(!sut.has_value());
  int handle = sut->getHandle();
  {
    SharedMemory sut2(std::move(*sut));
    EXPECT_THAT(handle, Eq(sut2.getHandle()));
  }
}

TEST_F(SharedMemory_Test, getHandleOfValidObject) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "1fec2518-70f7-412b-8be2-3174e6ada050");
  auto sut = createSut(SUT_SHM_NAME, shm::OpenMode::PURGE_AND_CREATE);
  ASSERT_FALSE(!sut.has_value());
  EXPECT_THAT(sut->getHandle(), Ne(SharedMemory::INVALID_HANDLE));
}

TEST_F(SharedMemory_Test, UnlinkNonExistingShmFails) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "56951e9d-5dd4-4a44-aa5b-bd369cb963e9");
  auto result = SharedMemory::unlinkIfExist(
      "/look_there's_a_dead_seagull_flying_its_name_is_dietlbart");
  ASSERT_FALSE(!result.has_value());
  EXPECT_FALSE(*result);
}

TEST_F(SharedMemory_Test, UnlinkExistingShmWorks) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "11f0b2f2-b891-41e4-bb82-648a9541582f");
  // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays) test
  // only
  constexpr const char SHM_NAME[] = "its_a_mee_monukulius";
  auto rawSharedMemory = createRawSharedMemory(SHM_NAME);
  ASSERT_TRUE(static_cast<bool>(rawSharedMemory));
  auto result = SharedMemory::unlinkIfExist(SHM_NAME);
  ASSERT_FALSE(!result.has_value());
  EXPECT_TRUE(*result);

  // delete the underyling fd pointer but do not cleanup raw shared memory since
  // is is already deleted with unlinkIfExist in the test
  delete rawSharedMemory.release();
}

TEST_F(SharedMemory_Test, ExclusiveCreateWorksWhenShmDoesNotExist) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "bfc44656-ef23-49ef-be96-0d4bfb592030");
  auto sut = createSut(SUT_SHM_NAME, shm::OpenMode::EXCLUSIVE_CREATE);
  ASSERT_FALSE(!sut.has_value());
  EXPECT_TRUE(sut->hasOwnership());
  EXPECT_THAT(sut->getHandle(), Ne(SharedMemory::INVALID_HANDLE));
}

TEST_F(SharedMemory_Test, ExclusiveCreateFailsWhenShmExists) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "19eca662-4f01-453b-9ae3-5cb2090e46ce");
  auto rawSharedMemory = createRawSharedMemory(SUT_SHM_NAME);
  ASSERT_TRUE(static_cast<bool>(rawSharedMemory));

  auto sut = createSut(SUT_SHM_NAME, shm::OpenMode::EXCLUSIVE_CREATE);
  ASSERT_TRUE(!sut.has_value());
}

TEST_F(SharedMemory_Test, PurgeAndCreateWorksWhenShmDoesNotExist) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "611694b6-d877-43a1-a6e3-dfef3f8a174b");
  auto sut = createSut(SUT_SHM_NAME, shm::OpenMode::PURGE_AND_CREATE);
  ASSERT_FALSE(!sut.has_value());
  EXPECT_TRUE(sut->hasOwnership());
  EXPECT_THAT(sut->getHandle(), Ne(SharedMemory::INVALID_HANDLE));
}

// Windows does not support this since the named semaphore is automatically
// deleted as soon as the last handle was closed with CloseHandle
#if !defined(_WIN32)
TEST_F(SharedMemory_Test, PurgeAndCreateWorksWhenShmExists) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "21d620f0-af45-46ad-a5b7-1c18026fb9a8");
  auto rawSharedMemory = createRawSharedMemory(SUT_SHM_NAME);
  ASSERT_TRUE(static_cast<bool>(rawSharedMemory));

  auto sut = createSut(SUT_SHM_NAME, shm::OpenMode::PURGE_AND_CREATE);
  ASSERT_FALSE(!sut.has_value());
  EXPECT_TRUE(sut->hasOwnership());
  EXPECT_THAT(sut->getHandle(), Ne(SharedMemory::INVALID_HANDLE));
}
#endif

TEST_F(SharedMemory_Test, CreateOrOpenCreatesShmWhenShmDoesNotExist) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "574b0b16-2458-49b0-8b37-3999e9b56072");
  {
    auto sut = createSut(SUT_SHM_NAME, shm::OpenMode::OPEN_OR_CREATE);
    ASSERT_FALSE(!sut.has_value());
    EXPECT_TRUE(sut->hasOwnership());
    EXPECT_THAT(sut->getHandle(), Ne(SharedMemory::INVALID_HANDLE));
  }
}

TEST_F(SharedMemory_Test, CreateOrOpenOpensShmWhenShmDoesExist) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "2413ddda-9d2e-4429-adba-81fe848a6a06");
  auto rawSharedMemory = createRawSharedMemory(SUT_SHM_NAME);
  ASSERT_TRUE(static_cast<bool>(rawSharedMemory));

  {
    auto sut = createSut(SUT_SHM_NAME, shm::OpenMode::OPEN_OR_CREATE);
    ASSERT_TRUE(sut.has_value());
    EXPECT_FALSE(sut->hasOwnership());
    EXPECT_THAT(sut->getHandle(), Ne(SharedMemory::INVALID_HANDLE));
  }
}

TEST_F(SharedMemory_Test, OpenWorksWhenShmExist) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "59ba1e1c-ec1c-45fb-bc85-c6256f9176fd");
  auto rawSharedMemory = createRawSharedMemory(SUT_SHM_NAME);
  ASSERT_TRUE(static_cast<bool>(rawSharedMemory));
  {
    auto sut = createSut(SUT_SHM_NAME, shm::OpenMode::OPEN_EXISTING);
    ASSERT_FALSE(!sut.has_value());
    EXPECT_FALSE(sut->hasOwnership());
    EXPECT_THAT(sut->getHandle(), Ne(SharedMemory::INVALID_HANDLE));
  }
}

TEST_F(SharedMemory_Test, OpenFailsWhenShmDoesNotExist) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "5b1878b9-d292-479c-bfe7-9826561152ee");
  auto sut = createSut(SUT_SHM_NAME, shm::OpenMode::OPEN_EXISTING);
  ASSERT_TRUE(!sut.has_value());
}

TEST_F(SharedMemory_Test, OpenFailsWhenCreatingShmInReadOnlyMode) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "80684160-b243-4ca1-b285-118d2ef36108");
  auto sut = SharedMemoryBuilder("readOnlyShmMem", shm::AccessMode::READ_ONLY,
                                 shm::OpenMode::PURGE_AND_CREATE,
                                 shm::perms::none, 100)
                 .create();

  ASSERT_TRUE(!sut.has_value());
  ASSERT_THAT(sut.error(),
              Eq(SharedMemoryError::INCOMPATIBLE_OPEN_AND_ACCESS_MODE));
}
}  // namespace shm_test