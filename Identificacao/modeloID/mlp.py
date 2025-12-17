import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sklearn.neural_network import MLPRegressor
from sklearn.model_selection import train_test_split, cross_val_score
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import mean_squared_error, r2_score

try:
    df = pd.read_csv('Identificacao/dados_pwm.csv')
    print("Dados carregados com sucesso!")
except FileNotFoundError:
    print("Erro: Arquivo 'dados_pwm.csv' não encontrado.")
    exit()

df_subida = df[df['Distancia_cm'].diff() > 0]  # mantém as linhas onde a diferença da distância é positiva
X = df_subida[['PWM']].values
y = df_subida['Distancia_cm'].values

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

scaler_X = StandardScaler()
scaler_y = StandardScaler() 

X_train_scaled = scaler_X.fit_transform(X_train)
X_test_scaled = scaler_X.transform(X_test)

y_train_scaled = scaler_y.fit_transform(y_train.reshape(-1, 1)).ravel()
y_test_scaled = scaler_y.transform(y_test.reshape(-1, 1)).ravel()

mlp = MLPRegressor(
    hidden_layer_sizes=(64, 64), 
    activation='relu', 
    solver='lbfgs', 
    max_iter=5000, 
    random_state=42,
    alpha=0.01 
)

print("\nRealizando Validação Cruzada (5-fold)...")
scores = cross_val_score(mlp, X_train_scaled, y_train_scaled, cv=5, scoring='r2')
print(f"Scores de Validação R² em cada dobra: {scores}")
print(f"Média de Validação R²: {scores.mean():.4f} (+/- {scores.std() * 2:.4f})")

if scores.mean() < 0.8:
    print("AVISO: O modelo parece ter dificuldade de generalizar durante a validação.")

print("\nTreinando a Rede Neural MLP no conjunto de treino completo...")
mlp.fit(X_train_scaled, y_train_scaled)

y_pred_scaled = mlp.predict(X_test_scaled)
y_pred = scaler_y.inverse_transform(y_pred_scaled.reshape(-1, 1)).ravel()

mse = mean_squared_error(y_test, y_pred)
r2 = r2_score(y_test, y_pred)

print(f"\n--- Resultados Finais (Dados de Teste) ---")
print(f"Erro Médio Quadrático (MSE): {mse:.4f}")
print(f"Coeficiente de Determinação (R²): {r2:.4f}")

plt.figure(figsize=(10, 6))
plt.scatter(X, y, color='lightgray', label='Dados Reais', alpha=0.6)

X_range = np.linspace(X.min(), X.max(), 200).reshape(-1, 1)
X_range_scaled = scaler_X.transform(X_range)
y_range_pred_scaled = mlp.predict(X_range_scaled)
y_range_pred = scaler_y.inverse_transform(y_range_pred_scaled.reshape(-1, 1)).ravel()

plt.plot(X_range, y_range_pred, color='red', linewidth=2, label='Modelo Identificado')
plt.title('Identificação do Sistema: MLP')
plt.xlabel('PWM')
plt.ylabel('Distância (cm)')
plt.legend()
plt.grid(True, linestyle='--', alpha=0.7)

nome_arquivo = 'Identificacao/modeloID/grafico_identificacao.png'
plt.savefig(nome_arquivo)
print(f"\nGráfico salvo com sucesso como '{nome_arquivo}'")