import glob
import os
import subprocess
import shutil
import platform

script_dir = os.path.dirname(os.path.realpath(__file__))

def compile_shader (file):
    print ("compile shader: " + file)

    system = platform.system()
    if system == 'Windows':
        dest = script_dir + '/projects/'
        target_macro = 'TARGET_WIN32=1'
    elif system == 'Darwin':
        dest = script_dir + '/projects/Debug/'
        target_macro = 'TARGET_MACOSX=1'
    elif system == 'Linux':
        dest = script_dir + '/projects/'
        target_macro = 'TARGET_LINUX=1'
    else:
        raise NotImplementedError

    subprocess.call (['glslangvalidator', '--define-macro', target_macro, '--target-env', 'vulkan1.0', '-V', file, '-o', file + '.spv'])

    os.makedirs (dest, exist_ok = True)

    print (" copying: " + file + '.spv')
    print (" to: " + dest)

    shutil.copy(file + '.spv', dest)
    os.remove(file + '.spv')


print (script_dir)

os.chdir(script_dir + "/res")

for file in glob.glob("*.vert"):
    compile_shader(file)

for file in glob.glob("*.frag"):
    compile_shader(file)

for file in glob.glob("*.comp"):
    compile_shader(file)

os.chdir(script_dir + "/examples/")
examples = next(os.walk('.'))[1]
for ex in examples:
    os.chdir(script_dir + "/examples/" + ex)
    for file in glob.glob("*.comp"):
        compile_shader(file)