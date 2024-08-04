#include "llvm/Support/CommandLine.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/CodeGen/MIRParser/MIParser.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/FileSystem.h"

#include <iostream>
#include <filesystem>
#include <vector>

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>

using namespace llvm;

// Command line options for Analyzer
static cl::opt<std::string> BCFileOutputDir(
    "bcfiles-dir",
    cl::desc("LLVM IR files output directory."),
    cl::value_desc("LLVM IR files output directory")
);

static cl::opt<std::string> OutputDir(
    "output-dir",
    cl::desc("Output directory for analyze results."),
    cl::desc("Output directory for analyze results")
);

int main(int argc, char **argv)
{
    cl::ParseCommandLineOptions(argc, argv);
    SMDiagnostic Err;
    std::string outputdir;

    if (BCFileOutputDir.empty()) {
        BOOST_LOG_TRIVIAL(error) << "LLVM IR files output directory should be specified.";
        exit(1);
    }

    if (!OutputDir.empty()) {
        outputdir = OutputDir;
    } else {
        outputdir = "ga-output";
    }

    std::vector<std::string> bcfiles;

    for (const auto& p : std::filesystem::recursive_directory_iterator(std::string(BCFileOutputDir))) {
			if (!std::filesystem::is_directory(p)) {
					std::filesystem::path path = p.path();
					if (boost::algorithm::ends_with(path.string(), ".bc")) {
						bcfiles.push_back(path.string());
					}
			}
	}

    for (const auto &f : bcfiles) {
        LLVMContext *context = new LLVMContext();
        std::unique_ptr<Module> mod = parseIRFile(f, Err, *context);

        if (mod == NULL) {
            BOOST_LOG_TRIVIAL(warning) << "File " << f << " is not LLVM IR bitcode file.";
            continue;
        }

        Module *m = mod.release();
        for (auto it = m->begin(); it != m->end(); it++) {
            Function *F = &*it;
            if (!F->hasName()) {
                continue;
            }

            std::string functionName = static_cast<std::string>(F->getName());
            if (functionName == "") {
                continue;
            }

            // Skip if it is definition only.
            if (F->isDeclaration()) {
                continue;
            }

            DISubprogram *subprogram = F->getSubprogram();
            if (subprogram == NULL) {
                BOOST_LOG_TRIVIAL(warning) << "File " << f << " may not be built with debug option.";
                continue;
            }
            unsigned line = subprogram->getLine();

            std::cout << "Source: " << m->getSourceFileName() << ", Function:" << functionName << ", Line: " << line << std::endl;
        }
    }

    if (sys::fs::create_directory(outputdir)) {
        BOOST_LOG_TRIVIAL(error) << "Failed to create directory %s\n", outputdir;
        exit(1);
    }
    return 0;
}