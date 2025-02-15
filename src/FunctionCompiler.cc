#include "FunctionCompiler.hh"

#include <stdio.h>
#include <string.h>

#include <stdexcept>
#include <phosg/Filesystem.hh>

#ifdef HAVE_RESOURCE_FILE
#include <resource_file/Emulators/PPC32Emulator.hh>
#endif

#include "Loggers.hh"
#include "CommandFormats.hh"

using namespace std;



bool function_compiler_available() {
#ifndef HAVE_RESOURCE_FILE
  return false;
#else
  return true;
#endif
}



const char* name_for_architecture(CompiledFunctionCode::Architecture arch) {
  switch (arch) {
    case CompiledFunctionCode::Architecture::POWERPC:
      return "PowerPC";
    case CompiledFunctionCode::Architecture::X86:
      return "x86";
    default:
      throw logic_error("invalid architecture");
  }
}



template <typename FooterT, typename U16T>
string CompiledFunctionCode::generate_client_command_t(
      const unordered_map<string, uint32_t>& label_writes,
      const string& suffix) const {
  FooterT footer;
  footer.num_relocations = this->relocation_deltas.size();
  footer.unused1.clear(0);
  footer.entrypoint_addr_offset = this->entrypoint_offset_offset;
  footer.unused2.clear(0);

  StringWriter w;
  if (!label_writes.empty()) {
    string modified_code = this->code;
    for (const auto& it : label_writes) {
      size_t offset = this->label_offsets.at(it.first);
      if (offset > modified_code.size() - 4) {
        throw runtime_error("label out of range");
      }
      *reinterpret_cast<be_uint32_t*>(modified_code.data() + offset) = it.second;
    }
    w.write(modified_code);
  } else {
    w.write(this->code);
  }
  w.write(suffix);
  while (w.size() & 3) {
    w.put_u8(0);
  }

  footer.relocations_offset = w.size();
  for (uint16_t delta : this->relocation_deltas) {
    w.put<U16T>(delta);
  }
  if (this->relocation_deltas.size() & 1) {
    w.put_u16(0);
  }

  w.put(footer);
  return move(w.str());
}

string CompiledFunctionCode::generate_client_command(
      const unordered_map<string, uint32_t>& label_writes,
      const string& suffix) const {
  if (this->arch == Architecture::POWERPC) {
    return this->generate_client_command_t<S_ExecuteCode_Footer_GC_B2, be_uint16_t>(
        label_writes, suffix);
  } else if ((this->arch == Architecture::X86) || (this->arch == Architecture::SH4)) {
    return this->generate_client_command_t<S_ExecuteCode_Footer_DC_PC_XB_BB_B2, le_uint16_t>(
        label_writes, suffix);
  } else {
    throw logic_error("invalid architecture");
  }
}

bool CompiledFunctionCode::is_big_endian() const {
  return this->arch == Architecture::POWERPC;
}



shared_ptr<CompiledFunctionCode> compile_function_code(
    CompiledFunctionCode::Architecture arch,
    const string& directory,
    const string& name,
    const string& text) {
#ifndef HAVE_RESOURCE_FILE
  (void)arch;
  (void)directory;
  (void)name;
  (void)text;
  throw runtime_error("function compiler is not available");

#else
  shared_ptr<CompiledFunctionCode> ret(new CompiledFunctionCode());
  ret->arch = arch;
  ret->name = name;
  ret->index = 0;

  if (arch == CompiledFunctionCode::Architecture::POWERPC) {
    auto assembled = PPC32Emulator::assemble(text, {directory});
    ret->code = move(assembled.code);
    ret->label_offsets = move(assembled.label_offsets);
  } else if (arch == CompiledFunctionCode::Architecture::X86) {
    throw runtime_error("x86 assembler is not implemented");
  }

  set<uint32_t> reloc_indexes;
  for (const auto& it : ret->label_offsets) {
    if (starts_with(it.first, "reloc")) {
      reloc_indexes.emplace(it.second / 4);
    } else if (starts_with(it.first, "newserv_index_")) {
      ret->index = stoul(it.first.substr(14), nullptr, 16);
    }
  }

  try {
    ret->entrypoint_offset_offset = ret->label_offsets.at("entry_ptr");
  } catch (const out_of_range&) {
    throw runtime_error("code does not contain entry_ptr label");
  }

  uint32_t prev_index = 0;
  for (const auto& it : reloc_indexes) {
    uint32_t delta = it - prev_index;
    if (delta > 0xFFFF) {
      throw runtime_error("relocation delta too far away");
    }
    ret->relocation_deltas.emplace_back(delta);
    prev_index = it;
  }

  return ret;
#endif
}



