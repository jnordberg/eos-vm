#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <fstream>
#include <string>

#include <catch2/catch.hpp>

#include <eosio/vm/backend.hpp>

using namespace eosio;
using namespace eosio::vm;

// host functions that are C-style functions
// wasm hex
/* Code used to generate test, compile with eosio-cpp v1.6.2 with minor manual edits to remove unneeded imports
 * extern "C" {
      struct state_t { float f; int i; };
      [[eosio::wasm_import]]
      void c_style_host_function_0();
      [[eosio::wasm_import]]
      void c_style_host_function_1(int);
      [[eosio::wasm_import]]
      void c_style_host_function_2(int, int);
      [[eosio::wasm_import]]
      void c_style_host_function_3(int, float);
      [[eosio::wasm_import]]
      void c_style_host_function_4(const state_t&);

      [[eosio::wasm_entry]]
      void apply(unsigned long long a, unsigned long long b, unsigned long long c) {
         if (a == 0)
            c_style_host_function_0(); 
         else if (a == 1)
            c_style_host_function_1((int)b); 
         else if (a == 2)
            c_style_host_function_2((int)b, (int)c); 
         else if (a == 3)
            c_style_host_function_3((int)b, *((int*)&c)); 
         else if (a == 4) {
            state_t s = {*((float*)&c), (int)b};
            c_style_host_function_4(s); 
         }
      }
   } */

#include "host_functions_tests_0.wasm.hpp"
// no return value and no input parameters
int c_style_host_function_state = 0;
struct state_t {
   float f = 0;
   int   i = 0;
};
void c_style_host_function_0() {
   c_style_host_function_state = 1; 
}
void c_style_host_function_1(int s) {
   c_style_host_function_state = s;
}
void c_style_host_function_2(int a, int b) {
   c_style_host_function_state = a+b;
}
void c_style_host_function_3(int a, float b) {
   c_style_host_function_state = a+b;
}
void c_style_host_function_4(const state_t& ss) {
   c_style_host_function_state = ss.i;
}
// Combinations:
// Member/Free
// host object/converted host object/no host object/discarded host object - done
// parameters: none/bool/int32_t/uint32_t/int64_t/uint64_t/float/double/cv-pointer/cv-reference/two
// result: void/bool/int32_t/uint32_t/int64_t/uint64_t/float/double/cv-pointer/cv_reference - done
// call/call_indirect/direct execution - done
//
// Things that a host function might do:
// - call back into wasm
// - exit
// - wasm -> native -> wasm -> native -> exit
// - throw
// - calling into a different execution context
// - wasm1/wasm2 mixed -> native -> exit wasm2

namespace {

template<typename T>
struct ref {
   ref() = default;
   ref(T& arg) : val(&arg) {}
   operator T&() { return *val; }
   T* val;
};

template<typename T>
using maybe_ref = std::conditional_t<std::is_reference_v<T>, ref<std::remove_reference_t<T>>, T>;

template<typename T>
maybe_ref<T> global_test_value;

struct static_host_function {
   template<typename T>
   static void put(T t) { global_test_value<T> = t; }
   template<typename T>
   static T get() { return global_test_value<T>; }
};

struct member_host_function {
   template<typename T>
   void put(T t) { global_test_value<T> = t; }
   template<typename T>
   T get() { return global_test_value<T>; }
};

struct discard_host_function {};

struct transform_host_function {
   operator member_host_function() { return {}; }
};

}

#include "host_functions_tests_1.wasm.hpp"

wasm_code host_functions_tests_1_code{
   host_functions_tests_1_wasm + 0,
   host_functions_tests_1_wasm + sizeof(host_functions_tests_1_wasm)};

template<class Functions, class Host, class Transform>
struct init_backend {
   init_backend(Host* host) : _host(host) {
      add<bool>("b");
      add<int32_t>("i32");
      add<uint32_t>("ui32");
      add<int64_t>("i64");
      add<uint64_t>("ui64");
      add<float>("f32");
      add<double>("f64");
      add<char*>("ptr");
      add<const char*>("cptr");
      add<volatile char*>("vptr");
      add<const volatile char*>("cvptr");
      add<char&>("ref");
      add<const char&>("cref");
      add<volatile char&>("vref");
      add<const volatile char&>("cvref");

      bkend.set_wasm_allocator(&wa);
      bkend.initialize(nullptr);

      rhf_t::resolve(bkend.get_module());
   }
   template<typename T>
   void add(const std::string& name) {
      rhf_t::template add<Transform, &Functions::template put<T>, wasm_allocator>("env", "put_" + name);
      rhf_t::template add<Transform, &Functions::template get<T>, wasm_allocator>("env", "get_" + name);
   }
   // forwarding functions
   template<typename... A>
   auto call_with_return(A&&... a) { return bkend.call_with_return(_host, static_cast<A&&>(a)...); }
   template<typename... A>
   auto call(A&&... a) { return bkend.call(_host, static_cast<A&&>(a)...); }
   decltype(auto) get_context() { return bkend.get_context(); }

