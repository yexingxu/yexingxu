

#include "shm/group.hpp"
#include "shm/shared_memory_object.hpp"
#include "shm/user.hpp"
#include "test.hpp"

namespace shm_test {

using namespace testing;
using namespace shm::details;

class SharedMemoryObject_Test : public Test {
 public:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(SharedMemoryObject_Test, CTorWithValidArguments) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "bbda60d2-d741-407e-9a9f-f0ca74d985a8");
  auto sut = SharedMemoryObjectBuilder(
                 "validShmMem", 100, shm::AccessMode::READ_WRITE,
                 shm::OpenMode::PURGE_AND_CREATE, shm::perms::none)
                 .create();
  sleep(100);
  EXPECT_THAT(!sut.has_value(), Eq(false));
}

TEST_F(SharedMemoryObject_Test, CTorOpenNonExistingSharedMemoryObject) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "d80278c3-1dd8-409d-9162-f7f900892526");
  auto sut =
      SharedMemoryObjectBuilder("pummeluff", 100, shm::AccessMode::READ_WRITE,
                                shm::OpenMode::OPEN_EXISTING, shm::perms::none)
          .create();

  EXPECT_THAT(!sut.has_value(), Eq(true));
}

TEST_F(SharedMemoryObject_Test, AllocateMemoryInSharedMemoryAndReadIt) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "6169ac70-a08e-4a19-80e4-57f0d5f89233");
  const uint64_t MEMORY_SIZE = 16;
  auto sut = SharedMemoryObjectBuilder(
                 "shmAllocate", MEMORY_SIZE, shm::AccessMode::READ_WRITE,
                 shm::OpenMode::PURGE_AND_CREATE, shm::perms::owner_all)
                 .create();

  auto* data_ptr = static_cast<uint8_t*>(sut->getBaseAddress());

  for (uint64_t i = 0; i < MEMORY_SIZE; ++i) {
    /// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    data_ptr[i] = static_cast<uint8_t>(i * 2 + 1);
  }

  auto sut2 = SharedMemoryObjectBuilder(
                  "shmAllocate", MEMORY_SIZE, shm::AccessMode::READ_ONLY,
                  shm::OpenMode::OPEN_EXISTING, shm::perms::none)
                  .create();

  auto* data_ptr2 = static_cast<uint8_t*>(sut2->getBaseAddress());

  for (uint64_t i = 0; i < MEMORY_SIZE; ++i) {
    /// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    EXPECT_THAT(data_ptr2[i], Eq(static_cast<uint8_t>(i) * 2 + 1));
  }
}

TEST_F(SharedMemoryObject_Test,
       OpenFailsWhenActualMemorySizeIsSmallerThanRequestedSize) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "bb58b45e-8366-42ae-bd30-8d7415791dd4");
  const uint64_t MEMORY_SIZE = 16U << 20U;  // 16 MB
  auto sut = SharedMemoryObjectBuilder(
                 "shmAllocate", 1, shm::AccessMode::READ_WRITE,
                 shm::OpenMode::PURGE_AND_CREATE, shm::perms::owner_all)
                 .create();

  auto sut2 = SharedMemoryObjectBuilder(
                  "shmAllocate", MEMORY_SIZE, shm::AccessMode::READ_ONLY,
                  shm::OpenMode::OPEN_EXISTING, shm::perms::none)
                  .create();

  ASSERT_TRUE(!sut2.has_value());
  EXPECT_THAT(sut2.error(),
              Eq(SharedMemoryObjectError::REQUESTED_SIZE_EXCEEDS_ACTUAL_SIZE));
}

TEST_F(SharedMemoryObject_Test, OpenSutMapsAllMemoryIntoProcess) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "0c8b41eb-74fd-4796-9e5e-fe6707f3c46c");
  const uint64_t MEMORY_SIZE = 1024;
  auto sut = SharedMemoryObjectBuilder(
                 "shmAllocate", MEMORY_SIZE * sizeof(uint64_t),
                 shm::AccessMode::READ_WRITE, shm::OpenMode::PURGE_AND_CREATE,
                 shm::perms::owner_all)
                 .create();

  auto* data_ptr = static_cast<uint64_t*>(sut->getBaseAddress());

  for (uint64_t i = 0; i < MEMORY_SIZE; ++i) {
    /// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    data_ptr[i] = i * 2 + 1;
  }

  auto sut2 =
      SharedMemoryObjectBuilder("shmAllocate", 1, shm::AccessMode::READ_ONLY,
                                shm::OpenMode::OPEN_EXISTING, shm::perms::none)
          .create();

  ASSERT_THAT(*sut2->get_size(), Ge(MEMORY_SIZE * sizeof(uint64_t)));

  auto* data_ptr2 = static_cast<uint64_t*>(sut2->getBaseAddress());

  for (uint64_t i = 0; i < MEMORY_SIZE; ++i) {
    /// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    EXPECT_THAT(data_ptr2[i], Eq(i * 2 + 1));
  }
}

TEST_F(SharedMemoryObject_Test, AcquiringOwnerWorks) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "a9859b5e-555b-4cff-b418-74168a9fd85a");
  auto sut = SharedMemoryObjectBuilder(
                 "shmAllocate", 8, shm::AccessMode::READ_WRITE,
                 shm::OpenMode::PURGE_AND_CREATE, shm::perms::owner_all)
                 .create();

  auto owner = sut->get_ownership();
  ASSERT_FALSE(!owner.has_value());

  EXPECT_THAT(owner->uid(), User::getUserOfCurrentProcess().getID());
  EXPECT_THAT(owner->gid(), Group::getGroupOfCurrentProcess().getID());
}

TEST_F(SharedMemoryObject_Test, AcquiringPermissionsWorks) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "2b36bc3b-16a0-4c18-a1cb-6815812c6616");
  const auto permissions = shm::perms::owner_all | shm::perms::group_write |
                           shm::perms::group_read | shm::perms::others_exec;
  auto sut =
      SharedMemoryObjectBuilder("shmAllocate", 8, shm::AccessMode::READ_WRITE,
                                shm::OpenMode::PURGE_AND_CREATE, permissions)
          .create();

  auto sut_perm = sut->get_permissions();
  ASSERT_FALSE(!sut_perm.has_value());
  EXPECT_THAT(*sut_perm, Eq(permissions));
}

TEST_F(SharedMemoryObject_Test, SettingOwnerWorks) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "da85be28-7e21-4207-9077-698a2ec188d6");
  auto sut = SharedMemoryObjectBuilder(
                 "shmAllocate", 8, shm::AccessMode::READ_WRITE,
                 shm::OpenMode::PURGE_AND_CREATE, shm::perms::owner_all)
                 .create();

  auto owner = sut->get_ownership();
  ASSERT_FALSE(!owner.has_value());

  // It is a slight stub test since we must be root to change the owner of a
  // file. But changing the owner from self to self is legal.
  ASSERT_FALSE(!sut->set_ownership(*owner).has_value());
}

TEST_F(SharedMemoryObject_Test, SettingPermissionsWorks) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "412abc8a-d1f8-4ceb-86db-f2790d2da58f");
  auto sut = SharedMemoryObjectBuilder(
                 "shmAllocate", 8, shm::AccessMode::READ_WRITE,
                 shm::OpenMode::PURGE_AND_CREATE, shm::perms::owner_all)
                 .create();

  ASSERT_FALSE(!sut->set_permissions(shm::perms::none).has_value());
  auto result = sut->get_permissions();
  ASSERT_FALSE(!result.has_value());
  EXPECT_THAT(*result, Eq(shm::perms::none));
}

}  // namespace shm_test