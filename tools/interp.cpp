#include <eosio/vm/backend.hpp>
#include <eosio/vm/error_codes.hpp>
#include <eosio/vm/host_function.hpp>
#include <eosio/vm/watchdog.hpp>

#include <iostream>

#include "wasi_api.hpp"

using namespace eosio;
using namespace eosio::vm;

struct cnv : type_converter<wasi_api> {
   using type_converter::type_converter;
   using type_converter::from_wasm;
   EOS_VM_FROM_WASM(bool, (uint32_t value)) { return value ? 1 : 0; }
   EOS_VM_FROM_WASM(uint8_t, (uint32_t value)) { return static_cast<uint8_t>(value); }
   EOS_VM_FROM_WASM(uint16_t, (uint32_t value)) { return static_cast<uint16_t>(value); }
   EOS_VM_FROM_WASM(char*, (void* ptr)) { return static_cast<char*>(ptr); }
   EOS_VM_FROM_WASM(const char*, (void* ptr)) { return static_cast<char*>(ptr); }
   EOS_VM_FROM_WASM(int*, (void* ptr)) { return static_cast<int*>(ptr); }
   EOS_VM_FROM_WASM(uint8_t*, (void* ptr)) { return static_cast<uint8_t*>(ptr); }
   EOS_VM_FROM_WASM(const uint8_t*, (void* ptr)) { return static_cast<const uint8_t*>(ptr); }
   EOS_VM_FROM_WASM(uint8_t**, (void* ptr)) { return static_cast<uint8_t**>(ptr); }
   EOS_VM_FROM_WASM(uint32_t*, (void* ptr)) { return static_cast<uint32_t*>(ptr); }
   EOS_VM_FROM_WASM(uint64_t*, (void* ptr)) { return static_cast<uint64_t*>(ptr); }
};

#define WASI_ADD_HOST_FUNCTION(cls, fn) \
   rhf_t::add<&cls::fn>("wasi_snapshot_preview1", #fn); \
   rhf_t::add<&cls::fn>("wasi_unstable", #fn);

/**
 * Simple implementation of an interpreter using eos-vm.
 */
int main(int argc, char** argv) {
   if (argc < 2) {
      std::cerr << "Error, no wasm file provided\n";
      return -1;
   }

   // Thread specific `allocator` used for wasm linear memory.
   wasm_allocator wa;

   // Specific the backend with no "host" for host functions.
   using rhf_t     = eosio::vm::registered_host_functions<wasi_api, execution_interface, cnv>;
   using backend_t = eosio::vm::backend<rhf_t>;

   WASI_ADD_HOST_FUNCTION(wasi_api, args_get)
   WASI_ADD_HOST_FUNCTION(wasi_api, args_sizes_get)
   WASI_ADD_HOST_FUNCTION(wasi_api, proc_exit)
   WASI_ADD_HOST_FUNCTION(wasi_api, fd_read)
   WASI_ADD_HOST_FUNCTION(wasi_api, fd_write)
   WASI_ADD_HOST_FUNCTION(wasi_api, fd_seek)
   WASI_ADD_HOST_FUNCTION(wasi_api, fd_close)
   WASI_ADD_HOST_FUNCTION(wasi_api, fd_fdstat_get)
   WASI_ADD_HOST_FUNCTION(wasi_api, fd_fdstat_set_flags)
   WASI_ADD_HOST_FUNCTION(wasi_api, fd_prestat_get)
   WASI_ADD_HOST_FUNCTION(wasi_api, fd_prestat_dir_name)
   WASI_ADD_HOST_FUNCTION(wasi_api, environ_sizes_get)
   WASI_ADD_HOST_FUNCTION(wasi_api, environ_get)
   WASI_ADD_HOST_FUNCTION(wasi_api, path_open)
   WASI_ADD_HOST_FUNCTION(wasi_api, path_filestat_get)
   WASI_ADD_HOST_FUNCTION(wasi_api, path_unlink_file)
   WASI_ADD_HOST_FUNCTION(wasi_api, clock_time_get)
   WASI_ADD_HOST_FUNCTION(wasi_api, random_get)

   watchdog wd{std::chrono::seconds(3)};
   wasi_api wasi{argc-1, &argv[1]};

   try {
      // Read the wasm into memory.
      auto code = read_wasm( argv[1] );

      // Instaniate a new backend using the wasm provided.
      backend_t bkend( code, wasi, &wa );

      wasi.exit = [&]() {
         bkend.exit(std::error_code());
      };

      wasi.offset_to_ptr = [&](auto offset) -> void* {
         return wa.create_pointer<char>(offset);
      };

      // Execute _start function provided by the wasm.
      bkend(wasi, "env", "_start");

   } catch ( const eosio::vm::exception& ex ) {
      std::cerr << "eos-vm interpreter error\n";
      std::cerr << ex.what() << " : " << ex.detail() << "\n";
   }
   return wasi.exit_code;
}
