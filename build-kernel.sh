#!/bin/bash

if [ $# != 2 ]; then
    echo "[*]usage $0 <linux kernel directory> <kernel build artifact directory>"
    exit 1
fi

KERNEL_DIR=$1
KERNEL_BUILD_ARTIFACT_DIR=$2
CFG_FILES_OUTPUT_DIR="${KERNEL_BUILD_ARTIFACT_DIR}/dot-files"

if [ -d "${KERNEL_BUILD_ARTIFACT_DIR}" ];then
    rm -fr "${KERNEL_BUILD_ARTIFACT_DIR}"
fi
mkdir -p "${CFG_FILES_OUTPUT_DIR}"

SCRIPT_PATH=$(dirname "$(readlink -f "$BASH_SOURCE")")
MYCC="${SCRIPT_PATH}/KernelFuzzCC"

cat <<'EOF' > "${MYCC}"
#!/bin/sh

CLANG=$(which clang)

if [ ! -e $CLANG ]; then
    exit
fi

input=""
if [ ! -t 0 ]; then
    input=$(cat)
fi

OFILE=`echo $* | sed -e 's/^.* \(.*\.o\) .*$/\\1/'`
if [ "x$OFILE" != x -a "$OFILE" != "$*" ] ; then
    if [ -z "${input}" ]; then
        $CLANG -emit-llvm -g "$@" >/dev/null 2>&1 > /dev/null
    else
        echo "${input}" | $CLANG -emit-llvm -g "$@" >/dev/null 2>&1 > /dev/null
    fi
    if [ -f "$OFILE" ] ; then
        BCFILE=`echo $OFILE | sed -e 's/o$/bc/'`
        #file $OFILE | grep -q "LLVM IR bitcode" && mv $OFILE $BCFILE || true
        if [ `file $OFILE | grep -c "LLVM IR bitcode"` -eq 1 ]; then
            mv $OFILE $BCFILE
        else
            touch $BCFILE
        fi
    fi
fi

if [ -z "${input}" ]; then
    exec $CLANG "$@"
else
    echo "$input" | exec $CLANG "$@"
fi  
EOF

chmod 755 "${MYCC}"

cd "${KERNEL_DIR}"

orig_cfg_files=$(find . -name "*.cfg")

make LLVM=1 CC="${MYCC}" clean
make LLVM=1 CC="${MYCC}" mrproper

make LLVM=1 CC="${MYCC}" O="${KERNEL_BUILD_ARTIFACT_DIR}" defconfig
make LLVM=1 CC="${MYCC}" O="${KERNEL_BUILD_ARTIFACT_DIR}" kvm_guest.config

./scripts/config --file "${KERNEL_BUILD_ARTIFACT_DIR}/.config" -e KCOV
./scripts/config --file "${KERNEL_BUILD_ARTIFACT_DIR}/.config" -e DEBUG_INFO_DWARF4 
./scripts/config --file "${KERNEL_BUILD_ARTIFACT_DIR}/.config" -e KASAN
./scripts/config --file "${KERNEL_BUILD_ARTIFACT_DIR}/.config" -e KASAN_INLINE
./scripts/config --file "${KERNEL_BUILD_ARTIFACT_DIR}/.config" -e CONFIGFS_FS
./scripts/config --file "${KERNEL_BUILD_ARTIFACT_DIR}/.config" -e SECURITYFS
./scripts/config --file "${KERNEL_BUILD_ARTIFACT_DIR}/.config" -e CMDLINE_BOOL
./scripts/config --file "${KERNEL_BUILD_ARTIFACT_DIR}/.config" --set-val CMDLINE "net.ifnames=0"

make LLVM=1 CC="${MYCC}" O="${KERNEL_BUILD_ARTIFACT_DIR}" olddefconfig

echo "[+]$(date) : Start build"
make LLVM=1 CC="${MYCC}" O="${KERNEL_BUILD_ARTIFACT_DIR}" -j$(nproc)
echo "[+]$(date) : End build"

export KERNEL_BUILD_ARTIFACT_DIR CFG_FILES_OUTPUT_DIR

echo "[+]$(date) : Start creating cfg files"
dirs=$(find "${KERNEL_BUILD_ARTIFACT_DIR}" -type d)

echo "${dirs}" | xargs -n 1 -P 4 bash -c '
    d="$0"
    common_path="${d}"

    while [[ "${KERNEL_BUILD_ARTIFACT_DIR#$common_path}" == "${KERNEL_BUILD_ARTIFACT_DIR}" ]]; do
        common_path="${common_path%/*}"
    done

    suffix="${KERNEL_BUILD_ARTIFACT_DIR#$common_path}"
    workdir="${d#$common_path}"
    workdir=$(echo ${CFG_FILES_OUTPUT_DIR}${suffix}${workdir} | sed '"'"'s://:/:g'"'"')
    
    mkdir -p "${workdir}"

    cd "${workdir}"

    dotfiles=$(find "${d}" -maxdepth 1 -name "*.bc")
    for df in ${dotfiles}; do
        echo "Analyzing: ${df}"
        opt -passes=dot-callgraph,dot-cfg ${df}
    done
' 

echo "[+]$(date) : End creating cfg files"

