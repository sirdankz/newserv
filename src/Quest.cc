#include "Quest.hh"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <phosg/Filesystem.hh>
#include <phosg/Encoding.hh>
#include <phosg/Random.hh>
#include <phosg/Strings.hh>

#include "Compression.hh"
#include "PSOEncryption.hh"
#include "Text.hh"

using namespace std;



struct PSODownloadQuestHeader {
  // When sending a DLQ to the client, this is the DECOMPRESSED size. When
  // reading it from a GCI file, this is the COMPRESSED size.
  le_uint32_t size;
  // Note: use PSO PC encryption, even for GC quests.
  le_uint32_t encryption_seed;
} __attribute__((packed));



bool category_is_mode(QuestCategory category) {
  return (category == QuestCategory::BATTLE) ||
         (category == QuestCategory::CHALLENGE) ||
         (category == QuestCategory::EPISODE_3);
}

const char* name_for_category(QuestCategory category) {
  switch (category) {
    case QuestCategory::RETRIEVAL:
      return "Retrieval";
    case QuestCategory::EXTERMINATION:
      return "Extermination";
    case QuestCategory::EVENT:
      return "Event";
    case QuestCategory::SHOP:
      return "Shop";
    case QuestCategory::VR:
      return "VR";
    case QuestCategory::TOWER:
      return "Tower";
    case QuestCategory::GOVERNMENT_EPISODE_1:
      return "GovernmentEpisode1";
    case QuestCategory::GOVERNMENT_EPISODE_2:
      return "GovernmentEpisode2";
    case QuestCategory::GOVERNMENT_EPISODE_4:
      return "GovernmentEpisode4";
    case QuestCategory::DOWNLOAD:
      return "Download";
    case QuestCategory::BATTLE:
      return "Battle";
    case QuestCategory::CHALLENGE:
      return "Challenge";
    case QuestCategory::SOLO:
      return "Solo";
    case QuestCategory::EPISODE_3:
      return "Episode3";
    default:
      return "Unknown";
  }
}



struct PSOQuestHeaderDC { // same for dc v1 and v2, thankfully
  uint32_t start_offset;
  uint32_t unknown_offset1;
  uint32_t size;
  uint32_t unused;
  uint8_t is_download;
  uint8_t unknown1;
  uint16_t quest_number; // 0xFFFF for challenge quests
  ptext<char, 0x20> name;
  ptext<char, 0x80> short_description;
  ptext<char, 0x120> long_description;
} __attribute__((packed));

struct PSOQuestHeaderPC {
  uint32_t start_offset;
  uint32_t unknown_offset1;
  uint32_t size;
  uint32_t unused;
  uint8_t is_download;
  uint8_t unknown1;
  uint16_t quest_number; // 0xFFFF for challenge quests
  ptext<char16_t, 0x20> name;
  ptext<char16_t, 0x80> short_description;
  ptext<char16_t, 0x120> long_description;
} __attribute__((packed));

struct PSOQuestHeaderGC {
  uint32_t start_offset;
  uint32_t unknown_offset1;
  uint32_t size;
  uint32_t unused;
  uint8_t is_download;
  uint8_t unknown1;
  uint8_t quest_number;
  uint8_t episode; // 1 = ep2. apparently some quests have 0xFF here, which means ep1 (?)
  ptext<char, 0x20> name;
  ptext<char, 0x80> short_description;
  ptext<char, 0x120> long_description;
} __attribute__((packed));

struct PSOQuestHeaderGCEpisode3 {
  // there's actually a lot of other important stuff in here but I'm lazy. it
  // looks like map data, cutscene data, and maybe special cards used during
  // the quest
  parray<uint8_t, 0x1DF0> unknown_a1;
  ptext<char, 0x14> name;
  ptext<char, 0x14> location;
  ptext<char, 0x3C> location2;
  ptext<char, 0x190> description;
  parray<uint8_t, 0x3A34> unknown_a2;
} __attribute__((packed));

struct PSOQuestHeaderBB {
  uint32_t start_offset;
  uint32_t unknown_offset1;
  uint32_t size;
  uint32_t unused;
  uint16_t quest_number; // 0xFFFF for challenge quests
  uint16_t unused2;
  uint8_t episode; // 0 = ep1, 1 = ep2, 2 = ep4
  uint8_t max_players;
  uint8_t joinable_in_progress;
  uint8_t unknown;
  ptext<char16_t, 0x20> name;
  ptext<char16_t, 0x80> short_description;
  ptext<char16_t, 0x120> long_description;
} __attribute__((packed));



