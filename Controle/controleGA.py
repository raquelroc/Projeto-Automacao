import numpy as np


# ======================================================
# 1) Modelo da planta (MLP)
# ======================================================
def modelo_mlp(pwm, mlp, scaler_X, scaler_y):
    pwm_scaled = scaler_X.transform(np.array([[pwm]]))
    y_pred_scaled = mlp.predict(pwm_scaled)
    y_pred = scaler_y.inverse_transform(y_pred_scaled.reshape(-1, 1))
    return float(y_pred[0][0])


# ======================================================
# 2) SIMULADOR DO CONTROLADOR P COM UM ÚNICO KP
# ======================================================
def simular_P_unico(Kp, referencia, mlp, scaler_X, scaler_y):
    dist = 0
    erros = []

    for _ in range(150):  # passos da simulação
        erro = referencia - dist
        pwm = np.clip(Kp * erro, 140, 180)
        dist = modelo_mlp(pwm, mlp, scaler_X, scaler_y)
        erros.append(erro)

    return np.array(erros)


# ======================================================
# 3) FITNESS — quanto menor melhor
# ======================================================
def fitness_P_unico(Kp, referencia, mlp, scaler_X, scaler_y):
    erros = simular_P_unico(Kp, referencia, mlp, scaler_X, scaler_y)
    mse = np.mean(erros**2)
    return mse


# ======================================================
# 4) ALGORITMO GENÉTICO PARA UM ÚNICO KP
# ======================================================
def GA_encontrar_Kp(mlp, scaler_X, scaler_y, referencia,
                    pop_size=30, geracoes=30,
                    prob_reproducao=0.8, taxa_mutacao=0.2):

    pop = np.random.uniform(0, 5, size=(pop_size,))

    for g in range(geracoes):

        # Avaliação
        fitness = np.array([fitness_P_unico(Kp, referencia, mlp, scaler_X, scaler_y)
                            for Kp in pop])

        melhor_idx = np.argmin(fitness)
        print(f"Geração {g+1}/{geracoes} | Melhor fitness = {fitness[melhor_idx]:.4f} | Kp = {pop[melhor_idx]:.4f}")

        weights = 1 / (fitness + 1e-8)
        probs = weights / weights.sum()

        nova_pop = pop.copy()

        # formar número de pares
        for _ in range(pop_size // 2):
            idx1 = np.random.choice(pop_size, p=probs)
            idx2 = np.random.choice(pop_size, p=probs)

            p1 = pop[idx1]
            p2 = pop[idx2]

            # chance de não reproduzir
            if np.random.rand() > prob_reproducao:
                continue

            # CROSSOVER: dois filhos (média + diferença invertida)
            cut_ratio = np.random.rand()
            filho1 = cut_ratio * p1 + (1 - cut_ratio) * p2
            filho2 = cut_ratio * p2 + (1 - cut_ratio) * p1

            # MUTAÇÃO
            filho1 += np.random.normal(0, taxa_mutacao)
            filho2 += np.random.normal(0, taxa_mutacao)

            filho1 = np.clip(filho1, 0, 5)
            filho2 = np.clip(filho2, 0, 5)

            # substitui os pais
            nova_pop[idx1] = filho1
            nova_pop[idx2] = filho2

        pop = nova_pop

    # Melhor final
    fitness = np.array([fitness_P_unico(Kp, referencia, mlp, scaler_X, scaler_y)
                        for Kp in pop])
    melhor_idx = np.argmin(fitness)
    return pop[melhor_idx]


# ======================================================
# 5) EXECUÇÃO: encontrar 5 valores de Kp
# ======================================================
if __name__ == "__main__":
    from Identificacao.modeloID.mlp import mlp, scaler_X, scaler_y

    print("\nIniciando busca de Kp por intervalo...\n")

    referencias = [45, 40, 35, 30, 25, 20, 15, 10, 5, 0]  # centro dos intervalos

    Kp_otimizados = []

    for ref in referencias:
        print(f"\n=== Otimizando Kp para referência {ref} cm ===")
        Kp_best = GA_encontrar_Kp(mlp, scaler_X, scaler_y, referencia=ref)
        Kp_otimizados.append(Kp_best)

    print("\n===== RESULTADOS FINAIS =====")
    print(f"Kp (50→45): {Kp_otimizados[0]:.4f}")
    print(f"Kp (45→40): {Kp_otimizados[1]:.4f}")
    print(f"Kp (40→35): {Kp_otimizados[2]:.4f}")
    print(f"Kp (35→30): {Kp_otimizados[3]:.4f}")
    print(f"Kp (30→25): {Kp_otimizados[4]:.4f}")
    print(f"Kp (25→20): {Kp_otimizados[5]:.4f}")
    print(f"Kp (20→15): {Kp_otimizados[6]:.4f}")
    print(f"Kp (15→10): {Kp_otimizados[7]:.4f}")
    print(f"Kp (10→5):  {Kp_otimizados[8]:.4f}")
    print(f"Kp (5→0):  {Kp_otimizados[9]:.4f}")

    resultado = f"""#define KP_50_40 {Kp_otimizados[0]:.4f}
#define Kp_40_30 {Kp_otimizados[1]:.4f}
#define Kp_30_20 {Kp_otimizados[2]:.4f}
#define Kp_20_10 {Kp_otimizados[3]:.4f}
#define Kp_10_0  {Kp_otimizados[4]:.4f}"""

    with open("Controle/kp_resultados.h", "w") as f:
        f.write(resultado)