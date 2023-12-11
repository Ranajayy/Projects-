#############################################################################################
# command to run the program and show the output: python3 vehicle_detection_main.py imshow use it on the terminal 
#############################################################################################

# Imports
import numpy as np
import serial
import os
import six.moves.urllib as urllib
import sys
import tarfile
import tensorflow as tf
import zipfile
import cv2
import numpy as np
import csv
import time
from packaging import version
from datetime import datetime
from collections import defaultdict
from io import StringIO
from PIL import Image

# Object detection imports
from utils import label_map_util
from utils import visualization_utils as vis_util

# initialize .csv
ser = serial.Serial('/dev/ttyACM0', 9600)
with open('traffic_measurement.csv', 'w') as f:
    writer = csv.writer(f)
    csv_line = \
        'Vehicle Type/Size, Vehicle Color, Vehicle Movement Direction, Vehicle Speed (km/h)'
    writer.writerows([csv_line.split(',')])

# input video you can atavch pi cam by commenting out source video and putting 0 in cv2.videoCapture(source_video )
source_video = '/home/ranajayy/Desktop/N.mp4'
cap = cv2.VideoCapture(source_video)
# Variables
height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
fps = int(cap.get(cv2.CAP_PROP_FPS))

total_passed_vehicle = 0
time=time.time()

# By default I use an "SSD with Mobilenet" model here. See the detection model zoo (https://github.com/tensorflow/models/blob/master/research/object_detection/g3doc/detection_model_zoo.md) for a list of other models that can be run out-of-the-box with varying speeds and accuracies.
# What model to download.
MODEL_NAME = 'ssd_mobilenet_v1_coco_2018_01_28'
MODEL_FILE = MODEL_NAME + '.tar.gz'
DOWNLOAD_BASE = \
    'http://download.tensorflow.org/models/object_detection/'

# Path to frozen detection graph. This is the actual model that is used for the object detection.
PATH_TO_CKPT = MODEL_NAME + '/frozen_inference_graph.pb'

# List of the strings that is used to add correct label for each box.
PATH_TO_LABELS = os.path.join('data', 'mscoco_label_map.pbtxt')

NUM_CLASSES =100

# Download Model
# uncomment if you have not download the model yet
# Load a (frozen) Tensorflow model into memory.
detection_graph = tf.Graph()
with detection_graph.as_default():
    #od_graph_def = tf.GraphDef()
    #with tf.gfile.GFile(PATH_TO_CKPT, 'rb') as fid:
    od_graph_def = tf.compat.v1.GraphDef() # use this line to run it with TensorFlow version 2.x
    with tf.compat.v2.io.gfile.GFile(PATH_TO_CKPT, 'rb') as fid: # use this line to run it with TensorFlow version 2.x
        serialized_graph = fid.read()
        od_graph_def.ParseFromString(serialized_graph)
        tf.import_graph_def(od_graph_def, name='')

# Loading label map
# Label maps map indices to category names, so that when our convolution network predicts 5, we know that this corresponds to airplane. Here I use internal utility functions, but anything that returns a dictionary mapping integers to appropriate string labels would be fine
label_map = label_map_util.load_labelmap(PATH_TO_LABELS)
categories = label_map_util.convert_label_map_to_categories(label_map,
        max_num_classes=NUM_CLASSES, use_display_name=True)
category_index = label_map_util.create_category_index(categories)


# Helper code
def load_image_into_numpy_array(image):
    (im_width, im_height) = image.size
    return np.array(image.getdata()).reshape((im_height, im_width,
            3)).astype(np.uint8)


# Detection
def object_detection_function(command):
    ma_pos=0
    m_pos=0
    m_flag=0
    count=0
    car_flag=0
    c=0
   
    if(command=="imwrite"):
        fourcc = cv2.VideoWriter_fourcc(*'XVID')
        output_movie = cv2.VideoWriter(source_video.split(".")[0]+'_output.avi', fourcc, fps, (height, widht))

    with detection_graph.as_default():
        #with tf.Session(graph=detection_graph) as sess:
        with tf.compat.v1.Session(graph=detection_graph) as sess: # use this line to run it with TensorFlow version 2.x

            # Definite input and output Tensors for detection_graph
            image_tensor = detection_graph.get_tensor_by_name('image_tensor:0')

            # Each box represents a part of the image where a particular object was detected.
            detection_boxes = detection_graph.get_tensor_by_name('detection_boxes:0')

            # Each score represent how level of confidence for each of the objects.
            # Score is shown on the result image, together with the class label.
            detection_scores = detection_graph.get_tensor_by_name('detection_scores:0')
            detection_classes = detection_graph.get_tensor_by_name('detection_classes:0')
            num_detections = detection_graph.get_tensor_by_name('num_detections:0')

            # for all the frames that are extracted from input video
            while cap.isOpened():
                (ret, frame) = cap.read()

                if not ret:
                    print ('end of the video file...')
                    break

                input_frame = frame

                # Expand dimensions since the model expects images to have shape: [1, None, None, 3]
                image_np_expanded = np.expand_dims(input_frame, axis=0)

                # Actual detection.
                (boxes, scores, classes, num) = \
                    sess.run([detection_boxes, detection_scores,
                             detection_classes, num_detections],
                             feed_dict={image_tensor: image_np_expanded})
                
                
                # Visualization of the results of a detection 
                (counter, csv_line) = \
                    vis_util.visualize_boxes_and_labels_on_image_array(
                    cap.get(1),
                    input_frame,
                    np.squeeze(boxes),
                    np.squeeze(classes).astype(np.int32),
                    np.squeeze(scores),
                    category_index,
                    use_normalized_coordinates=True,
                    line_thickness=4,
                    )
                
                final_score=np.squeeze(scores)
                for i in range(100):
                    if final_score[i]>0.5:
                        detected_class=int(classes[0][i])
                        #print(detected_class)
                        if(detected_class == 8)or (detected_class==3):
                            print("vehicle found")
                            box=np.squeeze(boxes)
                            xmin=(int(box[0][1]*width))
                            ymin=(int(box[0][0]*height))
                            ymax=(int(box[0][2]*height))
                            xmax=(int(box[0][3]*width))
                            #print(xmin,xmax)
                            if m_flag==0:
                                a=xmin
                                previous_a=xmin
                                current_a=xmin
                                previous_xm=xmax
                                current_xm=xmax
                                m_flag=5
                                
                            if (current_a > previous_a):
                                print("car going right")  
                                ser.write("right\n".encode('utf-8'))
                                
                            elif (current_a < previous_a):
                                print("car going left")
                                ser.write("left\n".encode('utf-8'))
                                c=c+1
                                print(c)
                                
                            count=count+1
                            current_a=xmin
                            current_xm=xmax
                            
                            if(count==40):
                                img_name="car_image_{}.png".format(time)
                                cv2.imwrite(img_name,input_frame)    
                            
                        else:
                            ser.write("right\n".encode('utf-8'))
                 
                    
                    
                    
                if(command=="imshow"):
                    cv2.imshow('vehicle detection', input_frame)
                    if cv2.waitKey(1) & 0xFF == ord('q'):
                        break
            cv2.destroyAllWindows()
   

import argparse
# Parse command line arguments
parser = argparse.ArgumentParser(description='Vehicle Detection TensorFlow.')
parser.add_argument("command",
                    metavar="<command>",
                    help="'imshow' or 'imwrite'")
args = parser.parse_args()
object_detection_function(args.command)		
