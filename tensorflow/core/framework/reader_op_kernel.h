/* Copyright 2015 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_FRAMEWORK_READER_OP_KERNEL_H_
#define TENSORFLOW_FRAMEWORK_READER_OP_KERNEL_H_

#include <functional>
#include <string>

#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/reader_interface.h"
#include "tensorflow/core/framework/resource_mgr.h"
#include "tensorflow/core/framework/resource_op_kernel.h"
#include "tensorflow/core/platform/mutex.h"
#include "tensorflow/core/platform/types.h"

namespace tensorflow {

// NOTE: This is now a very thin layer over ResourceOpKernel.
// TODO(sjhwang): Remove dependencies to this class, then delete this.

// Implementation for ops providing a Reader.
class ReaderOpKernel : public ResourceOpKernel<ReaderInterface> {
 public:
  using ResourceOpKernel::ResourceOpKernel;

  // Must be called by descendants before the first call to Compute()
  // (typically called during construction).  factory must return a
  // ReaderInterface descendant allocated with new that ReaderOpKernel
  // will take ownership of.
  void SetReaderFactory(std::function<ReaderInterface*()> factory)
      LOCKS_EXCLUDED(mu_) {
    mutex_lock l(mu_);
    DCHECK(resource_ == nullptr);
    factory_ = factory;
  }

 private:
  Status CreateResource(ReaderInterface** reader)
      EXCLUSIVE_LOCKS_REQUIRED(mu_) override {
    *reader = factory_();
    if (*reader == nullptr) {
      return errors::ResourceExhausted("Failed to allocate reader");
    }
    std::function<ReaderInterface*()> temp = nullptr;
    factory_.swap(temp);
    return Status::OK();
  }

  std::function<ReaderInterface*()> factory_ GUARDED_BY(mu_);
};

}  // namespace tensorflow

#endif  // TENSORFLOW_FRAMEWORK_READER_OP_KERNEL_H_
