//
// Copyright 2022 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "absl/log/log_streamer.h"

#include <iostream>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/base/attributes.h"
#include "absl/base/internal/sysinfo.h"
#include "absl/base/log_severity.h"
#include "absl/log/internal/test_actions.h"
#include "absl/log/internal/test_helpers.h"
#include "absl/log/internal/test_matchers.h"
#include "absl/log/log.h"
#include "absl/log/scoped_mock_log.h"
#include "absl/strings/string_view.h"

namespace {
using ::absl::log_internal::DeathTestExpectedLogging;
using ::absl::log_internal::DeathTestUnexpectedLogging;
using ::absl::log_internal::DeathTestValidateExpectations;
#if GTEST_HAS_DEATH_TEST
using ::absl::log_internal::DiedOfFatal;
#endif
using ::absl::log_internal::LogSeverity;
using ::absl::log_internal::Prefix;
using ::absl::log_internal::SourceFilename;
using ::absl::log_internal::SourceLine;
using ::absl::log_internal::Stacktrace;
using ::absl::log_internal::TextMessage;
using ::absl::log_internal::ThreadID;
using ::absl::log_internal::TimestampInMatchWindow;
using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::IsTrue;

auto* test_env ABSL_ATTRIBUTE_UNUSED = ::testing::AddGlobalTestEnvironment(
    new absl::log_internal::LogTestEnvironment);

void WriteToStream(absl::string_view data, std::ostream* os) {
  *os << "WriteToStream: " << data;
}
void WriteToStreamRef(absl::string_view data, std::ostream& os) {
  os << "WriteToStreamRef: " << data;
}

TEST(LogStreamerTest, LogInfoStreamer) {
  absl::ScopedMockLog test_sink(absl::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(
      test_sink,
      Send(AllOf(SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                 Prefix(IsTrue()), LogSeverity(Eq(absl::LogSeverity::kInfo)),
                 TimestampInMatchWindow(),
                 ThreadID(Eq(absl::base_internal::GetTID())),
                 TextMessage(Eq("WriteToStream: foo")),
                 ENCODED_MESSAGE(EqualsProto(R"pb(value {
                                                    str: "WriteToStream: foo"
                                                  })pb")),
                 Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  WriteToStream("foo", &absl::LogInfoStreamer("path/file.cc", 1234).stream());
}

TEST(LogStreamerTest, LogWarningStreamer) {
  absl::ScopedMockLog test_sink(absl::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(
      test_sink,
      Send(AllOf(SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                 Prefix(IsTrue()), LogSeverity(Eq(absl::LogSeverity::kWarning)),
                 TimestampInMatchWindow(),
                 ThreadID(Eq(absl::base_internal::GetTID())),
                 TextMessage(Eq("WriteToStream: foo")),
                 ENCODED_MESSAGE(EqualsProto(R"pb(value {
                                                    str: "WriteToStream: foo"
                                                  })pb")),
                 Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  WriteToStream("foo",
                &absl::LogWarningStreamer("path/file.cc", 1234).stream());
}

TEST(LogStreamerTest, LogErrorStreamer) {
  absl::ScopedMockLog test_sink(absl::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(
      test_sink,
      Send(AllOf(SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                 Prefix(IsTrue()), LogSeverity(Eq(absl::LogSeverity::kError)),
                 TimestampInMatchWindow(),
                 ThreadID(Eq(absl::base_internal::GetTID())),
                 TextMessage(Eq("WriteToStream: foo")),
                 ENCODED_MESSAGE(EqualsProto(R"pb(value {
                                                    str: "WriteToStream: foo"
                                                  })pb")),
                 Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  WriteToStream("foo", &absl::LogErrorStreamer("path/file.cc", 1234).stream());
}

#if GTEST_HAS_DEATH_TEST
TEST(LogStreamerDeathTest, LogFatalStreamer) {
  EXPECT_EXIT(
      {
        absl::ScopedMockLog test_sink;

        EXPECT_CALL(test_sink, Send)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());

        EXPECT_CALL(
            test_sink,
            Send(AllOf(
                SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                Prefix(IsTrue()), LogSeverity(Eq(absl::LogSeverity::kFatal)),
                TimestampInMatchWindow(),
                ThreadID(Eq(absl::base_internal::GetTID())),
                TextMessage(Eq("WriteToStream: foo")),
                ENCODED_MESSAGE(EqualsProto(R"pb(value {
                                                   str: "WriteToStream: foo"
                                                 })pb")))))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        WriteToStream("foo",
                      &absl::LogFatalStreamer("path/file.cc", 1234).stream());
      },
      DiedOfFatal, DeathTestValidateExpectations());
}
#endif

TEST(LogStreamerTest, LogStreamer) {
  absl::ScopedMockLog test_sink(absl::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(
      test_sink,
      Send(AllOf(SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                 Prefix(IsTrue()), LogSeverity(Eq(absl::LogSeverity::kError)),
                 TimestampInMatchWindow(),
                 ThreadID(Eq(absl::base_internal::GetTID())),
                 TextMessage(Eq("WriteToStream: foo")),
                 ENCODED_MESSAGE(EqualsProto(R"pb(value {
                                                    str: "WriteToStream: foo"
                                                  })pb")),
                 Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  WriteToStream(
      "foo", &absl::LogStreamer(absl::LogSeverity::kError, "path/file.cc", 1234)
                  .stream());
}

#if GTEST_HAS_DEATH_TEST
TEST(LogStreamerDeathTest, LogStreamer) {
  EXPECT_EXIT(
      {
        absl::ScopedMockLog test_sink;

        EXPECT_CALL(test_sink, Send)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());

        EXPECT_CALL(
            test_sink,
            Send(AllOf(
                SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                Prefix(IsTrue()), LogSeverity(Eq(absl::LogSeverity::kFatal)),
                TimestampInMatchWindow(),
                ThreadID(Eq(absl::base_internal::GetTID())),
                TextMessage(Eq("WriteToStream: foo")),
                ENCODED_MESSAGE(EqualsProto(R"pb(value {
                                                   str: "WriteToStream: foo"
                                                 })pb")))))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        WriteToStream("foo", &absl::LogStreamer(absl::LogSeverity::kFatal,
                                                "path/file.cc", 1234)
                                  .stream());
      },
      DiedOfFatal, DeathTestValidateExpectations());
}
#endif

TEST(LogStreamerTest, PassedByReference) {
  absl::ScopedMockLog test_sink(absl::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(
      test_sink,
      Send(AllOf(SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                 TextMessage(Eq("WriteToStreamRef: foo")),
                 ENCODED_MESSAGE(EqualsProto(R"pb(value {
                                                    str: "WriteToStreamRef: foo"
                                                  })pb")),
                 Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  WriteToStreamRef("foo", absl::LogInfoStreamer("path/file.cc", 1234).stream());
}

TEST(LogStreamerTest, StoredAsLocal) {
  absl::ScopedMockLog test_sink(absl::MockLogDefault::kDisallowUnexpected);

  auto streamer = absl::LogInfoStreamer("path/file.cc", 1234);
  WriteToStream("foo", &streamer.stream());
  streamer.stream() << " ";
  WriteToStreamRef("bar", streamer.stream());

  // The call should happen when `streamer` goes out of scope; if it
  // happened before this `EXPECT_CALL` the call would be unexpected and the
  // test would fail.
  EXPECT_CALL(
      test_sink,
      Send(AllOf(SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                 TextMessage(Eq("WriteToStream: foo WriteToStreamRef: bar")),
                 ENCODED_MESSAGE(EqualsProto(
                     R"pb(value {
                            str: "WriteToStream: foo WriteToStreamRef: bar"
                          })pb")),
                 Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
}

#if GTEST_HAS_DEATH_TEST
TEST(LogStreamerDeathTest, StoredAsLocal) {
  EXPECT_EXIT(
      {
        // This is fatal when it goes out of scope, but not until then:
        auto streamer = absl::LogFatalStreamer("path/file.cc", 1234);
        std::cerr << "I'm still alive" << std::endl;
        WriteToStream("foo", &streamer.stream());
      },
      DiedOfFatal, HasSubstr("I'm still alive"));
}
#endif

TEST(LogStreamerTest, LogsEmptyLine) {
  absl::ScopedMockLog test_sink(absl::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(test_sink, Send(AllOf(SourceFilename(Eq("path/file.cc")),
                                    SourceLine(Eq(1234)), TextMessage(Eq("")),
                                    ENCODED_MESSAGE(EqualsProto(R"pb(value {
                                                                       str: ""
                                                                     })pb")),
                                    Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  absl::LogInfoStreamer("path/file.cc", 1234);
}

#if GTEST_HAS_DEATH_TEST
TEST(LogStreamerDeathTest, LogsEmptyLine) {
  EXPECT_EXIT(
      {
        absl::ScopedMockLog test_sink;

        EXPECT_CALL(test_sink, Log)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());

        EXPECT_CALL(
            test_sink,
            Send(AllOf(
                SourceFilename(Eq("path/file.cc")), TextMessage(Eq("")),
                ENCODED_MESSAGE(EqualsProto(R"pb(value { str: "" })pb")))))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        // This is fatal even though it's never used:
        auto streamer = absl::LogFatalStreamer("path/file.cc", 1234);
      },
      DiedOfFatal, DeathTestValidateExpectations());
}
#endif

TEST(LogStreamerTest, MoveConstruction) {
  absl::ScopedMockLog test_sink(absl::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(
      test_sink,
      Send(AllOf(
          SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
          LogSeverity(Eq(absl::LogSeverity::kInfo)),
          TextMessage(Eq("hello world")),
          ENCODED_MESSAGE(EqualsProto(R"pb(value { str: "hello world" })pb")),
          Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  auto streamer1 = absl::LogInfoStreamer("path/file.cc", 1234);
  streamer1.stream() << "hello";
  absl::LogStreamer streamer2(std::move(streamer1));
  streamer2.stream() << " world";
}

TEST(LogStreamerTest, MoveAssignment) {
  absl::ScopedMockLog test_sink(absl::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(
      test_sink,
      Send(AllOf(SourceFilename(Eq("path/file.cc")), SourceLine(Eq(1234)),
                 LogSeverity(Eq(absl::LogSeverity::kInfo)),
                 TextMessage(Eq("hello")),
                 ENCODED_MESSAGE(EqualsProto(R"pb(value { str: "hello" })pb")),
                 Stacktrace(IsEmpty()))));

  EXPECT_CALL(
      test_sink,
      Send(AllOf(SourceFilename(Eq("elsewhere/name.cc")), SourceLine(Eq(5678)),
                 LogSeverity(Eq(absl::LogSeverity::kWarning)),
                 TextMessage(Eq("world; goodbye")),
                 ENCODED_MESSAGE(EqualsProto(R"pb(value {
                                                    str: "world; goodbye"
                                                  })pb")),
                 Stacktrace(IsEmpty()))));

  test_sink.StartCapturingLogs();
  auto streamer1 = absl::LogInfoStreamer("path/file.cc", 1234);
  streamer1.stream() << "hello";
  auto streamer2 = absl::LogWarningStreamer("elsewhere/name.cc", 5678);
  streamer2.stream() << "world";
  streamer1 = std::move(streamer2);
  streamer1.stream() << "; goodbye";
}

TEST(LogStreamerTest, CorrectDefaultFlags) {
  absl::ScopedMockLog test_sink(absl::MockLogDefault::kDisallowUnexpected);

  // The `boolalpha` and `showbase` flags should be set by default, to match
  // `LOG`.
  EXPECT_CALL(test_sink, Send(AllOf(TextMessage(Eq("false0xdeadbeef")))))
      .Times(2);

  test_sink.StartCapturingLogs();
  absl::LogInfoStreamer("path/file.cc", 1234).stream()
      << false << std::hex << 0xdeadbeef;
  LOG(INFO) << false << std::hex << 0xdeadbeef;
}

}  // namespace
