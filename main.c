/**
 * @file main.c
 *
 * @mainpage MNIST 1-Layer Neural Network
 *
 * @brief Main characteristics: Only 1 layer (= input layer), no hidden layer.  Feed-forward only.
 * No Sigmoid activation function. No back propagation.\n
 *
 * @details Learning is achieved simply by incrementally updating the connection weights based on the comparison
 * with the desired target output (supervised learning).\n
 *
 * Its performance (success rate) of 85% is far off the state-of-the-art techniques (surprise, surprise) 
 * but close the Yann Lecun's 88% when using only a linear classifier.
 *
 * @see [Simple 1-Layer Neural Network for MNIST Handwriting Recognition](http://mmlind.github.io/Simple_1-Layer_Neural_Network_for_MNIST_Handwriting_Recognition/)
 * @see http://yann.lecun.com/exdb/mnist/
 * @version [Github Project Page](http://github.com/mmlind/mnist-1lnn/)
 * @author [Matt Lind](http://mmlind.github.io)
 * @date July 2015
 *
 */
 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "screen.h"
#include "mnist-utils.h"
#include "mnist-stats.h"
#include "1lnn.h"

#include <omp.h>





/**
 * @details Trains a layer by looping through and training its cells
 * @param l A pointer to the layer that is to be training
 */

void trainLayer(Layer *l){
    
    // open MNIST files
    FILE *imageFile, *labelFile;
    imageFile = openMNISTImageFile(MNIST_TRAINING_SET_IMAGE_FILE_NAME);
    labelFile = openMNISTLabelFile(MNIST_TRAINING_SET_LABEL_FILE_NAME);
    
    
    // screen output for monitoring progress
    //displayImageFrame(5,5);

    int errCount = 0;

    // for test performance
    time_t startTrainingTime = time(NULL); 
    
    // Loop through all images in the file

    int readImg;

    for (int imgCount=0; imgCount<MNIST_MAX_TRAINING_IMAGES; imgCount++){
        
        // display progress
        displayLoadingProgressTraining(imgCount,3,5);
        
        // Reading next image and corresponding label
        MNIST_Image img = getImage(imageFile);
        MNIST_Label lbl = getLabel(labelFile);

        // set target ouput of the number displayed in the current image (=label) to 1, all others to 0
        Vector targetOutput;
        targetOutput = getTargetOutput(lbl);
        
        //displayImage(&img, 6,6);
        double maxOut = 0;
        int maxInd = 0;
     
        // loop through all output cells for the given image
	#pragma omp parallel for
        for (int j=0; j < NUMBER_OF_OUTPUT_CELLS; j++){
            //trainCell(&l->cell[i], &img, targetOutput.val[i]);

    	   double unnormalized_err = targetOutput.val[j] * NUMBER_OF_INPUT_CELLS;
	   //l->cell[j].output=0;
	   double temp_l_cell_j = 0;
    	   #pragma omp simd
    	   for (int i=0; i<NUMBER_OF_INPUT_CELLS; i++){
	   	if (img.pixel[i]){
     		//   c->input[i] = img->pixel[i] ? 1 : 0; 
		//	temp = c->input[i] * c->weight[i];
     		//   c->output += temp;
		//	unnormalized_err = unnormalized_err - c->input[i] * c->weight[i];
		//      c->weight[i] += LEARNING_RATE * c->input[i] * unnormalized_err/NUMBER_OF_INPUT_CELLS;
			temp_l_cell_j = temp_l_cell_j + l->cell[j].weight[i];
			unnormalized_err = unnormalized_err - l->cell[j].weight[i];
       	 	//	c->weight[i] += LEARNING_RATE * unnormalized_err/NUMBER_OF_INPUT_CELLS;
			l->cell[j].weight[i] += 0.0000637755 * unnormalized_err;
		}
   	    }
   	    //l->cell[j].output = temp_l_cell_j/NUMBER_OF_INPUT_CELLS;
            temp_l_cell_j = temp_l_cell_j/NUMBER_OF_INPUT_CELLS;

        
            if (temp_l_cell_j > maxOut){
            	maxOut = temp_l_cell_j;
            	maxInd = j;
            }
    	}


        if (maxInd!=lbl) errCount++;
        
        //printf("\n      Prediction: %d   Actual: %d ",predictedNum, lbl);

        //displayProgress(imgCount, errCount, 3, 66);
        
    }

    time_t endTrainingTime = time(NULL);
    double trainingTime = difftime(endTrainingTime, startTrainingTime);
    printf("\n	Training time is %.1f sec\n", trainingTime);
    
    // Close files
    fclose(imageFile);
    fclose(labelFile);

}




