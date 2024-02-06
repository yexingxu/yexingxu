#include "memory/segment.hpp"
#include "test.hpp"

namespace {
using namespace ::testing;
using namespace shm::memory;
using namespace shm::details;

class MePooSegment_test : public Test {
 public:
  class SharedMemoryObject_MOCKBuilder;
  struct SharedMemoryObject_MOCK {
    using Builder = SharedMemoryObject_MOCKBuilder;
    using createFct = std::function<void(
        const SharedMemory::Name_t, const uint64_t, const shm::AccessMode,
        const shm::OpenMode, const void*, const shm::access_rights)>;
    SharedMemoryObject_MOCK(const SharedMemory::Name_t& name,
                            const uint64_t memorySizeInBytes,
                            const shm::AccessMode accessMode,
                            const shm::OpenMode openMode,
                            const void* baseAddressHint,
                            const shm::access_rights permissions)
        : m_memorySizeInBytes(memorySizeInBytes),
          m_baseAddressHint(const_cast<void*>(baseAddressHint)) {
      if (createVerificator) {
        createVerificator(name, memorySizeInBytes, accessMode, openMode,
                          baseAddressHint, permissions);
      }
      filehandle = creat("/tmp/roudi_segment_test", S_IRWXU);
    }

    ~SharedMemoryObject_MOCK() { remove("/tmp/roudi_segment_test"); }

    shm::shm_handle_t getFileHandle() { return filehandle; }

    tl::expected<uint64_t, shm::FileStatError> get_size() const {
      return m_memorySizeInBytes;
    }

    void* getBaseAddress() { return &memory[0]; }

    uint64_t m_memorySizeInBytes{0};
    void* m_baseAddressHint{nullptr};
    static constexpr int MEM_SIZE = 100000;
    char memory[MEM_SIZE];
    shm::shm_handle_t filehandle;
    static createFct createVerificator;
  };

  class SharedMemoryObject_MOCKBuilder {
   public:
    SharedMemory::Name_t m_name = "";

    uint64_t m_memorySizeInBytes = 0U;

    shm::AccessMode m_accessMode = shm::AccessMode::READ_ONLY;

    shm::OpenMode m_openMode = shm::OpenMode::OPEN_EXISTING;

    tl::optional<const void*> m_baseAddressHint = tl::nullopt;

    shm::access_rights m_permissions = shm::perms::none;

   public:
    explicit SharedMemoryObject_MOCKBuilder(const SharedMemory::Name_t& name,
                                            const uint64_t memorySize,
                                            const shm::AccessMode& am,
                                            const shm::OpenMode& om,
                                            const shm::access_rights& rights)
        : m_name(name),
          m_memorySizeInBytes(memorySize),
          m_accessMode(am),
          m_openMode(om),
          m_permissions(rights) {}

    tl::expected<SharedMemoryObject_MOCK, SharedMemoryObjectError>
    create() noexcept {
      return SharedMemoryObject_MOCK(
          m_name, m_memorySizeInBytes, m_accessMode, m_openMode,
          (m_baseAddressHint) ? *m_baseAddressHint : nullptr, m_permissions);
    }
  };

  Config setupMepooConfig() {
    Config config;
    config.addMemPool({128, 100});
    return config;
  }

  static constexpr uint64_t RawMemorySize{20000};
  uint8_t m_rawMemory[RawMemorySize];
  BumpAllocator m_managementAllocator{
      BumpAllocator(m_rawMemory, RawMemorySize)};

  Config mepooConfig = setupMepooConfig();

  using SUT = Segment<SharedMemoryObject_MOCK, MemoryManager>;
  std::unique_ptr<SUT> createSut() {
    return std::make_unique<SUT>(mepooConfig, m_managementAllocator,
                                 Group{"iox_roudi_test1"},
                                 Group{"iox_roudi_test2"});
  }
};
MePooSegment_test::SharedMemoryObject_MOCK::createFct
    MePooSegment_test::SharedMemoryObject_MOCK::createVerificator;

TEST_F(MePooSegment_test, SharedMemoryFileHandleRightsAfterConstructor) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "4719f767-3413-46cd-8d1e-92a3ca92760e");
  GTEST_SKIP() << "@todo iox-#611 Test needs to be written";
}

