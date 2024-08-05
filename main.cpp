#include "llvm/Support/CommandLine.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/CodeGen/MIRParser/MIParser.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/FileSystem.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

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

static bool has_data(const std::vector<std::string> &v, const std::string &s)
{
    auto it = std::find(v.begin(), v.end(), s);
    return it != v.end();
}

static void write_to_file(const std::vector<std::string> &v, const std::string &outputdir, const std::string &filename)
{
    llvm::outs() << "Write to " << outputdir << "/" << filename << "\n";
    std::ofstream of(outputdir + "/" + filename, std::ofstream::out);
    for (const auto &s : v) {
        of << s;
    }
}

int main(int argc, char **argv)
{
    cl::ParseCommandLineOptions(argc, argv);
    SMDiagnostic Err;
    std::string outputdir;

    if (BCFileOutputDir.empty()) {
        llvm::errs() << "LLVM IR files output directory should be specified.\n";
        exit(1);
    }

    if (!OutputDir.empty()) {
        outputdir = OutputDir;
    } else {
        outputdir = "ga-output";
    }

    std::vector<std::string> bcfiles;
    std::vector<std::string> functions;
    std::vector<std::string> basicblocks;

    for (const auto& p : std::filesystem::recursive_directory_iterator(std::string(BCFileOutputDir))) {
        if (!std::filesystem::is_directory(p)) {
            std::filesystem::path path = p.path();
            if (boost::algorithm::ends_with(path.string(), ".bc")) {
                std::cout << "Add " << path.string() << std::endl;
                bcfiles.push_back(path.string());
            }
        }
	}

    if (bcfiles.empty()) {
        llvm::dbgs() << ".bc file is not found.\n";
        exit(1);
    }

    for (const auto &f : bcfiles) {
        llvm::outs() << "Analyzing file:" << f << "\n";

        LLVMContext *context = new LLVMContext();
        std::unique_ptr<Module> mod = parseIRFile(f, Err, *context);

        if (mod == nullptr) {
            llvm::outs() << "File " << f << " is not LLVM IR bitcode file.\n";
            continue;
        }

        Module *m = mod.release();
        for (auto it = m->begin(); it != m->end(); it++) {
            Function *F = &*it;
            if (!F->hasName()) {
                continue;
            }

            std::string functionName = F->getName().str();
            if (functionName == "") {
                continue;
            }

            // Skip if it is definition only.
            //if (F->isDeclaration()) {
            //    continue;
            //}

            llvm::outs() << "Analyzing function: " << functionName << "\n";

            DISubprogram *subprogram = F->getSubprogram();
            if (subprogram == nullptr) {
                llvm::dbgs() << "File " << f << " may not be built with debug option.\n";
                continue;
            }

            DIFile *file = subprogram->getFile();

            std::stringstream ss;
            ss << file->getDirectory().str() << "/" << file->getFilename().str() << "," << functionName << "," << subprogram->getLine() << std::endl;
            functions.push_back(ss.str());

            // Analyzing Basic Block
            for (auto &BB : *F) {
                std::string BBName = BB.hasName() ? BB.getName().str() : "<unnamed>";
                for (auto &Inst : BB) {
                    if (llvm::DILocation *Loc = Inst.getDebugLoc()) {
                        unsigned Line = Loc->getLine();
                        llvm::StringRef File = Loc->getFilename();
                        llvm::StringRef Directory = Loc->getDirectory();
                        std::stringstream ss;
                        ss << BBName << "," << Directory.str() << "/" << File.str() << "," << Line << std::endl;
                        if (!has_data(basicblocks, ss.str())) {
                            basicblocks.push_back(ss.str());
                        }
                    }
                }
            }
        }
    }

    if (sys::fs::create_directory(outputdir)) {
        llvm::errs() << "Failed to create directory %s\n", outputdir;
        exit(1);
    }

    write_to_file(functions, outputdir, "functionNames.txt");
    write_to_file(basicblocks, outputdir, "basicblocks.txt");

    return 0;
}