Quest::Quest(const string& bin_filename)
  : quest_id(-1),
    category(QuestCategory::UNKNOWN),
    episode(0),
    is_dcv1(false),
    joinable(false),
    file_format(FileFormat::BIN_DAT) {

  if (ends_with(bin_filename, ".bin.gci")) {
    this->file_format = FileFormat::BIN_DAT_GCI;
    this->file_basename = bin_filename.substr(0, bin_filename.size() - 8);
  } else if (ends_with(bin_filename, ".bin.dlq")) {
    this->file_format = FileFormat::BIN_DAT_DLQ;
    this->file_basename = bin_filename.substr(0, bin_filename.size() - 8);
  } else if (ends_with(bin_filename, ".bin")) {
    this->file_basename = bin_filename.substr(0, bin_filename.size() - 4);
  } else {
    throw runtime_error("quest does not have a valid .bin file");
  }

  string basename;
  {
    size_t slash_pos = bin_filename.rfind('/');
    if (slash_pos != string::npos) {
      basename = bin_filename.substr(slash_pos + 1);
    } else {
      basename = bin_filename;
    }
  }
  basename.resize(basename.size() - ((this->file_format == FileFormat::BIN_DAT) ? 4 : 8));

  // quest filenames are like:
  // b###-VV.bin for battle mode
  // c###-VV.bin for challenge mode
  // e###-gc3.bin for episode 3
  // q###-CAT-VV.bin for normal quests

  if (basename.empty()) {
    throw invalid_argument("empty filename");
  }

  if (basename[0] == 'b') {
    this->category = QuestCategory::BATTLE;
  } else if (basename[0] == 'c') {
    this->category = QuestCategory::CHALLENGE;
  } else if (basename[0] == 'e') {
    this->category = QuestCategory::EPISODE_3;
  } else if (basename[0] != 'q') {
    throw invalid_argument("filename does not indicate mode");
  }

  // if the quest category is still unknown, expect 3 tokens (one of them will
  // tell us the category)
  vector<string> tokens = split(basename, '-');
  if (tokens.size() != (2 + (this->category == QuestCategory::UNKNOWN))) {
    throw invalid_argument("incorrect filename format");
  }

  // parse the number out of the first token
  this->quest_id = strtoull(tokens[0].c_str() + 1, nullptr, 10);

  // get the category from the second token if needed
  if (this->category == QuestCategory::UNKNOWN) {
    if (tokens[1] == "gov") {
      if (this->episode == 0) {
        this->category = QuestCategory::GOVERNMENT_EPISODE_1;
      } else if (this->episode == 1) {
        this->category = QuestCategory::GOVERNMENT_EPISODE_2;
      } else if (this->episode == 2) {
        this->category = QuestCategory::GOVERNMENT_EPISODE_4;
      } else {
        throw invalid_argument("government quest has incorrect episode");
      }
    } else {
      static const unordered_map<std::string, QuestCategory> name_to_category({
        {"ret", QuestCategory::RETRIEVAL},
        {"ext", QuestCategory::EXTERMINATION},
        {"evt", QuestCategory::EVENT},
        {"shp", QuestCategory::SHOP},
        {"vr",  QuestCategory::VR},
        {"twr", QuestCategory::TOWER},
        {"dl",  QuestCategory::DOWNLOAD},
        {"1p",  QuestCategory::SOLO},
      });
      this->category = name_to_category.at(tokens[1]);
    }
    tokens.erase(tokens.begin() + 1);
  }

  static const unordered_map<std::string, GameVersion> name_to_version({
    {"d1",  GameVersion::DC},
    {"dc",  GameVersion::DC},
    {"pc",  GameVersion::PC},
    {"gc",  GameVersion::GC},
    {"gc3", GameVersion::GC},
    {"bb",  GameVersion::BB},
  });
  this->version = name_to_version.at(tokens[1]);

  // the rest of the information needs to be fetched from the .bin file's
  // contents

  auto bin_compressed = this->bin_contents();
  auto bin_decompressed = prs_decompress(*bin_compressed);

  switch (this->version) {
    case GameVersion::PATCH:
      throw invalid_argument("patch server quests are not valid");
      break;

    case GameVersion::DC: {
      if (bin_decompressed.size() < sizeof(PSOQuestHeaderDC)) {
        throw invalid_argument("file is too small for header");
      }
      auto* header = reinterpret_cast<const PSOQuestHeaderDC*>(bin_decompressed.data());
      this->joinable = false;
      this->episode = 0;
      this->name = decode_sjis(header->name);
      this->short_description = decode_sjis(header->short_description);
      this->long_description = decode_sjis(header->long_description);
      this->is_dcv1 = (tokens[1] == "d1");
      break;
    }

    case GameVersion::PC: {
      if (bin_decompressed.size() < sizeof(PSOQuestHeaderPC)) {
        throw invalid_argument("file is too small for header");
      }
      auto* header = reinterpret_cast<const PSOQuestHeaderPC*>(bin_decompressed.data());
      this->joinable = false;
      this->episode = 0;
      this->name = header->name;
      this->short_description = header->short_description;
      this->long_description = header->long_description;
      break;
    }

    case GameVersion::GC: {
      if (this->category == QuestCategory::EPISODE_3) {
        // these all appear to be the same size
        if (bin_decompressed.size() != sizeof(PSOQuestHeaderGCEpisode3)) {
          throw invalid_argument("file is incorrect size");
        }
        auto* header = reinterpret_cast<const PSOQuestHeaderGCEpisode3*>(bin_decompressed.data());
        this->joinable = false;
        this->episode = 0xFF;
        this->name = decode_sjis(header->name);
        this->short_description = decode_sjis(header->location2);
        this->long_description = decode_sjis(header->description);
      } else {
        if (bin_decompressed.size() < sizeof(PSOQuestHeaderGC)) {
          throw invalid_argument("file is too small for header");
        }
        auto* header = reinterpret_cast<const PSOQuestHeaderGC*>(bin_decompressed.data());
        this->joinable = false;
        this->episode = (header->episode == 1);
        this->name = decode_sjis(header->name);
        this->short_description = decode_sjis(header->short_description);
        this->long_description = decode_sjis(header->long_description);
      }
      break;
    }

    case GameVersion::BB: {
      if (bin_decompressed.size() < sizeof(PSOQuestHeaderBB)) {
        throw invalid_argument("file is too small for header");
      }
      auto* header = reinterpret_cast<const PSOQuestHeaderBB*>(bin_decompressed.data());
      this->joinable = header->joinable_in_progress;
      this->episode = header->episode;
      this->name = header->name;
      this->short_description = header->short_description;
      this->long_description = header->long_description;
      break;

    }
  }
}

