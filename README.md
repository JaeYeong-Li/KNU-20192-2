# Audience response analysis system based on emotion recognition algorithms
⌨ team project for [2019-2] KNU capstone project2<br>
🖱 team mentor justinjoy
🎥 Demo Video: https://www.youtube.com/watch?v=6N2loif-hoY&feature=youtu.be

## Introduction
**A system that detects and analyzes people's faces in videos using Amazon Rekognition API**
**Statistical analysis results to quantify audience responses**

## System Architecture
![시스템아키텍처](https://user-images.githubusercontent.com/42104898/103221845-f3c1fa80-4966-11eb-8ac4-b71e7136cd74.png)

## Results
#### facial expression-based appraisal system
1. Get snapshot while taking a video(GStreamer/cv2)
2. save snapshot in local to web storage
3. emotion analysis of snpashot
4. process the anlaysis results into "emotion formula"
5. Visualize Results

#### "emotion formula"
we formulate a formula with real video and people's reaction
