Table of Contents
=================

   * [Project description](#project-description)
   * [Dataset overview](#dataset-overview)
   * [Measurements conversion](#measurements-conversion)
   * [Extra hardware](#extra-hardware)
   * [Script overview](#dataset-overview)
   * [Classification results](#classification-results)

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
In folowing images I plotted two <b>randomly-chosen</b> files between the ones just described above:

<img src="https://github.com/papamauro/wlk_jog_arduino101/blob/master/plot.png" class="img-responsive">
<br />
<img src="https://github.com/papamauro/wlk_jog_arduino101/blob/master/plot2.png" class="img-responsive">

A deep analysis of these plots (and many others in the data-set of course) brought me to the following conclusions:
<ol type="1">
  <li>Patterns can be better recognized in y-axis;</li>
  <li>Sometimes patterns start after an initial range of worse recognizable samples;</li>
  <li>In this dataset undersampling and average operations are not adviceable, indeed 128 samples already cover at least one movement pattern in the plots;</li>
  <li>Neurons only fit 128 bytes each, but files provides thousands of samples and can be used to train multiple neurons.</li>
</ol>

# Measurements conversion
As already said, motion-sense dataset provides accelerometers samples in G units. This of course is not a problem but if someone wants to use the "learning part" of this script in order to recognize movements from the accelerometer of the arduino board then a conversion is needed. Indeed Arduino 101 provides accelerometer data in bytes (as default). To convert motion-sense from G units to bytes some further information are needed like: the model of the accelerometer chip used to get data and how this chip was configured.

This dataset is collected by using an Iphone 6s accelerometer so I had to find the related accelerometer chip datasheet. This datasheet is not available online but Iphone 6s should feature the same chip of Iphone 6. As reported <a href="https://www.chipworks.com/about-chipworks/overview/blog/comparing-invensense-and-bosch-accelerometers-found-iphone-6" rel="nofollow noreferrer">here</a>, Iphone 6 (just like the Iphone 6s) has two different chips but with high probability InvenSense MPU-6700 is the chip used here. Again, datasheet is not available for the MPU-6700, however its specifications should be similar to those published by InvenSense for the <a href="http://www.invensense.com/wp-content/uploads/2015/02/MPU-6500-Datasheet2.pdf">MPU-6500</a>.

In order to convert G units to bytes we need to multiply G to the LSB sensitivity level of the chip used for collecting data. Now, MPU-6500 provides four different sensitivity ranges:

* ±2g: 16384 LSB/g
* ±4g: 8192 LSB/g
* ±8g: 4096 LSB/g
* ±16g: 2048 LSB/g

After asking to dataset authors <a href="https://github.com/mmalekzadeh/motion-sense/issues/1">which was the sensivity level used (±8g: 4096 LSB/g)</a>, I was finally able to properly convert gravity units in bytes by multiplying for 4096 constant.

# Extra hardware
To run the script an sd card reader is needed. Information about how to set it up can be found <a href="https://www.monocilindro.com/2016/04/17/arduinogenuino-101-and-sd-card-module/">here</a>. The sd-card must contain the folders <b>B_Accelerometer_data/wlk_7</b> and <b>B_Accelerometer_data/wlk_9</b> that can be downloaded <a href="https://github.com/mmalekzadeh/motion-sense/tree/master/data">here</a>.

# Script overview
For a breaf introduction to Arduino 101 board and CuriePME library please refer to Intel <a href="https://github.com/intel/Intel-Pattern-Matching-Technology/tree/master/examples/DrawingInTheAir">DrawingInTheAir</a> example.
This script opens the 48 (24+24) files of the dataset three times, for a total of 144 file accesses. 128 times files are opened to train board neurons, the rest is used to classify movements. Opening file order is randomly-chosen. <b>Of course, when a file is opened multiple times (for both training or classifying), the scripts always picks a different range of samples</b>. As said before I chose to discard x and z axes to work only with y axis. y values are then converted in bytes and mapped to a single byte value, so that every y-sample is related to a single byte and a neuron can contain (at least) a full pattern inside (see <i>Dataset overview</i> section above). After some approaches tried this turned out to be the most efficient one. Mapping function needs to know the dataset max and min value which of course are ±32768 (±8g * 4096).

```cpp
void readFromFile(int index, String fileName, byte vector[])
{
    double y;
    unsigned int skip;
   
    File myFile;
    Serial.print(fileName);
    Serial.print("\n");
    myFile=SD.open(fileName.c_str());

    if (myFile) {

      if(fileName.charAt(0)==106)
        skip = 500 + ((3-jog[index])*200);
      else
        skip = 500 + ((3-wlk[index])*200);
        
      for (unsigned int j = 0; j < skip && myFile.available(); j++)
        myFile.readStringUntil('\n'); //Skip lines

      for (unsigned int i = 0; i < vectorNumBytes && myFile.available(); i++) {
        
        myFile.readStringUntil(',');
        myFile.readStringUntil(',');
        String yString = myFile.readStringUntil(',');
        myFile.readStringUntil('\n');
        
        y = atof(yString.c_str());
        y = y*4096;
        
        vector[i] = (byte) map(y, DataSetMin, DataSetMax, 0, 255);
        }
    }
    myFile.close();
}

```

# Classification results
As already said files are opened by the script in random oder bringing us different results every time. However, after running the script several times I'm confindent to say that it is able to recognize walking and joggin movement with an accuracy of %87,5. In the following table an example result of the script run is shown:

| File Name        | Kind of movement | Results               |
| ---------------- | ---------------- | --------------------- |  
| jog_9/sub_4.csv  | Jogging          | Success               |  
| jog_9/sub_4.csv  | Jogging          | Success               |  
| jog_9/sub_9.csv  | Jogging          | Not able to recognize |  
| jog_9/sub_9.csv  | Jogging          | Not able to recognize |  
| jog_9/sub_13.csv | Jogging          | Success               |
| jog_9/sub_13.csv | Jogging          | Success               |
| jog_9/sub_14.csv | Jogging          | Success               |
| jog_9/sub_14.csv | Jogging          | Success               |
| wlk_7/sub_2.csv  | Walking          | Success               |
| wlk_7/sub_3.csv  | Walking          | Success               |
| wlk_7/sub_6.csv  | Walking          | Success               |
| wlk_7/sub_14.csv | Walking          | Success               |
| wlk_7/sub_16.csv | Walking          | Success               |
| wlk_7/sub_16.csv | Walking          | Success               |
| wlk_7/sub_17.csv | Walking          | Success               |
| wlk_7/sub_17.csv | Walking          | Success               |