/**
 * @details Tests a layer by looping through and testing its cells
 * Exactly the same as TrainLayer() but WITHOUT LEARNING.
 * @param l A pointer to the layer that is to be training
 */

void testLayer(Layer *l){
    
    // open MNIST files
    FILE *imageFile, *labelFile;
    imageFile = openMNISTImageFile(MNIST_TESTING_SET_IMAGE_FILE_NAME);
    labelFile = openMNISTLabelFile(MNIST_TESTING_SET_LABEL_FILE_NAME);
    
    
    // screen output for monitoring progress
    //displayImageFrame(7,5);
    
    int errCount = 0;
   
    time_t startTestingTime = time(NULL);
 
    int readImg;

    // Loop through all images in the file
    //#pragma omp parallel for
    for (int imgCount=0; imgCount<MNIST_MAX_TESTING_IMAGES; imgCount++){
        
        // display progress
        displayLoadingProgressTesting(imgCount,5,5);
        
        // Reading next image and corresponding label
	MNIST_Image img;
	MNIST_Label lbl;
	//#pragma omp critical(readImg)
	{
        	img = getImage(imageFile);
        	lbl = getLabel(labelFile);
	}
        
        // set target ouput of the number displayed in the current image (=label) to 1, all others to 0
        Vector targetOutput;
        targetOutput = getTargetOutput(lbl);
        
       // displayImage(&img, 8,6);
        
        // loop through all output cells for the given image
	#pragma omp parallel for
        for (int j=0; j < NUMBER_OF_OUTPUT_CELLS; j++){
            //testCell(&l->cell[i], &img, targetOutput.val[i]);

            double temp = 0; 
    	    #pragma omp simd
    	    for (int i=0; i<NUMBER_OF_INPUT_CELLS; i++){
            //c->input[i] = img->pixel[i] ? 1 : 0;
	    	if (img.pixel[i]){
        	   temp += l->cell[j].weight[i];
	    	}
    	    }
    
    	    l->cell[j].output = temp/NUMBER_OF_INPUT_CELLS;       


        }
        
        int predictedNum = getLayerPrediction(l);
        if (predictedNum!=lbl) errCount++;
        
        //printf("\n      Prediction: %d   Actual: %d ",predictedNum, lbl);
        
        //displayProgress(imgCount, errCount, 5, 66);
        
    }
    
    time_t endTestingTime = time(NULL);
    double testingTime = difftime(endTestingTime, startTestingTime);

    printf(" \n testing time is: %.1f sec \n\n", testingTime);

    // Close files
    fclose(imageFile);
    fclose(labelFile);
    
}





/**
 * @details Main function to run MNIST-1LNN
 */

int main(int argc, const char * argv[]) {
    
    // remember the time in order to calculate processing time at the end
    time_t startTime = time(NULL);
    
    // clear screen of terminal window
    clearScreen();
    printf("    MNIST-1LNN: a simple 1-layer neural network processing the MNIST handwriting images\n");
    
    // initialize all connection weights to random values between 0 and 1
    Layer outputLayer;
    initLayer(&outputLayer);
    trainLayer(&outputLayer);

    printf("Done training\n");
    testLayer(&outputLayer);

    locateCursor(38, 5);
    
    // Calculate and print the program's total execution time
    time_t endTime = time(NULL);
    double executionTime = difftime(endTime, startTime);
    printf("\n    DONE! Total execution time: %.1f sec\n\n",executionTime);
    
    return 0;
}