static string basename_for_filename(const std::string& filename) {
  size_t slash_pos = filename.rfind('/');
  if (slash_pos != string::npos) {
    return filename.substr(slash_pos + 1);
  }
  return filename;
}

std::string Quest::bin_filename() const {
  return basename_for_filename(this->file_basename + ".bin");
}

std::string Quest::dat_filename() const {
  return basename_for_filename(this->file_basename + ".dat");
}

shared_ptr<const string> Quest::bin_contents() const {
  if (!this->bin_contents_ptr) {
    switch (this->file_format) {
      case FileFormat::BIN_DAT:
        this->bin_contents_ptr.reset(new string(load_file(this->file_basename + ".bin")));
        break;
      case FileFormat::BIN_DAT_GCI:
        this->bin_contents_ptr.reset(new string(this->decode_gci(this->file_basename + ".bin.gci")));
        break;
      case FileFormat::BIN_DAT_DLQ:
        this->bin_contents_ptr.reset(new string(this->decode_dlq(this->file_basename + ".bin.dlq")));
        break;
      default:
        throw logic_error("invalid quest file format");
    }
  }
  return this->bin_contents_ptr;
}

shared_ptr<const string> Quest::dat_contents() const {
  if (!this->dat_contents_ptr) {
    switch (this->file_format) {
      case FileFormat::BIN_DAT:
        this->dat_contents_ptr.reset(new string(load_file(this->file_basename + ".dat")));
        break;
      case FileFormat::BIN_DAT_GCI:
        this->dat_contents_ptr.reset(new string(this->decode_gci(this->file_basename + ".dat.gci")));
        break;
      case FileFormat::BIN_DAT_DLQ:
        this->dat_contents_ptr.reset(new string(this->decode_dlq(this->file_basename + ".dat.dlq")));
        break;
      default:
        throw logic_error("invalid quest file format");
    }
  }
  return this->dat_contents_ptr;
}