FunctionCodeIndex::FunctionCodeIndex(const string& directory) {
  if (!function_compiler_available()) {
    function_compiler_log.info("Function compiler is not available");
    return;
  }

  uint32_t next_menu_item_id = 0;
  for (const auto& filename : list_directory(directory)) {
    if (!ends_with(filename, ".s") || ends_with(filename, ".inc.s")) {
      continue;
    }
    bool is_patch = ends_with(filename, ".patch.s");
    string name = filename.substr(0, filename.size() - (is_patch ? 8 : 2));

    try {
      string path = directory + "/" + filename;
      string text = load_file(path);
      auto code = compile_function_code(
          CompiledFunctionCode::Architecture::POWERPC, directory, name, text);
      if (code->index != 0) {
        if (!this->index_to_function.emplace(code->index, code).second) {
          throw runtime_error(string_printf(
              "duplicate function index: %08" PRIX32, code->index));
        }
      }
      this->name_to_function.emplace(name, code);
      if (is_patch) {
        code->menu_item_id = next_menu_item_id++;
        this->menu_item_id_to_patch_function.emplace(code->menu_item_id, code);
        this->name_to_patch_function.emplace(name, code);
      }

      string index_prefix = code->index ? string_printf("%02X => ", code->index) : "";
      string patch_prefix = is_patch ? string_printf("[%08" PRIX32 "] ", code->menu_item_id) : "";
      function_compiler_log.info("Compiled function %s%s%s (%s)",
          index_prefix.c_str(), patch_prefix.c_str(), name.c_str(), name_for_architecture(code->arch));

    } catch (const exception& e) {
      function_compiler_log.warning("Failed to compile function %s: %s", name.c_str(), e.what());
    }
  }
}

vector<MenuItem> FunctionCodeIndex::patch_menu() const {
  vector<MenuItem> ret;
  ret.emplace_back(PatchesMenuItemID::GO_BACK, u"Go back", u"", 0);
  for (const auto& it : this->name_to_patch_function) {
    const auto& fn = it.second;
    ret.emplace_back(fn->menu_item_id, decode_sjis(fn->name), u"",
        MenuItem::Flag::REQUIRES_SEND_FUNCTION_CALL);
  }
  return ret;
}



DOLFileIndex::DOLFileIndex(const string& directory) {
  if (!function_compiler_available()) {
    function_compiler_log.info("Function compiler is not available");
    return;
  }
  if (!isdir(directory)) {
    function_compiler_log.info("DOL file directory is missing");
    return;
  }

  uint32_t next_menu_item_id = 0;
  for (const auto& filename : list_directory(directory)) {
    if (!ends_with(filename, ".dol")) {
      continue;
    }
    string name = filename.substr(0, filename.size() - 4);

    try {
      shared_ptr<DOLFile> dol(new DOLFile());
      dol->menu_item_id = next_menu_item_id++;
      dol->name = name;

      string path = directory + "/" + filename;
      dol->data = load_file(path);

      this->name_to_file.emplace(dol->name, dol);
      this->item_id_to_file.emplace_back(dol);
      function_compiler_log.info("Loaded DOL file %s", filename.c_str());

    } catch (const exception& e) {
      function_compiler_log.warning("Failed to load DOL file %s: %s", filename.c_str(), e.what());
    }
  }
}

vector<MenuItem> DOLFileIndex::menu() const {
  vector<MenuItem> ret;
  ret.emplace_back(ProgramsMenuItemID::GO_BACK, u"Go back", u"", 0);
  for (const auto& dol : this->item_id_to_file) {
    ret.emplace_back(dol->menu_item_id, decode_sjis(dol->name), u"",
        MenuItem::Flag::REQUIRES_SEND_FUNCTION_CALL);
  }
  return ret;
}