   using backend_t = eosio::vm::backend<Host>;
   using rhf_t     = eosio::vm::registered_host_functions<Host>;
   wasm_allocator wa;
   backend_t bkend{host_functions_tests_1_code};
   Host * _host;
};

// FIXME: allow direct calling of an imported and exported function
const std::vector<std::string> fun_prefixes = { /*"",*/ "call.", "call_indirect." };

template<typename T>
std::vector<T> test_values = { 0, 1, std::numeric_limits<T>::min(), std::numeric_limits<T>::max() };

template<>
std::vector<bool> test_values<bool> = { true, false };

int next_ptr_offset() {
   static int counter = 4;
   return counter++;
}

template<typename T, typename B>
void check_put(B& bkend, const std::string& name) {
   for(auto fun : fun_prefixes) {
      fun += "put_" + name;
      for(const T value : test_values<T>) {
         bkend.call("env", fun, value);
         CHECK(global_test_value<T> == value);
      }
   }
}

template<typename T, typename B>
void check_put_ptr(B& bkend, const std::string& name) {
   for(auto fun : fun_prefixes) {
      fun += "put_" + name;
      int offset = next_ptr_offset();
      const T value = const_cast<T>(bkend.get_context().linear_memory() + offset);
      bkend.call("env", fun, value);
      CHECK(global_test_value<T> == value);
   }
}

template<typename T, typename B>
void check_put_ref(B& bkend, const std::string& name) {
   for(auto fun : fun_prefixes) {
      fun += "put_" + name;
      int offset = next_ptr_offset();
      const auto value = &const_cast<T>(*(bkend.get_context().linear_memory() + offset));
      bkend.call("env", fun, value);
      CHECK(global_test_value<T>.val == value);
   }
}

template<typename T, typename B, typename F>
void check_get(B& bkend, const std::string& name, F getter) {
   for(auto fun : fun_prefixes) {
      fun += "get_" + name;
      for(const T value : test_values<T>) {
         global_test_value<T> = value;
         CHECK(getter(bkend.call_with_return("env", fun)) == value);
      }
   }
}

template<typename T, typename B>
void check_get_ptr(B& bkend, const std::string& name) {
   for(auto fun : fun_prefixes) {
      fun += "get_" + name;
      int offset = next_ptr_offset();
      global_test_value<T> = const_cast<T>(bkend.get_context().linear_memory() + offset);
      CHECK(bkend.call_with_return("env", fun)->to_ui32() == offset);
   }
}

template<typename T, typename B>
void check_get_ref(B& bkend, const std::string& name) {
   for(auto fun : fun_prefixes) {
      fun += "get_" + name;
      int offset = next_ptr_offset();
      global_test_value<T> = const_cast<T>(*(bkend.get_context().linear_memory() + offset));
      CHECK(bkend.call_with_return("env", fun)->to_ui32() == offset);
   }
}

template<class Backend>
void test_parameters(Backend&& bkend) {
   for(auto fun : fun_prefixes) {
      fun += "put_b";
      bkend.call("env", fun, true);
      CHECK(global_test_value<bool> == true);
      bkend.call("env", fun, false);
      CHECK(global_test_value<bool> == false);
      // Extra tests for bool:
      bkend.call("env", fun, 42);
      CHECK(global_test_value<bool> == true);
      bkend.call("env", fun, 0x10000);
      CHECK(global_test_value<bool> == true);
   }

   check_put<int32_t>(bkend, "i32");
   check_put<uint32_t>(bkend, "ui32");
   check_put<int64_t>(bkend, "i64");
   check_put<uint64_t>(bkend, "ui64");
   check_put<float>(bkend, "f32");
   check_put<double>(bkend, "f64");
   check_put_ptr<char*>(bkend, "ptr");
   check_put_ptr<const char*>(bkend, "cptr");
   check_put_ptr<volatile char*>(bkend, "vptr");
   check_put_ptr<const volatile char*>(bkend, "cvptr");
   check_put_ref<char&>(bkend, "ref");
   check_put_ref<const char&>(bkend, "cref");
   check_put_ref<volatile char&>(bkend, "vref");
   check_put_ref<const volatile char&>(bkend, "cvref");
}