string Quest::decode_dlq(const string& filename) {
  uint32_t decompressed_size;
  uint32_t key;
  string data;
  {
    auto f = fopen_unique(filename, "rb");
    decompressed_size = freadx<le_uint32_t>(f.get());
    key = freadx<le_uint32_t>(f.get());
    data = read_all(f.get());
  }

  PSOPCEncryption encr(key);

  // The compressed data size does not need to be a multiple of 4, but the PC
  // encryption (which is used for all download quests, even in V3) requires the
  // data size to be a multiple of 4. We'll just temporarily stick a few bytes
  // on the end, then throw them away later if needed.
  size_t original_size = data.size();
  data.resize((data.size() + 3) & (~3));
  encr.decrypt(data);
  data.resize(original_size);

  if (prs_decompress_size(data) != decompressed_size) {
    throw runtime_error("decompressed size does not match size in header");
  }

  return data;
}

string Quest::decode_gci(const string& filename) {

  string data = load_file(filename);
  if (data.size() < 0x2080 + sizeof(PSODownloadQuestHeader)) {
    throw runtime_error(string_printf(
        "GCI file is truncated before download quest header (have 0x%zX bytes)", data.size()));
  }
  PSODownloadQuestHeader* h = reinterpret_cast<PSODownloadQuestHeader*>(
      data.data() + 0x2080);

  string compressed_data_with_header = data.substr(0x2088, h->size);

  // For now, we can only load unencrypted quests, unfortunately
  // TODO: Figure out how GCI encryption works and implement it here.

  // Unlike the DLQ header, this one is stored little-endian. The compressed
  // data immediately follows this header.
  struct DecryptedHeader {
    uint32_t unknown1;
    uint32_t unknown2;
    uint32_t decompressed_size;
    uint32_t unknown4;
  } __attribute__((packed));
  if (compressed_data_with_header.size() < sizeof(DecryptedHeader)) {
    throw runtime_error("GCI file compressed data truncated during header");
  }
  DecryptedHeader* dh = reinterpret_cast<DecryptedHeader*>(
      compressed_data_with_header.data());
  if (dh->unknown1 || dh->unknown2 || dh->unknown4) {
    throw runtime_error("GCI file appears to be encrypted");
  }

  string data_to_decompress = compressed_data_with_header.substr(sizeof(DecryptedHeader));
  size_t decompressed_bytes = prs_decompress_size(data_to_decompress);

  size_t expected_decompressed_bytes = dh->decompressed_size - 8;
  if (decompressed_bytes < expected_decompressed_bytes) {
    throw runtime_error(string_printf(
        "GCI decompressed data is smaller than expected size (have 0x%zX bytes, expected 0x%zX bytes)",
        decompressed_bytes, expected_decompressed_bytes));
  }

  // The caller expects to get PRS-compressed data when calling bin_contents()
  // and dat_contents(), so we shouldn't decompress it here.
  return data_to_decompress;
}



QuestIndex::QuestIndex(const std::string& directory) : directory(directory) {
  auto filename_set = list_directory(this->directory);
  vector<string> filenames(filename_set.begin(), filename_set.end());
  sort(filenames.begin(), filenames.end());
  for (const auto& filename : filenames) {
    string full_path = this->directory + "/" + filename;

    if (ends_with(filename, ".gba")) {
      shared_ptr<string> contents(new string(load_file(full_path)));
      this->gba_file_contents.emplace(make_pair(filename, contents));
      continue;
    }

    if (ends_with(filename, ".bin") ||
        ends_with(filename, ".bin.gci") ||
        ends_with(filename, ".bin.dlq")) {
      try {
        shared_ptr<Quest> q(new Quest(full_path));
        this->version_id_to_quest.emplace(make_pair(q->version, q->quest_id), q);
        this->version_name_to_quest.emplace(make_pair(q->version, q->name), q);
        string ascii_name = encode_sjis(q->name);
        log(INFO, "Indexed quest %s (%s-%" PRId64 ", %s, episode=%hhu, joinable=%s, dcv1=%s)",
            ascii_name.c_str(), name_for_version(q->version), q->quest_id,
            name_for_category(q->category), q->episode,
            q->joinable ? "true" : "false", q->is_dcv1 ? "true" : "false");
      } catch (const exception& e) {
        log(WARNING, "Failed to parse quest file %s (%s)", filename.c_str(), e.what());
      }
    }
  }
}

