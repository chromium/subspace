add_library(cir_lib STATIC
    "lib/run.cc"
    "lib/run.h"
    "lib/visit.cc"
    "lib/visit.h"
)

add_executable(cir
    "main.cc"
)

set_target_properties(cir PROPERTIES LINKER_LANGUAGE CXX)

# Subspace config.
target_include_directories(cir_lib PUBLIC ..)
target_link_libraries(cir_lib subspace)
target_include_directories(cir PUBLIC ..)
target_link_libraries(cir subspace cir_lib)

# Clang/LLVM config.
if($ENV{LLVM_INSTALL_DIR})
    target_include_directories(cir_lib PUBLIC
        $ENV{LLVM_INSTALL_DIR}/include
    )
    target_link_directories(cir PRIVATE
        $ENV{LLVM_INSTALL_DIR}/lib
    )
else()
    target_include_directories(cir_lib PUBLIC
        ../../llvm/build/install/include
    )
    target_link_directories(cir PRIVATE
        ../../llvm/build/install/lib
    )
endif()

if(MSVC)
    # This depends on if the LLVM build was debug or release:
    if($ENV{LLVM_DEBUG})
        set_property(TARGET cir_lib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
        set_property(TARGET cir PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
    else()
        set_property(TARGET cir_lib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        set_property(TARGET cir PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    endif()
endif()
target_link_libraries(cir
    clangAnalysis.lib
    clangAnalysisFlowSensitive.lib
    clangAnalysisFlowSensitiveModels.lib
    clangAPINotes.lib
    clangARCMigrate.lib
    clangAST.lib
    clangASTMatchers.lib
    clangBasic.lib
    clangCodeGen.lib
    clangCrossTU.lib
    clangDependencyScanning.lib
    clangDirectoryWatcher.lib
    clangDriver.lib
    clangDynamicASTMatchers.lib
    clangEdit.lib
    clangExtractAPI.lib
    clangFormat.lib
    clangFrontend.lib
    clangFrontendTool.lib
    clangHandleCXX.lib
    clangHandleLLVM.lib
    clangIndex.lib
    clangIndexSerialization.lib
    clangInterpreter.lib
    clangLex.lib
    clangParse.lib
    clangRewrite.lib
    clangRewriteFrontend.lib
    clangSema.lib
    clangSerialization.lib
    clangStaticAnalyzerCheckers.lib
    clangStaticAnalyzerCore.lib
    clangStaticAnalyzerFrontend.lib
    clangSupport.lib
    clangTooling.lib
    clangToolingASTDiff.lib
    clangToolingCore.lib
    clangToolingInclusions.lib
    clangToolingInclusionsStdlib.lib
    clangToolingRefactoring.lib
    clangToolingSyntax.lib
    clangTransformer.lib
    libclang.lib
    # For clang::tooling.
    version.dll
    # build/install/bin/llvm-config --libnames
    LLVMWindowsManifest.lib LLVMXRay.lib LLVMLibDriver.lib LLVMDlltoolDriver.lib LLVMCoverage.lib LLVMLineEditor.lib LLVMXCoreDisassembler.lib LLVMXCoreCodeGen.lib LLVMXCoreDesc.lib LLVMXCoreInfo.lib LLVMX86TargetMCA.lib LLVMX86Disassembler.lib LLVMX86AsmParser.lib LLVMX86CodeGen.lib LLVMX86Desc.lib LLVMX86Info.lib LLVMWebAssemblyDisassembler.lib LLVMWebAssemblyAsmParser.lib LLVMWebAssemblyCodeGen.lib LLVMWebAssemblyDesc.lib LLVMWebAssemblyUtils.lib LLVMWebAssemblyInfo.lib LLVMVEDisassembler.lib LLVMVEAsmParser.lib LLVMVECodeGen.lib LLVMVEDesc.lib LLVMVEInfo.lib LLVMSystemZDisassembler.lib LLVMSystemZAsmParser.lib LLVMSystemZCodeGen.lib LLVMSystemZDesc.lib LLVMSystemZInfo.lib LLVMSparcDisassembler.lib LLVMSparcAsmParser.lib LLVMSparcCodeGen.lib LLVMSparcDesc.lib LLVMSparcInfo.lib LLVMRISCVTargetMCA.lib LLVMRISCVDisassembler.lib LLVMRISCVAsmParser.lib LLVMRISCVCodeGen.lib LLVMRISCVDesc.lib LLVMRISCVInfo.lib LLVMPowerPCDisassembler.lib LLVMPowerPCAsmParser.lib LLVMPowerPCCodeGen.lib LLVMPowerPCDesc.lib LLVMPowerPCInfo.lib LLVMNVPTXCodeGen.lib LLVMNVPTXDesc.lib LLVMNVPTXInfo.lib LLVMMSP430Disassembler.lib LLVMMSP430AsmParser.lib LLVMMSP430CodeGen.lib LLVMMSP430Desc.lib LLVMMSP430Info.lib LLVMMipsDisassembler.lib LLVMMipsAsmParser.lib LLVMMipsCodeGen.lib LLVMMipsDesc.lib LLVMMipsInfo.lib LLVMLanaiDisassembler.lib LLVMLanaiCodeGen.lib LLVMLanaiAsmParser.lib LLVMLanaiDesc.lib LLVMLanaiInfo.lib LLVMHexagonDisassembler.lib LLVMHexagonCodeGen.lib LLVMHexagonAsmParser.lib LLVMHexagonDesc.lib LLVMHexagonInfo.lib LLVMBPFDisassembler.lib LLVMBPFAsmParser.lib LLVMBPFCodeGen.lib LLVMBPFDesc.lib LLVMBPFInfo.lib LLVMAVRDisassembler.lib LLVMAVRAsmParser.lib LLVMAVRCodeGen.lib LLVMAVRDesc.lib LLVMAVRInfo.lib LLVMARMDisassembler.lib LLVMARMAsmParser.lib LLVMARMCodeGen.lib LLVMARMDesc.lib LLVMARMUtils.lib LLVMARMInfo.lib LLVMAMDGPUTargetMCA.lib LLVMAMDGPUDisassembler.lib LLVMAMDGPUAsmParser.lib LLVMAMDGPUCodeGen.lib LLVMAMDGPUDesc.lib LLVMAMDGPUUtils.lib LLVMAMDGPUInfo.lib LLVMAArch64Disassembler.lib LLVMAArch64AsmParser.lib LLVMAArch64CodeGen.lib LLVMAArch64Desc.lib LLVMAArch64Utils.lib LLVMAArch64Info.lib LLVMOrcJIT.lib LLVMWindowsDriver.lib LLVMMCJIT.lib LLVMJITLink.lib LLVMInterpreter.lib LLVMExecutionEngine.lib LLVMRuntimeDyld.lib LLVMOrcTargetProcess.lib LLVMOrcShared.lib LLVMDWP.lib LLVMDebugInfoLogicalView.lib LLVMDebugInfoGSYM.lib LLVMOption.lib LLVMObjectYAML.lib LLVMObjCopy.lib LLVMMCA.lib LLVMMCDisassembler.lib LLVMLTO.lib LLVMPasses.lib LLVMCFGuard.lib LLVMCoroutines.lib LLVMipo.lib LLVMVectorize.lib LLVMLinker.lib LLVMInstrumentation.lib LLVMFrontendOpenMP.lib LLVMFrontendOpenACC.lib LLVMFrontendHLSL.lib LLVMExtensions.lib LLVMDWARFLinker.lib LLVMGlobalISel.lib LLVMMIRParser.lib LLVMAsmPrinter.lib LLVMSelectionDAG.lib LLVMCodeGen.lib LLVMObjCARCOpts.lib LLVMIRPrinter.lib LLVMInterfaceStub.lib LLVMFileCheck.lib LLVMFuzzMutate.lib LLVMTarget.lib LLVMScalarOpts.lib LLVMInstCombine.lib LLVMAggressiveInstCombine.lib LLVMTransformUtils.lib LLVMBitWriter.lib LLVMAnalysis.lib LLVMProfileData.lib LLVMSymbolize.lib LLVMDebugInfoPDB.lib LLVMDebugInfoMSF.lib LLVMDebugInfoDWARF.lib LLVMObject.lib LLVMTextAPI.lib LLVMMCParser.lib LLVMIRReader.lib LLVMAsmParser.lib LLVMMC.lib LLVMDebugInfoCodeView.lib LLVMBitReader.lib LLVMFuzzerCLI.lib LLVMCore.lib LLVMRemarks.lib LLVMBitstreamReader.lib LLVMBinaryFormat.lib LLVMTargetParser.lib LLVMTableGen.lib LLVMSupport.lib LLVMDemangle.lib
)