TEST_CASE( "Test host function parameters", "[host_functions_parameters]" ) {
   test_parameters(init_backend<static_host_function, nullptr_t, nullptr_t>{nullptr});
   member_host_function mhf;
   test_parameters(init_backend<member_host_function, member_host_function, member_host_function>{&mhf});
   discard_host_function dhf;
   test_parameters(init_backend<static_host_function, discard_host_function, nullptr_t>{&dhf});
   transform_host_function thf;
   test_parameters(init_backend<member_host_function, transform_host_function, member_host_function>{&thf});
}

template<class Backend>
void test_results(Backend&& bkend) {
   for(auto fun : {/*"get_b",*/ "call.get_b", "call_indirect.get_b"}) {
      global_test_value<bool> = false;
      CHECK(bkend.call_with_return("env", fun)->to_ui32() == 0u);
      global_test_value<bool> = true;
      CHECK(bkend.call_with_return("env", fun)->to_ui32() == 1u);
   }

   check_get<int32_t>(bkend, "i32", [](auto x){ return x->to_i32(); });
   check_get<uint32_t>(bkend, "ui32", [](auto x){ return x->to_ui32(); });
   check_get<int64_t>(bkend, "i64", [](auto x){ return x->to_i64(); });
   check_get<uint64_t>(bkend, "ui64", [](auto x){ return x->to_ui64(); });
   check_get<float>(bkend, "f32", [](auto x){ return x->to_f32(); });
   check_get<double>(bkend, "f64", [](auto x){ return x->to_f64(); });
   check_get_ptr<char*>(bkend, "ptr");
   check_get_ptr<const char*>(bkend, "cptr");
   check_get_ptr<volatile char*>(bkend, "vptr");
   check_get_ptr<const volatile char*>(bkend, "cvptr");
   check_get_ref<char&>(bkend, "ref");
   check_get_ref<const char&>(bkend, "cref");
   check_get_ref<volatile char&>(bkend, "vref");
   check_get_ref<const volatile char&>(bkend, "cvref");
}

TEST_CASE( "Test host function results", "[host_functions_results]" ) {
   test_results(init_backend<static_host_function, nullptr_t, nullptr_t>{nullptr});
   member_host_function mhf;
   test_results(init_backend<member_host_function, member_host_function, member_host_function>{&mhf});
   discard_host_function dhf;
   test_results(init_backend<static_host_function, discard_host_function, nullptr_t>{&dhf});
   transform_host_function thf;
   test_results(init_backend<member_host_function, transform_host_function, member_host_function>{&thf});
}

TEST_CASE( "Test C-style host function system", "[C-style_host_functions_tests]") { 
   wasm_allocator wa;
   using backend_t = eosio::vm::backend<nullptr_t>;
   using rhf_t     = eosio::vm::registered_host_functions<nullptr_t>;
   rhf_t::add<nullptr_t, &c_style_host_function_0, wasm_allocator>("env", "c_style_host_function_0");
   rhf_t::add<nullptr_t, &c_style_host_function_1, wasm_allocator>("env", "c_style_host_function_1");
   rhf_t::add<nullptr_t, &c_style_host_function_2, wasm_allocator>("env", "c_style_host_function_2");
   rhf_t::add<nullptr_t, &c_style_host_function_3, wasm_allocator>("env", "c_style_host_function_3");
   rhf_t::add<nullptr_t, &c_style_host_function_4, wasm_allocator>("env", "c_style_host_function_4");

   backend_t bkend(host_functions_test_0_wasm);
   bkend.set_wasm_allocator(&wa);
   bkend.initialize(nullptr);

   rhf_t::resolve(bkend.get_module());

   bkend.call(nullptr, "env", "apply", (uint64_t)0, (uint64_t)0, (uint64_t)0);
   CHECK(c_style_host_function_state == 1);

   bkend.call(nullptr, "env", "apply", (uint64_t)1, (uint64_t)2, (uint64_t)0);
   CHECK(c_style_host_function_state == 2);

   bkend.call(nullptr, "env", "apply", (uint64_t)2, (uint64_t)1, (uint64_t)2);
   CHECK(c_style_host_function_state == 3);

   float f = 2.4f;
   bkend.call(nullptr, "env", "apply", (uint64_t)3, (uint64_t)2, *(uint64_t*)&f);
   CHECK(c_style_host_function_state == 0x40199980);

   bkend.call(nullptr, "env", "apply", (uint64_t)4, (uint64_t)5, *(uint64_t*)&f);
   CHECK(c_style_host_function_state == 5);
}