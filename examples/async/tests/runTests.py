#!/usr/bin/env python3
import os, glob, sys, subprocess

translationTMPFilename = "/tmp/"+str(os.getpid())+".slugsin"

for filename in glob.glob("*.structuredslugs"):
    sys.stdout.write(filename+": ")
    sys.stdout.flush()
    
    # Compile
    cp = subprocess.Popen("../../../tools/StructuredSlugsParser/compiler.py "+filename, shell=True, bufsize=1048768, stdout=subprocess.PIPE, close_fds=True)
    with open(translationTMPFilename,"wb") as outFile:
        for a in cp.stdout.readlines():
            outFile.write(a)
    cp.wait()
    assert cp.returncode==0
    
    # Run
    rp = subprocess.Popen("../../../src/slugs --asyncPartitionedTransitions "+translationTMPFilename, shell=True, bufsize=1048768, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
    realizable = None
    for a in rp.stdout.readlines():    
        a = a.decode("utf-8").strip()
        if a.find("RESULT: Specification is realizable.") != -1:
            realizable = True
        elif a.find("RESULT: Specification is unrealizable.") != -1:
            realizable = False
    rp.wait()        
    assert rp.returncode==0
    
    # Check if alright
    if filename.startswith("realizable_"):
        assert realizable==True
        print("ok.")
    elif filename.startswith("unrealizable_"):
        assert realizable==False
        print("ok.")
    else:
        print("?????? Correct result not known")
