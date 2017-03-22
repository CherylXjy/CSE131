#! /bin/sh
echo

color() {
  c="$1"
  shift
  echo -e "\e[${c}m${@}\e[0m"
}

#BUILD GLC
color 35 "Building ./glc..."
make
make_status="$?"

if [ "$make_status" != 0 ]; then
    color 31 "Build failed."
    echo $make_out
    exit 1 
fi

[ -x glc ] || { echo "Error: glc not executable"; exit 1; }
[ -x p4exe ] || { echo "Error: p4exe not found or executable"; exit 1; }

#Static vars
# ROOT="public_samples/"
ROOT="cse131_testcases/"
# ROOT = "samples/"

#Counters
numPass=0
numTest=0

#Command line args
showPass=true
showFail=true
onlyTestCase=""

while true; do
    case "$1" in
        "--hidepass") showPass=false; shift;;
        "--hidefail") showFail=false; shift;;
        "--onlytest")     shift; onlyTestCase=$1; shift;;
        *)            break;;
    esac
done
LIST=
if [ "$#" = "0" ]; then
    LIST="ls $ROOT*.glsl"
else
    for test in "$@"; do
        LIST="$LIST $test"
    done
fi

for file in $ROOT*.glsl; do
    fileNoExt="${file%.glsl}"

    fileName="${fileNoExt:${#ROOT}}"
    if [ "$onlyTestCase" != "" ] && [ "$onlyTestCase" != "$fileName" ]; then
        continue
    fi

    let numTest+=1

    goldenSegfault=false
    userSegfault=false

    (eval ./p4exe < $file > deleteMeGolden 2> /dev/null) 2> /dev/null || goldenSegfault=true
    (eval "./glc < $file > deleteMeUser 2> deleteMeGLCErr") 2> /dev/null || userSegfault=true

    #Check for segfault
    if [ "$goldenSegfault" = true ] || [ "$userSegfault" == true ]; then
        if [ "$showFail" = true ]; then
            color 31 FAILED $fileName
            cat deleteMeGLCErr
            if [ "$goldenSegfault" = true ]; then
                echo "segfault in p4exe"
            fi
            if [ "$userSegfault" == true ]; then
                echo "segfault in glc"
            fi
        fi
        continue
    fi

    mv deleteMeGolden $fileNoExt.bc
    (eval ./gli $fileNoExt.bc > deleteMeGolden 2> /dev/null) 2> /dev/null

    mv deleteMeUser $fileNoExt.bc
    (eval ./gli $fileNoExt.bc > deleteMeUser) 2> /dev/null
    rm $fileNoExt.bc

    failed=false
    cmp --silent deleteMeUser deleteMeGolden || failed=true
    if [ "$failed" = true ]; then
        if [ "$showFail" = true ]; then
            color 31 FAILED $fileName
            cat deleteMeGLCErr
            diff -wu --label "glc" deleteMeUser --label "p4exe" deleteMeGolden
        fi
    else
        let numPass+=1
        if [ "$showPass" = true ]; then
            color 32 PASSED $fileName
        fi
    fi
done
echo
color 35 "TOTAL: $numPass/$numTest"
rm deleteMe*