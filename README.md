# Filtering-of-seismic-data

# Table of Contents
- Presentation
-	Overview
-	Features
-	Future Features
-	Data
-	Usage
-	Acknowledgments

# Presentation

Google Drive with powerpoint:
https://docs.google.com/presentation/d/1KQLxOyJ9tyEQlOTg9fsVNGG-N88_qxhY/edit?usp=sharing&ouid=100257406178980417502&rtpof=true&sd=true

# Overview

-	This model processes both time-series and frequency-series data using a convolutional neural network - LSTM.
-	It utilizes the Space Apps 2024 Seismic Detection Data Packet for training.
-	This model is used to filter the data for seismic events so as to reduce quantity of data to be sent to Earth.
-	This project uses TensorFlow for building and training the model; and Pandas, Sklearn and Scipy for data preprocessing.

# Features

-	Flagging earthquakes events: This model filters through the data and flags the appropriate peaks as earthquake events.

# Future Features

- Data compression algorithm.

# Usage

-	The model is trained using the Autoencoded_CNN.ipynb script. Checkpoints are saved.
-	The model is evaluated using graphs of identified seismic events provided by NASA.
-	Flagging: Use the model to distinguish earthquake events from noise.

# Acknowledgements

-	Pandas
-	TensorFlow
-	NumPy
-	SKLearn
- Scipy
- Tableau


