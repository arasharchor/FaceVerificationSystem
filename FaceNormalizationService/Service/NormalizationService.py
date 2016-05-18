# -*- coding: utf-8 -*-
"""
@author: gleotescu
"""

from ctypes import *

libraryPath = "./FaceNormalizationBuild/x64/Release/FaceNormalizationBuild.dll"
modelPath   = "./models/landmarks_points.dat"
outputPath  = 'test_output' 

faceNormLib = CDLL(libraryPath)

#init function 
init = faceNormLib.init
init.argtypes = [c_char_p]
init.restype  = c_int

#normalize function
normalize = faceNormLib.normalizeImage
normalize.argTypes = [c_char_p, c_char_p, c_int]
normalize.restype  = c_char_p

cModelPath = c_char_p(modelPath)

res = init(cModelPath)
if res != 0:
    print 'failed while trying to init face normalization tool'
else:
    print 'face normalization tool loaded succesfully'
    
images = ['image_1.jpg', 'test_input/image_2.jpg', 'test_input/image_3.jpg', 'test_input/image_4.jpg']
outPath  = 'test_output'
cOutPath = c_char_p(outputPath)
cMode    = c_int(1)

for img in images:
    cImg = c_char_p(img)
    res = normalize(cImg, cOutPath, cMode)
    print res    