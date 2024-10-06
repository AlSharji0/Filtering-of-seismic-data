# Filtering-of-seismic-data

# Table of Contents
-	Overview
-	Features
-	Data
-	Usage
-	Acknowledgments

# Overview

-	This model processes both time-series and frequency-series data using a convolutional neural network.
-	It utilizes the Space Apps 2024 Seismic Detection Data Packet for training.
-	This model is used to filter the data for seismic events so as to reduce quantity of data to be sent to Earth.
-	This project uses TensorFlow for building and training the model, and Pandas for data preprocessing.

# Features

-	Flagging earthquakes events: This model filters through the data and flags the appropriate peaks as earthquake events.
-	Data compression: This model 
-	Hash table:

# Usage

-	Training the model:
  o	The model is trained using the Autoencoded_CNN.ipynb script.
  o	Dataset files are queued using a Kernel driver.
  o	Checkpoints are saved
-	Evaluating the model
  o	.
-	Flagging
  o	Use the model to distinguish earthquake events from noise.
-	Data compression
  o	Compresses data before sending it to minimize energy usage during transfer.
-	Hash tables
  o	Uses hash tables to ensure data integrity.

# Acknowledgements

-	Pandas
-	TensorFlow
-	NumPy
-	SKLearn
-	Matplotlib
-	Hashlib



