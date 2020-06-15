import os
import subprocess
import platform

script_dir = os.path.dirname (os.path.realpath (__file__)) 
build_dir = os.path.join (script_dir, 'cmake')
projects_dir = os.path.join (script_dir, 'projects')

if not os.path.isdir (projects_dir):
    os.mkdir (projects_dir)

system = platform.system()

if system == 'Windows':
    subprocess.call (['cmake', '-G', 'Visual Studio 16 2019', build_dir], cwd = projects_dir)
elif system == 'Darwin':
    subprocess.call (['cmake', '-G', 'Xcode', build_dir], cwd = projects_dir)
else:
    subprocess.call (['cmake', build_dir], cwd = projects_dir)