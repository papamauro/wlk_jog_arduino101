Table of Contents
=================

   * [Project description](#project-description)
   * [Dataset overview](#dataset-overview)
   * [Classifying](#classifying)

# Project description
This project aims to train the KNN chip of Genuino/Arduino 101 to recognize walking ang jogging movements from a MIT-licensed accelerometer data-set described at:
* **Mohammad Malekzadeh, Richard G. Clegg, Andrea Cavallaro, and Hamed Haddadi. 2018. [Protecting Sensory Data against Sensitive Inferences](https://arxiv.org/abs/1802.07802). In W-P2DS’18: 1st Workshop on Privacy by Design in Distributed Systems , April 23–26, 2018, Porto, Portugal. ACM, New York, NY, USA, 6 pages. https: //doi.org/10.1145/3195258.3195260**

Further specifications about the <b>motion-sense</b> data-set used can be found at <a href="https://github.com/mmalekzadeh/motion-sense">here</a>. This project is heavily based on Intel <a href="https://github.com/intel/Intel-Pattern-Matching-Technology/tree/master/examples/DrawingInTheAir">DrawingInTheAir</a> example.

# Dataset overview
Since the board provides only 128 neurons of 128 byte each, I had to analyze the dataset to figure out which could be the best data optimization strategy. First of all, <b>motion-sense</b> follows this folder-structure:
<ol type="A">
  <li>DeviceMotion_data</li>
  <li>Accelerometer_data
    <ul>
      <li>dws_1</li>
      <li>...</li>
      <li>jog_9</li>
      <li>wlk_7</li>
    </ul>
  </li>
  <li>Gyroscope_data</li>
</ol>
DeviceMotion_data provides both accelerometer and gyroscope information. Since I wanted to work with accelerometer data i focused on Accelerometer_data folder. Of course we are going to use only files in <i>jog_9</i> and <i>wlk_7</i> sub-folders because we are interested in classifying joggin and walking movements. Each folder contains 24 csv files of thousands of samples. Every file is related to a different dataset participant.
In folowing image we plot a randomly-chosen file between the ones just described above:
