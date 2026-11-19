#include "mesh/mesh_storage.h"

#include <SD.h>
#include <SPI.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "core/at_link.h"
#include "core/mesh_history.h"
#include "log.h"

namespace mesh_storage {

namespace {

// The microSD slot of the Cardputer v1.1 (M5Cardputer examples/Basic/sdcard).
constexpr int kSckPin = 40;
constexpr int kMisoPin = 39;
constexpr int kMosiPin = 14;
constexpr int kCsPin = 12;
constexpr uint32_t kSpiHz = 25000000;

const char* const kParent = "/Tasca";
const char* const kFolder = "/Tasca/Mesh";
const char* const kMessagesPath = "/Tasca/Mesh/messages.txt";
const char* const kMessagesNewPath = "/Tasca/Mesh/messages.new";
const char* const kNodesPath = "/Tasca/Mesh/nodes.txt";

constexpr size_t kKeptMessages = 500;
// The message file is rewritten with the last 500 once it holds this many more lines: one rewrite per 50 messages.
constexpr size_t kRewriteAfter = 50;
constexpr uint32_t kNodesEveryMs = 60000;
// A message line: 200 bytes of text, each escaped at most to 2, and the fields before it.
constexpr size_t kMaxLine = 512;

bool saving_ = false;
// Lines in the message file: messages and marks.
size_t fileLines = 0;
size_t savedAdded = 0;
uint32_t savedNodes = 0;
uint32_t nodesSavedMs = 0;

void fail(const char* what) {
    logMessage(LogLevel::Warn, "MESH", "history not saved: %s", what);
    saving_ = false;
}

// Applies every whole line of path to store; a last line cut by a restart is skipped. Returns the lines applied.
size_t load(const char* path, MeshStore& store) {
    File file = SD.open(path, FILE_READ);
    if (!file) {
        return 0;
    }
    LineSplitter splitter(kMaxLine);
    std::string line;
    size_t lines = 0;
    uint8_t buffer[256];
    while (const int n = file.read(buffer, sizeof(buffer))) {
        if (n < 0) {
            break;
        }
        for (int i = 0; i < n; i++) {
            if (splitter.add(static_cast<char>(buffer[i]), line) && loadHistoryLine(line, store)) {
                lines++;
            }
        }
    }
    file.close();
    return lines;
}

bool write(File& file, const std::string& text) {
    return file.write(reinterpret_cast<const uint8_t*>(text.data()), text.size()) == text.size();
}

void rewriteMessages(const MeshStore& store) {
    File file = SD.open(kMessagesNewPath, FILE_WRITE);
    if (!file) {
        fail("cannot write the messages");
        return;
    }
    bool ok = true;
    for (const MeshMessage& m : store.messages()) {
        ok = ok && write(file, historyMessageLine(m));
    }
    file.close();
    if (!ok) {
        fail("card full");
        return;
    }
    SD.remove(kMessagesPath);
    if (!SD.rename(kMessagesNewPath, kMessagesPath)) {
        fail("cannot replace the messages");
        return;
    }
    fileLines = store.messages().size();
}

void appendMessages(MeshStore& store) {
    const size_t fresh = std::min(store.added() - savedAdded, store.messages().size());
    const std::vector<std::pair<uint32_t, Delivery>> marks = store.takeDeliveryChanges();
    savedAdded = store.added();
    if (fresh == 0 && marks.empty()) {
        return;
    }
    File file = SD.open(kMessagesPath, FILE_APPEND);
    if (!file) {
        fail("cannot open the messages");
        return;
    }
    bool ok = true;
    for (size_t i = store.messages().size() - fresh; i < store.messages().size(); i++) {
        ok = ok && write(file, historyMessageLine(store.messages()[i]));
    }
    for (const auto& mark : marks) {
        ok = ok && write(file, historyDeliveryLine(mark.first, mark.second));
    }
    file.close();
    if (!ok) {
        fail("card full");
        return;
    }
    fileLines += fresh + marks.size();
    if (fileLines >= kKeptMessages + kRewriteAfter) {
        rewriteMessages(store);
    }
}

void saveNodes(const MeshStore& store, uint32_t nowMs) {
    if (store.nodesChanged() == savedNodes || nowMs - nodesSavedMs < kNodesEveryMs) {
        return;
    }
    File file = SD.open(kNodesPath, FILE_WRITE);
    if (!file) {
        fail("cannot write the nodes");
        return;
    }
    bool ok = true;
    for (const MeshNode& node : store.nodes()) {
        ok = ok && write(file, historyNodeLine(node));
    }
    file.close();
    if (!ok) {
        fail("card full");
        return;
    }
    savedNodes = store.nodesChanged();
    nodesSavedMs = nowMs;
}

}  // namespace

void begin(MeshStore& store) {
    SPI.begin(kSckPin, kMisoPin, kMosiPin, kCsPin);
    if (!SD.begin(kCsPin, SPI, kSpiHz)) {
        logMessage(LogLevel::Info, "MESH", "no microSD card: history kept until restart");
        return;
    }
    if ((!SD.exists(kParent) && !SD.mkdir(kParent)) || (!SD.exists(kFolder) && !SD.mkdir(kFolder))) {
        fail("cannot create the folder");
        return;
    }
    saving_ = true;
    fileLines = load(kMessagesPath, store);
    const size_t nodes = load(kNodesPath, store);
    savedAdded = store.added();
    savedNodes = store.nodesChanged();
    logMessage(LogLevel::Info, "MESH", "history loaded: %u messages, %u nodes",
               static_cast<unsigned>(store.messages().size()), static_cast<unsigned>(nodes));
}

void save(MeshStore& store, uint32_t nowMs) {
    if (!saving_) {
        store.takeDeliveryChanges();
        return;
    }
    appendMessages(store);
    if (saving_) {
        saveNodes(store, nowMs);
    }
}

void clearMessages(MeshStore& store) {
    store.clearMessages();
    store.takeDeliveryChanges();
    savedAdded = store.added();
    if (!saving_) {
        return;
    }
    File file = SD.open(kMessagesPath, FILE_WRITE);
    if (!file) {
        fail("cannot clear the messages");
        return;
    }
    file.close();
    fileLines = 0;
    logMessage(LogLevel::Info, "MESH", "messages cleared");
}

bool saving() {
    return saving_;
}

}  // namespace mesh_storage
