# !/bin/bash
CUR_DIR="$(pwd)"
# Default format both cmake and clang
CLANG_FORMAT=1
CMAKE_FORMAT=1
FORMAT_DIR=${CUR_DIR}
usage() {
    echo "Code format tool, using for cmake and source code formating"
    echo "Options (default both cmake and clang is applied):"
    echo "  -l               : disable clang formating"
    echo "  -m               : disable cmake formating"
    echo "  -d               : directory to be formatted"
    exit;
}


while getopts "lmd:h" opt; do
    case $opt in
    l)
        CLANG_FORMAT=0
    ;;

    m)
        CMAKE_FORMAT=0
    ;;

    d)
        FORMAT_DIR=$OPTARG
        [ ! -d ${FORMAT_DIR} ] && FORMAT_DIR=${CURRENT_DIR}
    ;;

    h) usage ;;
    esac
done

    echo "Directory: ${FORMAT_DIR}"

# Clang format
if [ $CLANG_FORMAT = 1 ]; then
    if [ -n $(command -v clang-format) ] ; then
        :
    else
        echo "Install clang-format first"
        sudo apt install clang-format
    fi
    echo "-----------------------------------------------------------------------------"
    echo "                            Formating source code"
    echo "-----------------------------------------------------------------------------"
    find ${FORMAT_DIR} -type f -regex '.*\.\(cpp\|hpp\|h\|cc\|cxx\|c\)' ! -regex '.*build.*' ! -regex '.*out.*' -exec clang-format -style=file -i --verbose {} \;
    echo ""
fi

# Cmake format
if [ $CMAKE_FORMAT = 1 ]; then
    if  [ -n $(command -v cmake-format) ] ; then
        :
    else
        echo "Install cmake-format first"
        sudo apt install cmake-format
    fi

    echo "-----------------------------------------------------------------------------"
    echo "                            Formating cmake code"
    echo "-----------------------------------------------------------------------------"
    find ${FORMAT_DIR} -type f -regex '.*CMakeLists.txt' ! -regex '.*build.*' ! -regex '.*out.*' -exec sh -c 'cmake-format -o {} {} ; echo Formatting [1/1] {} ' \; 
    echo ""
fi