shared_ptr<const Quest> QuestIndex::get(GameVersion version,
    uint32_t id) const {
  return this->version_id_to_quest.at(make_pair(version, id));
}

shared_ptr<const string> QuestIndex::get_gba(const string& name) const {
  return this->gba_file_contents.at(name);
}

vector<shared_ptr<const Quest>> QuestIndex::filter(GameVersion version,
    bool is_dcv1, QuestCategory category, int16_t episode) const {
  auto it = this->version_id_to_quest.lower_bound(make_pair(version, 0));
  auto end_it = this->version_id_to_quest.upper_bound(make_pair(version, 0xFFFFFFFF));

  vector<shared_ptr<const Quest>> ret;
  for (; it != end_it; it++) {
    shared_ptr<const Quest> q = it->second;
    if ((q->is_dcv1 != is_dcv1) || (q->category != category)) {
      continue;
    }

    // Only check episode and solo if the category isn't a mode (that is, ignore
    // episode if querying for battle/challenge/solo quests). Also, ignore
    // ignore episode if it's < 0 (e.g. for the download quest menu).
    if ((episode >= 0) && !category_is_mode(category) && ((q->episode != episode))) {
      continue;
    }

    ret.emplace_back(q);
  }

  return ret;
}



static string create_download_quest_file(const string& compressed_data,
    size_t decompressed_size, uint32_t seed = 0) {
  if (seed == 0) {
    seed = random_object<uint32_t>();
  }

  string data(8, '\0');
  auto* header = reinterpret_cast<PSODownloadQuestHeader*>(data.data());
  header->size = decompressed_size;
  header->encryption_seed = seed;
  data += compressed_data;

  // Add temporary extra bytes if necessary so encryption won't fail
  size_t original_size = data.size();
  data.resize((data.size() + 3) & (~3));

  PSOPCEncryption encr(seed);
  encr.encrypt(data.data() + sizeof(PSODownloadQuestHeader),
      data.size() - sizeof(PSODownloadQuestHeader));
  data.resize(original_size);

  return data;
}

shared_ptr<Quest> Quest::create_download_quest() const {
  string decompressed_bin = prs_decompress(*this->bin_contents());

  // The download flag needs to be set in the bin header, or else the client
  // will ignore it when scanning for download quests in an offline game.
  void* data_ptr = decompressed_bin.data();
  switch (this->version) {
    case GameVersion::DC:
      if (decompressed_bin.size() < sizeof(PSOQuestHeaderDC)) {
        throw runtime_error("bin file is too small for header");
      }
      reinterpret_cast<PSOQuestHeaderDC*>(data_ptr)->is_download = 0x01;
      break;
    case GameVersion::PC:
      if (decompressed_bin.size() < sizeof(PSOQuestHeaderPC)) {
        throw runtime_error("bin file is too small for header");
      }
      reinterpret_cast<PSOQuestHeaderPC*>(data_ptr)->is_download = 0x01;
      break;
    case GameVersion::GC:
      if (decompressed_bin.size() < sizeof(PSOQuestHeaderGC)) {
        throw runtime_error("bin file is too small for header");
      }
      reinterpret_cast<PSOQuestHeaderGC*>(data_ptr)->is_download = 0x01;
      break;
    case GameVersion::BB:
      throw invalid_argument("PSOBB does not support download quests");
    default:
      throw invalid_argument("unknown game version");
  }

  shared_ptr<Quest> dlq(new Quest(*this));

  string compressed_bin = prs_compress(decompressed_bin);
  dlq->bin_contents_ptr.reset(new string(create_download_quest_file(
      compressed_bin, decompressed_bin.size())));

  dlq->dat_contents_ptr.reset(new string(create_download_quest_file(
      *this->dat_contents(), prs_decompress_size(*this->dat_contents()))));

  return dlq;
}
