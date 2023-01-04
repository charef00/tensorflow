/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

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

#ifndef TENSORFLOW_COMPILER_XLA_SERVICE_GPU_IR_EMISSION_UTILS_H_
#define TENSORFLOW_COMPILER_XLA_SERVICE_GPU_IR_EMISSION_UTILS_H_

#include <utility>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "tensorflow/compiler/xla/service/hlo_instruction.h"

namespace xla {
namespace gpu {

constexpr int64 kWarpSize = 32;

// Returns true if `hlo` will be implemented as a call to BLAS gemm.
bool ImplementedAsGemm(const HloInstruction& hlo);

// Returns true if `hlo` will be implemented as a call to cuDNN convolution.
bool ImplementedAsDnnConvolution(const HloInstruction& hlo);

// A call to cuDNN for batch normalization is represented as CustomCall HLO with
// a call target equal to one of these strings.
//
// The operands to and outputs of these calls are the same as those of the
// corresponding HLOs, except:
//
//  - epsilon and feature_index are proper operands, at the end of the operands
//    list.  They must be HLO constants.
//  - The cuDNN forward training call returns inv_stddev =
//    1/sqrt(variance + epsilon) in place of plain variance.
//  - Similarly, BatchNormGrad accepts inv_stddev in place of the variance
//    operand.
extern const char* const kCudnnBatchNormForwardInferenceCallTarget;
extern const char* const kCudnnBatchNormForwardTrainingCallTarget;
extern const char* const kCudnnBatchNormBackwardCallTarget;

// Returns true if `hlo` will be implemented as a call to a cuDNN batch
// normalization routine.
//
// This returns true if `hlo` is a CustomCall HLO with a call target equal to
// one of the kCudnnBatchNormFoo constants above, but returns *false* for HLOs
// with one of the kBatchNorm opcodes, because these are lowered either to a
// sequence of generic HLOs or to a cuDNN CustomCall.
bool IsCustomCallToDnnBatchNorm(const HloInstruction& hlo);

// Returns true if `hlo` will be implemented as a library call, e.g. cuBLAS gemm
// or cuDNN convolution.
bool ImplementedAsLibraryCall(const HloInstruction& hlo);

bool IsReductionToVector(const HloInstruction& reduce);

// Emits call to "vprintf" with given format and arguments.
llvm::Value* EmitPrintf(tensorflow::StringPiece fmt,
                        tensorflow::gtl::ArraySlice<llvm::Value*> arguments,
                        llvm::IRBuilder<>* builder);

// Emits code to shuffle data between threads of a warp. This has the same
// semantics as the PTX "shfl.down" instruction [0] but works for values of any
// size. The last operand of the emitted "shfl" is `kWarpSize - 1`.
//
// [0]
// http://docs.nvidia.com/cuda/parallel-thread-execution/#data-movement-and-conversion-instructions-shfl
llvm::Value* EmitShuffleDown(llvm::Value* value, llvm::Value* offset,
                             llvm::IRBuilder<>* builder);

}  // namespace gpu
}  // namespace xla

#endif  // TENSORFLOW_COMPILER_XLA_SERVICE_GPU_IR_EMISSION_UTILS_H_
