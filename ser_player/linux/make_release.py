import subprocess
import shutil
import os
import sys


# Function to return list of libraries from exe or lib
def get_libs(file):
    lib_list = []
    ldd_out = subprocess.check_output(['ldd', file])
    ldd_out =  ldd_out.split('\n')
    for line in ldd_out:
        if line.find('Qt') != -1:
            split_line = line.split(' ');
            lib_list.append(os.path.abspath(split_line[2]))
    return lib_list


# Function to check if an item is in a list
def is_item_in_list(item, item_list):
    for i in item_list:
        if i == item:
            return 1;
    return 0;


# Function to remove and replace directory
def remove_and_replace_dir(d):
    if os.path.exists(d):
        shutil.rmtree(d)
    os.makedirs(d)


# Get base directory of destination bundle
base_dest_dir = os.path.dirname(os.path.realpath(__file__))
base_dest_dir = os.path.abspath(base_dest_dir + '/../../ser_player_linux/SER_Player')
bin_file_dir = os.path.dirname(os.path.realpath(__file__))
bin_file_dir = os.path.abspath(bin_file_dir + '/../../bin/')

# Required plugins - it was a bit of trial and error figuring out which plugins
# are actually required!
plugin_list = ['/imageformats/libqtiff.so', '/imageformats/libqjpeg.so']

# Remove and replace top directory to ensure the package is clean
remove_and_replace_dir(base_dest_dir)

# Copy executable to desination directory
dst_dir = base_dest_dir
shutil.copy2(bin_file_dir + '/SER_Player', dst_dir)
print 'bin_file_dir + /SER_Player: ' + bin_file_dir + '/SER_Player'

#Get list of libs from executable
lib_list1 = get_libs(bin_file_dir + '/SER_Player')


# Now get list of libs from first list of libs
for lib in lib_list1:
    temp_list = get_libs(lib)
    for item in temp_list:
        if is_item_in_list(item, lib_list1) == 0:
            # This lib is not in the original lib list
            lib_list1.append(item)

# Copy platforms plugin
src_plugins_path = os.path.dirname(lib_list1[0])
src_plugins_path = os.path.abspath(src_plugins_path + '/../plugins/')
dst_dir = os.path.abspath(base_dest_dir + '/platforms')
remove_and_replace_dir(dst_dir)

# Check for and extra libs required by libqxcb.so
new_libs = get_libs(src_plugins_path + '/platforms/libqxcb.so')
for item in new_libs:
    if is_item_in_list(item, lib_list1) == 0:
        # This lib is not in the original lib list
        lib_list1.append(item)

# Copy libqxcb library to destination directory
shutil.copy2(src_plugins_path + '/platforms/libqxcb.so', dst_dir)

# Copy other plugins
dst_dir = os.path.abspath(base_dest_dir + '/plugins')
remove_and_replace_dir(dst_dir)

for plugin in plugin_list:
    src_plugin_file = src_plugins_path + plugin
    dst_plugin_file = dst_dir + plugin
    print "src_plugin_file: " + src_plugin_file
    print "dst_plugin_file: " + dst_plugin_file

    # Check for and extra libs required by this plugin
    new_libs = get_libs(src_plugin_file)
    for item in new_libs:
        if is_item_in_list(item, lib_list1) == 0:
            # This lib is not in the original lib list
            lib_list1.append(item)
            
    # Create dest directory if it does not already exist
    dst_plugin_dir = os.path.dirname(dst_plugin_file)
    if not os.path.exists(dst_plugin_dir):
        os.makedirs(dst_plugin_dir)
    
    # Copy required plugins to directory
    shutil.copy2(src_plugin_file, dst_plugin_file)


# Copy required librarys to libs directory
dst_dir = base_dest_dir + "/libs"
remove_and_replace_dir(dst_dir)
for lib in lib_list1:
    shutil.copy2(lib, dst_dir)
    print "#" + lib

