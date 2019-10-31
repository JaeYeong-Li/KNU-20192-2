#Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#PDX-License-Identifier: MIT-0 (For details, see https://github.com/awsdocs/amazon-rekognition-developer-guide/blob/master/LICENSE-SAMPLECODE.)

import boto3
import json
from operator import itemgetter

if __name__ == "__main__":
    photo='angry.jpg'
    bucket='jeongjiu'
    client=boto3.client('rekognition')
    first = 0
    firstnum = 0
    response = client.detect_faces(Image={'S3Object':{'Bucket':bucket,'Name':photo}},Attributes=['ALL'])

    print('Detected faces for ' + photo)    
    for faceDetail in response['FaceDetails']:
        print('The detected face is between ' + str(faceDetail['AgeRange']['Low']) 
              + ' and ' + str(faceDetail['AgeRange']['High']) + ' years old')
        print('Here are the other attributes:')
        #print(str(faceDetail['Emotions'][0]))
        #print(len(faceDetail['Emotions']))
        data = sorted(faceDetail['Emotions'], key=itemgetter('Confidence'), reverse=True)
        for d in data:
            print(d)
        
        if (data[0]['Type']=='HAPPY'):
            print('Positive')
        if (data[0]['Type']=='DISGUSTED'):
            print('Negative')
        elif (data[0]['Type']=='SAD'):
            print('Negative')
        elif(data[0]['Type']=='DISGUSTED' and data[0]['Type']=='ANGRY'):
            print('Negative')
        elif(data[0]['Type']=='ANGRY' and data[0]['Type']=='DISGUSTED'):
            print('Negative')
        elif(data[0]['Type']=='CALM' and data[0]['Type']=='SAD'):
            print('Negative')
        elif(data[0]['Type']=='SAD' and data[0]['Type']=='CLAM'):
            print('Negative')
        elif (data[0]['Type']=='ANGRY' and data[0]['Confidence'] >=98):
            print('Negative')
        else:
            print('Normal')


        #for i in range(len(faceDetail['Emotions'])):
          #  if(first < faceDetail['Emotions'][i]['Confidence']):
           #     firstnum = i
                

        #print(json.dumps(faceDetail['Emotions'], indent=4, sort_keys=True))

