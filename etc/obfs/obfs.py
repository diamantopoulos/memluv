"""
Obfuscating MemLuv source code - shame on me 
"""
import os
import glob
import fnmatch
import shutil
import subprocess
import sys

print ("INFO: --------------------- \nINFO: Obfuscating MemLuv source code \nINFO: ---------------------")

 
def copyFile(src, dest):
    try:
        shutil.copy(src, dest)
    # eg. src and dest are the same file
    except shutil.Error as e:
        print('Error: %s' % e)
    # eg. source or destination doesn't exist
    except IOError as e:
        print('Error: %s' % e.strerror)

obfuscator_bin="../../contrib/Stunnix-CXX-Obfus-4.1-Linux-trial/bin/cxx-obfus"

print ("*" * 20)

obfs_path="../../src_obfs/"
for root, dirs, files in os.walk("../../src/"):
    #print ("dirs is", dirs)
    #print ("files is", files)
    
    if ((not fnmatch.fnmatch(root, '*/hls/prj*')) and (not fnmatch.fnmatch(root, '*/.simvision*')) ):
        cur_dir=os.path.basename(root)
        cur_dir=obfs_path+cur_dir
        print ("INFO: cur_dir is",cur_dir)
        print ("INFO: root is", root)
        if not os.path.exists(cur_dir):
            print ("INFO: Creating directory ",cur_dir)
            os.makedirs(cur_dir)
        for file in os.listdir(root):
            if file.endswith("CMakeLists.txt"):
                src=root+"/"+file
                dst=cur_dir+"/"+file
                print ("INFO: Copying file ",src, "to file ", dst) 
                copyFile(src, dst)
            elif ((file.endswith(".c")) or (file.endswith(".h"))):
                src=root+"/"+file
                dst=cur_dir+"/"+file
                print ("INFO: Obfuscating file ",src, "to file ", dst) 
                log = open(dst, "w")
                sys.stdout.flush()
                process = subprocess.Popen([obfuscator_bin, '-i', 'none', '--jam', '1', src], shell=False, stdout=log)
                process.wait()
            else:
                print("INFO: Skipping file", file)
