/*
 * Tencent is pleased to support the open source community by making ScriptX available.
 * Copyright (C) 2021 THL A29 Limited, a Tencent company.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "../../../src/foundation.h"
#include "../../../src/types.h"
#include "../../../src/utils/GlobalWeakBookkeeping.hpp"
#include "../V8Helper.h"

namespace script {

namespace v8_backend {

// T is script type
// type is mapped v8 type
template <typename T>
struct _V8ValueType {};

template <typename T>
using V8ValueType = typename _V8ValueType<T>::type;

#define TypeMap(scriptType, v8Type) \
  template <>                       \
  struct _V8ValueType<scriptType> { \
    using type = v8Type;            \
  }

TypeMap(::script::Value, v8::Value);
TypeMap(::script::Object, v8::Object);
TypeMap(::script::String, v8::String);
TypeMap(::script::Number, v8::Number);
TypeMap(::script::Boolean, v8::Boolean);
TypeMap(::script::Function, v8::Function);
TypeMap(::script::Array, v8::Array);
TypeMap(::script::ByteBuffer, v8::Value);
TypeMap(::script::Unsupported, v8::Value);
// TypeMap(::script::ConfigObject, v8::ConfigObject);

#undef TypeMap

template <typename T>
class GlobalRefState {
 private:
  // v8::CopyablePersistentTraits deprecated in 10.5, removed in 12.5
  // always use v8::Global
  using V8Global = v8::Global<v8_backend::V8ValueType<T>>;

 public:
  V8Engine* engine_ = nullptr;
  V8Global ref_{};
  ::script::internal::GlobalWeakBookkeeping::HandleType handle_{};

  GlobalRefState() = default;

  GlobalRefState(V8Engine* scriptEngine, const Local<T>& localReference);

  GlobalRefState(V8Engine* scriptEngine, const V8Global& v8Global);

  // v8::Global don't support copy
  GlobalRefState(const GlobalRefState& copy)
      : engine_(copy.engine_), ref_{currentEngineIsolateChecked(), copy.ref_} {}

  GlobalRefState& operator=(const GlobalRefState& assign) {
    if (this != &assign) {
      engine_ = assign.engine_;
      ref_ = V8Global(currentEngineIsolateChecked(), assign.ref_);
    }
    return *this;
  }

  GlobalRefState(GlobalRefState&& move) noexcept
      : engine_(move.engine_), ref_(std::move(move.ref_)) {
    // v8 Ref don't have a move assign
    // this is still a copy
    move.ref_.Reset();
    move.engine_ = nullptr;
  }

  GlobalRefState& operator=(GlobalRefState&& assign) noexcept {
    if (this != &assign) {
      engine_ = assign.engine_;
      // v8 Ref don't have a move assign
      // this is still a copy
      ref_ = std::move(assign.ref_);
      assign.ref_.Reset();
      assign.engine_ = nullptr;
    }
    return *this;
  }

  void swap(GlobalRefState& other) {
    std::swap(ref_, other.ref_);
    std::swap(engine_, other.engine_);
  }

  void makeWeak() {
    if (!ref_.IsEmpty()) ref_.SetWeak();
  }
};

}  // namespace v8_backend

template <typename T>
struct internal::ImplType<Local<T>> {
  using type = typename v8::Local<v8_backend::V8ValueType<T>>;
};

template <typename T>
struct internal::ImplType<Global<T>> {
  using type = typename v8_backend::GlobalRefState<T>;
};

template <typename T>
struct internal::ImplType<Weak<T>> {
  using type = typename v8_backend::GlobalRefState<T>;
};

}  // namespace script
