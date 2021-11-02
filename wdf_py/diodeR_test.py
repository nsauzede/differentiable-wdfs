# %%
import numpy as np
from scipy.special import wrightomega
import matplotlib.pyplot as plt
import tensorflow as tf
from model_utils import save_model

# %%
def diode_pair_func(x, R = 1.0e-9, Is = 1.0e-9):
    a = x
    Vt = 25.85e-3
    R_Is = Is * R
    R_Is_overVt = R_Is / Vt
    logR_Is_overVt = np.log(R_Is_overVt)
    lamb = np.sign(a)
    b = a + 2 * lamb * (R_Is - Vt * wrightomega(logR_Is_overVt + lamb * a / Vt + R_Is_overVt))
    return np.float32(b)

# %%
N = 1000
test_x = None
for R_order in np.linspace(1, 9, 20):
    R = 10**R_order
    x_vals = np.linspace(-2.5, 2.5, N)
    in_vals = np.stack([x_vals, np.ones(N) * R]).transpose()

    if test_x is None:
        test_x = in_vals
    else:
        test_x = np.concatenate([test_x, in_vals])

plt.plot(test_x[:,0])
plt.plot(np.log(test_x[:,1]) / 2)

# %%
ideal_y = np.zeros_like(test_x[:,0])
for n in range(len(ideal_y)):
    # multiply by -1 to make the data line up better
    ideal_y[n] = -1 * diode_pair_func(*test_x[n])

# use log of impedance instead of impedance!
test_x[:,1] = np.log(test_x[:,1])

# %%
# plt.plot(test_x[:,0] + 5 * np.log(test_x[:,1]), ideal_y)
plt.plot(test_x[:,0])
# plt.plot(test_x[:,1])
plt.plot(ideal_y)

# %%
n_layers = 4
layer_size = 8
layer_xs = []
inputs = tf.keras.Input(shape=(2,))
for n in range(n_layers):
    layer_in = inputs if n < 1 else layer_xs[n-1]
    layer_width = layer_size if n < n_layers - 1 else 1
    layer_act = 'tanh' if n < n_layers - 1 else 'linear'
    layer_xs.append(tf.keras.layers.Dense(layer_width,
                                          activation=layer_act,
                                          kernel_initializer='orthogonal')(layer_in))

diode_model = tf.keras.Model(inputs=inputs, outputs=layer_xs[-1])
diode_model.summary()

# %%
mse_loss = tf.keras.losses.MeanSquaredError()

eps = np.finfo(np.float32).eps
def esr_loss(target_y, predicted_y, emphasis_func=lambda x : x):
    target_yp = emphasis_func(target_y)
    pred_yp = emphasis_func(predicted_y)
    mse = tf.math.reduce_sum(tf.math.square(target_yp - pred_yp))
    energy = tf.math.reduce_sum(tf.math.square(target_yp))
    
    loss_unnorm = mse / tf.cast(energy + eps, tf.float32)
    return tf.sqrt(loss_unnorm / N)

pre_loss_mse = mse_loss(ideal_y, diode_model(test_x)).numpy()
pre_loss_esr = esr_loss(ideal_y, diode_model(test_x)).numpy()
print(f'Loss before training: {pre_loss_mse}, {pre_loss_esr}')

def my_loss(target_y, predicted_y):
    return mse_loss(target_y, predicted_y) + esr_loss(target_y, predicted_y)

# %%
optimizer = tf.keras.optimizers.Adam(learning_rate=2e-5)
diode_model.compile(optimizer, my_loss)
diode_model.fit(test_x, ideal_y, epochs=2000)

y_test = diode_model(test_x).numpy().flatten()

# %%
plt.plot(ideal_y)
plt.plot(y_test, '--')

# plt.xlim(1400, 1600)
# plt.ylim(-0.25, 0.25)

# %%
diode_model.save('diodeR_test_model')

# %%
save_model(diode_model, 'diodeR_test_model.json')

# %%
# val_x = np.array([[0.45355588, 5.2008276]])
# print(diode_model(val_x))
# print(-1 * diode_pair_func(val_x[0,0], np.exp(val_x[0,1])))
