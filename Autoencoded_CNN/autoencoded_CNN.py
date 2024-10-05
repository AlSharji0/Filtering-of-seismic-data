import pandas as pd
import numpy as np
from sklearn.preprocessing import normalize
import tensorflow as tf
from tensorflow.keras.models import Model
from tensorflow.keras.layers import Input, Conv1D, MaxPooling1D, UpSampling1D, Dense, Flatten, Reshape
import sys


class CNN_Autoencoder:
    def __init__(self, time_steps, n_filters, kernel_size, pool_size, latent_dim, epochs, batch_size, optimizer='adam', loss='mse'):
        self.time_steps = time_steps
        self.n_filters = n_filters
        self.kernel_size = kernel_size
        self.pool_size = pool_size
        self.latent_dim = latent_dim
        self.optimizer = optimizer
        self.loss = loss
        self.autoencoder = None
        self.epochs = epochs
        self.batch_size = batch_size
    
    def build_model(self):
        input_layer = Input(shape=(self.time_steps, 1))

        x = input_layer
        for filter in self.n_filters:
            x = Conv1D(filter, kernel_size=self.kernel_size, activation='relu', padding='same')(x)
            x = MaxPooling1D(pool_size=self.pool_size, padding='same')(x)

        latent = Conv1D(self.latent_dim, kernel_size=self.kernel_size, activation='relu', padding='same')(x)

        x = latent
        for filters in reversed(self.n_filters):
            x = Conv1D(filters, kernel_size=self.kernel_size, activation='relu', padding='same')(x)
            x = UpSampling1D(size=self.pool_size)(x)

        decoded = Conv1D(1, kernel_size=self.kernel_size, activation='sigmoid', padding='same')(x)
        decoded = tf.keras.layers.Cropping1D(cropping=(0, 1))(decoded)

        self.autoencoder = Model(input_layer, decoded)
        self.autoencoder.compile(optimizer=self.optimizer, loss=self.loss)

        self.autoencoder.summary()

    def train(self, data):
        history = self.autoencoder.fit(data, data, epochs=self.epochs, batch_size=self.batch_size)
        return history
    
    def reconstruct(self, data):
        return self.autoencoder.predict(data)

    def calculate_error(self, data, recon_data):
        mse = np.mean(np.power(data - recon_data, 2), axis=1)
        return mse
    
    def detect_anomalies(self, data, threshold_percentile=95):
        reconstructed_data = self.reconstruct(data)
        reconstruction_error = self.calculate_error(data, reconstructed_data)
        threshold = np.percentile(reconstruction_error, threshold_percentile)
        anomalies = (reconstruction_error > threshold)
        return anomalies, threshold


#time_steps = 120 // (df['time_rel(sec)'].iloc[2] - df['time_rel(sec)'].iloc[1])#Assuming the average earthquake is around 2 minutes --May need adjustment.
#time_steps = time_steps.astype(int)

def main(file_path):
    df = pd.read_csv(file_path)
    df['velocity(m/s)'] = np.abs(df['velocity(m/s)'])

    df = df.drop(df.columns[0], axis=1)

    time_steps = 120 // (df['time_rel(sec)'].iloc[2] - df['time_rel(sec)'].iloc[1])  # Assuming the average earthquake is around 2 minutes -- May need adjustment.
    time_steps = int(time_steps)
    print(f"time_steps: {time_steps}")

    df = df[df['velocity(m/s)'] > 1e-13]

    norm = normalize(df, norm='l2', axis=0)
    df_norm = pd.DataFrame(norm, columns=df.columns)

    n_samples = df_norm.shape[0] // time_steps
    print(f"n_samples: {n_samples}")

    velocity_data = df_norm['velocity(m/s)'].values[:n_samples * time_steps].reshape(n_samples, time_steps, 1)
    velocity_data = velocity_data.astype(np.float32)

    cnn_autoencoder = CNN_Autoencoder(
        time_steps=time_steps,
        n_filters=[32, 16],
        kernel_size=3,
        pool_size=2,
        latent_dim=8,
        epochs=50,
        batch_size=64
    )

    cnn_autoencoder.build_model()
    history = cnn_autoencoder.train(velocity_data)
    print(history.history)

    anomalies, threshold = cnn_autoencoder.detect_anomalies(velocity_data, threshold_percentile=95)
    print(f"Anomalies detected: {anomalies}, Threshold: {threshold}")

    #for i in range(len(anomalies)): 
        #for j in range(i * time_steps, time_steps + (i * time_steps)):
            #df['Labels'].iloc[j] = anomalies[i]

if len(sys.argv) < 2:
    print("Please provide the path to the data file.")
    sys.exit(1)
    
file_path = sys.argv[1]
main(file_path)