import numpy as np
import sys
import os


# Simulação do Sistema Usando o Modelo MLP
def modelo_mlp(pwm, mlp, scaler_X, scaler_y):
    pwm_scaled = scaler_X.transform(np.array([[pwm]]))
    y_pred_scaled = mlp.predict(pwm_scaled)
    y_pred = scaler_y.inverse_transform(y_pred_scaled.reshape(-1, 1))
    return float(y_pred[0][0])

# 2) Simulador do Controle PID
def simular_pid(Kp, Ki, Kd, referencia, mlp, scaler_X, scaler_y, passos=200):
    erro_i = 0
    erro_ant = 0
    dist = 0
    pwm = 0
    dt = 0.1  # passo fictício da simulação

    erros = []

    for _ in range(passos):
        erro = referencia - dist
        erro_i += erro * dt
        erro_d = (erro - erro_ant) / dt

        pwm = Kp*erro + Ki*erro_i + Kd*erro_d
        pwm = np.clip(pwm, 140, 180)  # limites do sistema real

        dist = modelo_mlp(pwm, mlp, scaler_X, scaler_y)

        erro_ant = erro
        erros.append(erro)

    return np.array(erros)


# 3) Função de Fitness do PID
def fitness_pid(K, mlp, scaler_X, scaler_y):
    Kp, Ki, Kd = K
    referencia = 25  # exemplo: levitar a 25cm

    erros = simular_pid(Kp, Ki, Kd, referencia, mlp, scaler_X, scaler_y)

    mse = np.mean(erros**2)
    overshoot = max(0, (max(erros) - referencia))

    return mse + overshoot


# 4) Algoritmo Evolucionário
def evoluir_pid(mlp, scaler_X, scaler_y, pop_size=20, gerações=30):
    # População inicial aleatória
    pop = np.random.uniform(low=[0, 0, 0], high=[3, 1, 1], size=(pop_size, 3))

    for g in range(gerações):
        # Avaliar população
        fitness = np.array([fitness_pid(ind, mlp, scaler_X, scaler_y) for ind in pop])
        elite_idx = np.argsort(fitness)
        pop = pop[elite_idx]  # ordenar melhor → pior
        fitness = fitness[elite_idx]

        print(f"Geração {g+1}/{gerações} | Melhor fitness: {fitness[0]:.4f}")

        # Seleção → top 20%
        elite = pop[:pop_size//5]

        # Nova população
        nova_pop = elite.copy()
        while len(nova_pop) < pop_size:
            # Crossover
            pais = elite[np.random.choice(len(elite), size=2, replace=False)]
            filho = (pais[0] + pais[1]) / 2

            # Mutação
            mut = np.random.normal(0, 0.1, size=3)
            filho += mut

            filho = np.clip(filho, 0, 5)  # manter parâmetros razoáveis

            nova_pop = np.vstack([nova_pop, filho])

        pop = nova_pop

    melhor = pop[0]
    return melhor


# 5) Execução do Algoritmo
# Adiciona o diretório raiz ao path
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

if __name__ == "__main__":
    from Identificacao.modeloID.mlp import mlp, scaler_X, scaler_y

    print("\nIniciando Evolução do PID...")
    melhor_pid = evoluir_pid(mlp, scaler_X, scaler_y)

    print("\n Melhores parâmetros encontrados:")
    print(f"Kp = {melhor_pid[0]:.4f}")
    print(f"Ki = {melhor_pid[1]:.4f}")
    print(f"Kd = {melhor_pid[2]:.4f}")
