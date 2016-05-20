# -*- coding: utf-8 -*-
"""
@author: gleotescu
"""

import tempfile
import sys
import os
import ctypes
import base64
from flask import Flask, request, jsonify
from PIL import Image
from io import BytesIO

libraryPath = "./FaceNormalizationBuild/x64/Release/FaceNormalizationBuild.dll"
modelPath   = "./models/landmarks_points.dat"
outputPath  = 'test_output' 

faceNormLib = ctypes.CDLL(libraryPath)

#init function 
init = faceNormLib.init
init.argtypes = [ctypes.c_char_p]
init.restype  = ctypes.c_int

#normalize function
normalize = faceNormLib.normalizeImage
normalize.argTypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
normalize.restype  = ctypes.c_char_p

cModelPath = ctypes.c_char_p(modelPath)

res = init(cModelPath)
if res != 0:
    print 'failed while trying to init face normalization tool'
else:
    print 'face normalization tool loaded succesfully'

c2DMode = ctypes.c_int(1)
c3DMode = ctypes.c_int(0)

# web service 
app = Flask(__name__)

@app.route('/')
def init():
    return 'face normalization service'

@app.route('/normalize2D/', methods=['POST'])
def handleRequest():
    try:
        json  = request.get_json()
        imgData = json['input'][0]['data']
        filename = 'original.jpeg' #json['input'][0]['filename']
                
        #create directory to store current image
        imgDir  = tempfile.mkdtemp(dir='temp')
        imgPath = os.path.join(imgDir, filename)

        im = Image.open(BytesIO(base64.b64decode(imgData)))
        im.save(imgPath, 'JPEG')
        
        cImgDir  = ctypes.c_char_p(imgDir)
        cImgPath = ctypes.c_char_p(imgPath)
        
        facesStr = normalize(cImgPath, cImgDir, c2DMode)
        if len(facesStr) > 0:        
            faces = facesStr.split(';')
        else:
            faces = []
            
        facesData = []        
        for face in faces:
            print 'loading face', face
            with open(face, "rb") as image_file:
                faceData = base64.b64encode(image_file.read())
                facesData.append(faceData)
        
        return jsonify(faces=facesData, originalImage=imgData)
    
    except:
        error = "Unexpected error:", sys.exc_info()[0]
        print error
        return jsonify(faces=error)



if __name__ == '__main__':
    app.run(host= '0.0.0.0')
    app.run()    