#!/bin/zsh

# Make sure that we are in the root of the project
cd $(dirname $0)
cd ..

# Set passed tests counter
passed=0
sum_compressed_size=0
extension=exe
if [[ $extension == "exe" ]]; then
    execution_layer=wine
fi
encoder=./build/encoder.${extension}
decoder=./build/decoder.${extension}

echo "Test: encoder and decoder available"
if [ -e ${encoder} ] && [ -e ${decoder} ]; then
    echo "  [TEST PASSED]: both utils are available"
    ((passed+=1))
else
    echo "  [TEST FAILED]: some of utils are missing"
fi

for file in $(ls test/calgarycorpus); do;
    # Clear the previous iterations if present
    rm -f ./test/${file} ./test/${file}.ppmd ./test/${file}_original

    # Copy verified instance of a test file
    cp ./test/calgarycorpus/${file} ./test/${file}

    # Run encoder
    ${execution_layer} ${encoder} ./test/${file} &>/dev/null

    # Check encoded file has appropriate size
    echo "Test: encoded file has normal size"
    if [ -s ./test/${file}.ppmd ]; then
        echo "  [TEST PASSED]: encoded file created successfully"
        ((passed+=1))
    else
        echo "  [TEST FAILED]: encoded file has size zero"
    fi

    # Rename original file to make sure it doesn't affect the process
    mv ./test/${file} ./test/${file}_original

    # Run decoder
    ${execution_layer} ${decoder} ./test/${file}.ppmd

    # Compare results
    echo "Test: decoder gives identical file as origin"
    cmp --silent ./test/${file} ./test/${file}_original
    if [ $? -eq 0 ]; then
        echo "  [TEST PASSED]: encoded and decoded files are identical"
        ((passed+=1))
    else
        echo "  [TEST FAILED]: encoded and decoded files are different"
    fi

    # Show additional information
    echo && echo "Passed $passed/29 tests"
    
    original_size=$(wc -c <./test/${file})
    compressed_size=$(wc -c <./test/${file}.ppmd)
    ((sum_compressed_size+=${compressed_size}))
    cr=$(( 8.0 * ${compressed_size} / ${original_size} ))

    echo "Code ratio = $cr" && echo
    
    # Clear the remainings
    rm test/${file} test/${file}.ppmd test/${file}_original
done

echo "Total compressed size= $sum_compressed_size"