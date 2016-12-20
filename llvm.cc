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

#include <iostream>

using namespace std;
using namespace llvm;

Function *makeFunc(LLVMContext &C, Module* mod) {
  // Module Construction
  
  Constant* c = mod->getOrInsertFunction("mul_add",
					 /*ret type*/                          IntegerType::getInt32Ty(C),
					 /*args*/                              IntegerType::getInt32Ty(C),
					 IntegerType::getInt32Ty(C),
					 IntegerType::getInt32Ty(C),
					 /*varargs terminated with null*/       NULL);

  Function* mul_add = cast<Function>(c);
  mul_add->setCallingConv(CallingConv::C);


  Argument *args = &*mul_add->arg_begin();
  Value* x = args++;
  x->setName("x");
  Value* y = args++;
  y->setName("y");
  Value* z = args++;
  z->setName("z");

  BasicBlock* block = BasicBlock::Create(getGlobalContext(), "entry", mul_add);
  IRBuilder<> builder(block);

  Value* tmp = builder.CreateBinOp(Instruction::Mul,
				   x, y, "tmp");
  Value* tmp2 = builder.CreateBinOp(Instruction::Add,
				    tmp, z, "tmp2");

  builder.CreateRet(tmp2);

  //return mod;
  return mul_add;
}


int main(int argc, char **argv) {
  LLVMContext C;
  std::unique_ptr<Module> Owner(new Module("test", C));
  Module *mod = Owner.get();

  cout << "got here" << endl;
  Function *muladd = makeFunc(C, mod);
  
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

  std::vector<GenericValue> Args(3);
  Args[0].IntVal = APInt(32, 3);
  Args[1].IntVal = APInt(32, 5);
  Args[2].IntVal = APInt(32, 6);

  GenericValue GV = EE->runFunction(muladd, Args);
  outs() << "Result: " << GV.IntVal << "\n";
  
  
  return 0;
}
