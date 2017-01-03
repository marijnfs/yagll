#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/Support/TargetSelect.h"

#include <iostream>

using namespace std;
using namespace llvm;

Function *makeFunc(LLVMContext &C, Module* mod) {
  std::vector<Type*> inputs;
  inputs.push_back(Type::getInt32Ty(C));
  inputs.push_back(Type::getInt32Ty(C));
  inputs.push_back(Type::getInt32Ty(C));

  //Function *mul_add = mod->getFunction("mul_add");

  FunctionType *FT = FunctionType::get(Type::getInt32Ty(C),
				       inputs, false);
  Function *mul_add = Function::Create(FT, Function::ExternalLinkage, "mul_add", mod);
  //Function *mul_add = cast<Function>(mod->getOrInsertFunction("muladd", FT));
  mul_add->setCallingConv(CallingConv::C);

  auto args = mul_add->arg_begin();

  Argument *x = &*args++;
  x->setName("x");
  Argument *y = &*args++;
  y->setName("y");
  Argument *z = &*args++;
  z->setName("z");

  
  BasicBlock* block = BasicBlock::Create(C, "entry", mul_add, 0);
  Value *tmp = BinaryOperator::CreateMul(x, y, "tmp", block);
  Value *result = BinaryOperator::CreateAdd(tmp, z, "result", block);
  ReturnInst::Create(C, result, block);

  //return mod;
  return mul_add;
}


int main(int argc, char **argv) {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();
  
  LLVMContext C;

  std::unique_ptr<Module> Owner(new Module("test", C));
  Module *mod = Owner.get();

  Function *muladd = makeFunc(C, mod);

  //muladd = mod->getFunction("muladd");
  if (verifyFunction(*muladd)) {
    cout << "failed verification" << endl;
    return 1;
  }

  std::string errStr;
  ExecutionEngine *EE =
    EngineBuilder(std::move(Owner))
    .setErrorStr(&errStr)
    .create();

    if (!EE) {
    errs() << argv[0] << ": Failed to construct ExecutionEngine: " << errStr
	   << "\n";
    return 1;
  }

  errs() << "verifying... ";
  if (verifyModule(*mod)) {
    errs() << argv[0] << ": Error constructing function!\n";
    return 1;
  }

  EE->finalizeObject();
    
  /*std::vector<GenericValue> Args(3);
  Args[0].IntVal = APInt(32, 1300);
  Args[1].IntVal = APInt(32, 1500);
  Args[2].IntVal = APInt(32, 1600);
  //GenericValue GV = EE->runFunction(muladd, Args);*/

  //cout << muladd << endl;
  typedef uint32_t bla(uint32_t, uint32_t, uint32_t);

  bla *f = (bla*)EE->getFunctionAddress("mul_add");
  
  outs() << "Result: " << f(1233,12315,61313) << "\n";
  
  
  return 0;
}