TEST_F(MePooSegment_test, SharedMemoryCreationParameter) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "0fcfefd4-3a84-43a5-9805-057a60239184");
  GTEST_SKIP_FOR_ADDITIONAL_USER()
      << "This test requires the -DTEST_WITH_ADDITIONAL_USER=ON cmake argument";

  MePooSegment_test::SharedMemoryObject_MOCK::createVerificator =
      [](const SharedMemory::Name_t f_name, const uint64_t,
         const shm::AccessMode f_accessMode, const shm::OpenMode openMode,
         const void*, const shm::access_rights) {
        EXPECT_THAT(f_name, Eq(SharedMemory::Name_t("iox_roudi_test2")));
        EXPECT_THAT(f_accessMode, Eq(shm::AccessMode::READ_WRITE));
        EXPECT_THAT(openMode, Eq(shm::OpenMode::PURGE_AND_CREATE));
      };
  SUT sut{mepooConfig, m_managementAllocator, Group{"iox_roudi_test1"},
          Group{"iox_roudi_test2"}};
  MePooSegment_test::SharedMemoryObject_MOCK::createVerificator =
      MePooSegment_test::SharedMemoryObject_MOCK::createFct();
}

TEST_F(MePooSegment_test, GetSegmentSize) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "0eee50c0-251e-4313-bb35-d83a0de27ce2");
  GTEST_SKIP_FOR_ADDITIONAL_USER()
      << "This test requires the -DTEST_WITH_ADDITIONAL_USER=ON cmake argument";

  auto sut = createSut();
  EXPECT_THAT(sut->getSegmentSize(),
              Eq(MemoryManager::requiredChunkMemorySize(mepooConfig)));
}

TEST_F(MePooSegment_test, GetReaderGroup) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "ad3fd360-3765-45ae-8285-fe4ae60c91ae");
  GTEST_SKIP_FOR_ADDITIONAL_USER()
      << "This test requires the -DTEST_WITH_ADDITIONAL_USER=ON cmake argument";

  auto sut = createSut();
  EXPECT_THAT(sut->getReaderGroup(), Eq(Group("iox_roudi_test1")));
}

TEST_F(MePooSegment_test, GetWriterGroup) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "3aa34489-bd46-4e77-89d6-20e82211e1a4");
  GTEST_SKIP_FOR_ADDITIONAL_USER()
      << "This test requires the -DTEST_WITH_ADDITIONAL_USER=ON cmake argument";

  auto sut = createSut();
  EXPECT_THAT(sut->getWriterGroup(), Eq(Group("iox_roudi_test2")));
}

TEST_F(MePooSegment_test, GetMemoryManager) {
  ::testing::Test::RecordProperty("TEST_ID",
                                  "4bc4af78-4beb-42eb-aee4-0f7cffb66411");
  GTEST_SKIP_FOR_ADDITIONAL_USER()
      << "This test requires the -DTEST_WITH_ADDITIONAL_USER=ON cmake argument";

  auto sut = createSut();
  ASSERT_THAT(sut->getMemoryManager().getNumberOfMemPools(), Eq(1U));
  auto config = sut->getMemoryManager().getMemPoolInfo(0);
  ASSERT_THAT(config.m_numChunks, Eq(100U));

  constexpr uint32_t USER_PAYLOAD_SIZE{128U};
  auto chunkSettingsResult = ChunkSettings::create(
      USER_PAYLOAD_SIZE, CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
  ASSERT_FALSE(!chunkSettingsResult.has_value());
  auto& chunkSettings = chunkSettingsResult.value();

  auto res = sut->getMemoryManager().getChunk(chunkSettings);

  if (res.has_value()) {
    EXPECT_THAT(res.value().getChunkHeader()->userPayloadSize(),
                Eq(USER_PAYLOAD_SIZE));
  } else {
    GTEST_FAIL() << "getChunk failed with: " << asStringLiteral(res.error());
  }
}

}  // namespace