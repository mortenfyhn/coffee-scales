import numpy as np
import matplotlib.pyplot as plt

# Simulate data
np.random.seed(42)
t = np.linspace(0, 10, 1000)  # 10 seconds at 100 Hz
true_weight = 5.0 + 0.5 * np.sin(2 * np.pi * 0.5 * t)  # Weight signal (dynamic)
drift = 0.02 * t  # Slow drift
noise = np.random.normal(0, 0.1, size=t.shape)  # Measurement noise
raw_data = true_weight + drift + noise  # Total signal

# Kalman filter setup
x = np.array([0.0, 0.0])  # Initial state: [real_weight, drift]
P = np.eye(2)  # Initial covariance
A = np.array([[1, 0],  # State transition matrix
              [0, 1]])
H = np.array([1, 1])  # Observation matrix
Q = np.array([[0.1, 0],  # Process noise covariance
              [0, 1e-5]])
R = 0.01  # Measurement noise covariance

# Kalman filter variables
estimated_weights = []
estimated_drifts = []

# Run the Kalman filter
for z in raw_data:
    # Predict
    x_pred = A @ x
    P_pred = A @ P @ A.T + Q

    # Update
    K = P_pred @ H.T / (H @ P_pred @ H.T + R)  # Kalman gain
    x = x_pred + K * (z - H @ x_pred)          # State update
    P = (np.eye(2) - np.outer(K, H)) @ P_pred  # Covariance update

    # Save results
    estimated_weights.append(x[0])
    estimated_drifts.append(x[1])

# Plot the results
plt.figure(figsize=(12, 6))

# Raw data vs True weight
plt.plot(t, raw_data, label="Raw Data (Measured)", alpha=0.5)
plt.plot(t, true_weight, label="True Weight", linestyle="dashed")

# Filtered weight
plt.plot(t, estimated_weights, label="Filtered Weight (Kalman)")

# Drift
plt.plot(t, drift, label="True Drift", linestyle="dashed")
plt.plot(t, estimated_drifts, label="Estimated Drift (Kalman)")

plt.xlabel("Time (s)")
plt.ylabel("Weight (grams)")
plt.legend()
plt.title("Kalman Filter for Weight and Drift Separation")
plt.grid()
plt.show()
