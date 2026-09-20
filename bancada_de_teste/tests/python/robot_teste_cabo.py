#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
==============================================================================
Projeto: Bancada de Teste - Estação de Recarga Veicular (IEC 61851-1)
Script: robot_teste_cabo.py
Finalidade: Automação de teste em execução única com validação de cenários,
            nomes descritivos de passos e geração de relatório (HTML e Markdown).
==============================================================================
"""

import os
import sys
import time
import argparse
from datetime import datetime
import serial
import serial.tools.list_ports

# Tabela com as etapas organizadas por Cenário e Nomes Descritivos
ETAPAS_TESTE = [
    {
        "cenario": "Cenário 1: Emulação de Cabos de Recarga (Pino PP)",
        "passo": 1,
        "nome": "Ativar Cabo Desconectado (S1 - 4700 Ω)",
        "cmd": "cabo 1",
        "esperado": "OK",
    },
    {
        "cenario": "Cenário 1: Emulação de Cabos de Recarga (Pino PP)",
        "passo": 2,
        "nome": "Ativar Cabo de 13 A (S2 - 1500 Ω)",
        "cmd": "cabo 2",
        "esperado": "OK",
    },
    {
        "cenario": "Cenário 1: Emulação de Cabos de Recarga (Pino PP)",
        "passo": 3,
        "nome": "Ativar Cabo de 20 A (S3 - 680 Ω)",
        "cmd": "cabo 3",
        "esperado": "OK",
    },
    {
        "cenario": "Cenário 1: Emulação de Cabos de Recarga (Pino PP)",
        "passo": 4,
        "nome": "Ativar Cabo de 32 A (S4 - 220 Ω)",
        "cmd": "cabo 4",
        "esperado": "OK",
    },
    {
        "cenario": "Cenário 1: Emulação de Cabos de Recarga (Pino PP)",
        "passo": 5,
        "nome": "Ativar Cabo de 63 A (S5 - 100 Ω)",
        "cmd": "cabo 5",
        "esperado": "OK",
    },
    {
        "cenario": "Cenário 1: Emulação de Cabos de Recarga (Pino PP)",
        "passo": 6,
        "nome": "Simular Falha no Cabo (S6 - 47 Ω)",
        "cmd": "cabo 6",
        "esperado": "OK",
    },
    {
        "cenario": "Cenário 1: Emulação de Cabos de Recarga (Pino PP)",
        "passo": 7,
        "nome": "Desligar Cabo (Circuito Aberto Total)",
        "cmd": "cabo 0",
        "esperado": "OK",
    },
    {
        "cenario": "Cenário 2: Validação de Segurança e Erros",
        "passo": 8,
        "nome": "Rejeitar Cabo Inexistente (cabo 99)",
        "cmd": "cabo 99",
        "esperado": "ERRO",
    },
    {
        "cenario": "Cenário 2: Validação de Segurança e Erros",
        "passo": 9,
        "nome": "Rejeitar Comando Desconhecido",
        "cmd": "comando_invalido",
        "esperado": "ERRO",
    },
]


def detectar_porta_serial():
    """Detecta automaticamente a porta COM disponível."""
    portas = list(serial.tools.list_ports.comports())
    if not portas:
        return None

    preferenciais = ["COM5", "COM3"]
    for p in portas:
        if p.device in preferenciais:
            return p.device

    return portas[0].device


class ValidadorEmuladorCabo:
    def __init__(self, porta, baudrate=115200, intervalo=1.0):
        self.porta = porta
        self.baudrate = baudrate
        self.intervalo = intervalo
        self.conexao = None
        self.historico_resultados = []

    def conectar(self):
        print(f"\n[*] Conectando à bancada na porta {self.porta} ({self.baudrate} baud)...")
        try:
            self.conexao = serial.Serial(self.porta, self.baudrate, timeout=0.6)
            time.sleep(2.0)  # Aguarda estabilização do bootloader
            self.conexao.reset_input_buffer()
            print("[+] Conexão serial estabelecida com sucesso!")
            return True
        except Exception as e:
            print(f"[-] Falha ao abrir porta {self.porta}: {e}")
            return False

    def enviar_e_confirmar(self, etapa):
        """Envia comando, mede tempo e valida resposta."""
        comando = etapa["cmd"]
        esperado = etapa["esperado"]

        t_inicio = time.time()
        self.conexao.write(f"{comando}\n".encode("utf-8"))
        self.conexao.flush()

        time.sleep(0.12)
        linhas_resposta = []
        while self.conexao.in_waiting > 0:
            linha = self.conexao.readline().decode("utf-8", errors="ignore").strip()
            if linha and not linha.startswith("bancada>"):
                linhas_resposta.append(linha)

        latencia_ms = int((time.time() - t_inicio) * 1000)
        texto_resposta = " | ".join(linhas_resposta)

        aprovado = esperado in texto_resposta
        status_str = "PASSOU" if aprovado else "FALHOU"

        registro = {
            "cenario": etapa["cenario"],
            "passo": etapa["passo"],
            "nome": etapa["nome"],
            "comando": comando,
            "esperado": esperado,
            "resposta": texto_resposta,
            "latencia_ms": latencia_ms,
            "status": status_str,
            "timestamp": datetime.now().strftime("%H:%M:%S"),
        }
        self.historico_resultados.append(registro)
        return registro

    def executar_teste_unico(self):
        """Executa a rotina de validação."""
        print("\n=========================================================================================")
        print("         HOMOLOGAÇÃO DA BANCADA - EXECUÇÃO POR CENÁRIOS E PASSOS NOMEADOS               ")
        print("=========================================================================================")

        cenario_anterior = ""
        total_passos = len(ETAPAS_TESTE)

        for etapa in ETAPAS_TESTE:
            if etapa["cenario"] != cenario_anterior:
                cenario_anterior = etapa["cenario"]
                print(f"\n--- [ {cenario_anterior} ] ---")

            reg = self.enviar_e_confirmar(etapa)
            sinal = "✔" if reg["status"] == "PASSOU" else "✖"
            print(f"[{reg['timestamp']}] Passo {reg['passo']}/{total_passos}: {reg['nome']}")
            print(f"            Enviado: '{reg['comando']}' | Retorno: '{reg['resposta']}' | {sinal} {reg['status']} ({reg['latencia_ms']} ms)")

            time.sleep(self.intervalo)

        self.desconectar()
        self.exibir_relatorio_terminal()
        self.salvar_relatorio_arquivo()

    def desconectar(self):
        """Retorna ao estado seguro e fecha a porta serial."""
        if self.conexao and self.conexao.is_open:
            try:
                self.conexao.write(b"cabo 0\n")
                time.sleep(0.1)
                self.conexao.close()
            except Exception:
                pass
            print("\n[*] Desconectado da porta serial (Estado final: Cabo aberto).")

    def exibir_relatorio_terminal(self):
        """Imprime o relatório final tabulado no terminal."""
        total = len(self.historico_resultados)
        aprovados = sum(1 for r in self.historico_resultados if r["status"] == "PASSOU")
        taxa = (aprovados / total * 100) if total > 0 else 0

        print("\n==================================================================================================================")
        print("                                     RELATÓRIO SIMPLIFICADO DE TESTES                                            ")
        print("==================================================================================================================")
        print(f"Data/Hora:     {datetime.now().strftime('%d/%m/%Y %H:%M:%S')} | Porta: {self.porta} ({self.baudrate} baud)")
        print(f"Resultado:     {aprovados}/{total} PASSOU ({taxa:.0f}%)")
        print("------------------------------------------------------------------------------------------------------------------")
        print(f"{'#':<4} {'Nome do Passo':<40} {'Enviado':<18} {'Recebido':<38} {'Tempo':<8} {'Status'}")
        print("------------------------------------------------------------------------------------------------------------------")

        cenario_atual = ""
        for r in self.historico_resultados:
            if r["cenario"] != cenario_atual:
                cenario_atual = r["cenario"]
                print(f"\n>> {cenario_atual}:")

            resp_curta = (r['resposta'][:35] + '...') if len(r['resposta']) > 35 else r['resposta']
            print(f"{r['passo']:<4} {r['nome']:<40} {r['comando']:<18} {resp_curta:<38} {r['latencia_ms']:>4} ms  {r['status']}")

        print("==================================================================================================================")
        if taxa == 100.0:
            print(">>> STATUS GERAL: TODOS OS CENÁRIOS HOMOLOGADOS COM SUCESSO (100%) <<<")
        else:
            print(">>> STATUS GERAL: DETECTADA FALHA EM UM OU MAIS PASSOS <<<")
        print("==================================================================================================================\n")

    def salvar_relatorio_arquivo(self):
        """Gera relatórios simplificados em HTML e Markdown em tests/relatorios/."""
        pasta_relatorios = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "..", "relatorios")
        )
        os.makedirs(pasta_relatorios, exist_ok=True)

        data_hora = datetime.now().strftime("%d/%m/%Y às %H:%M:%S")
        total = len(self.historico_resultados)
        aprovados = sum(1 for r in self.historico_resultados if r["status"] == "PASSOU")
        taxa = (aprovados / total * 100) if total > 0 else 0

        # Agrupa por cenário
        cenarios_dict = {}
        for r in self.historico_resultados:
            cenarios_dict.setdefault(r["cenario"], []).append(r)

        # Markdown
        md_caminho = os.path.join(pasta_relatorios, "relatorio_simplificado.md")
        linhas_md = [
            f"# Relatório de Teste da Bancada",
            f"",
            f"**Data e Hora:** {data_hora} | **Porta Serial:** `{self.porta}` ({self.baudrate} bps)  ",
            f"**Resultado:** **{aprovados}/{total} passos aprovados ({taxa:.0f}%)**",
            f"",
            f"---",
            f"",
        ]

        for cenario, passos in cenarios_dict.items():
            linhas_md.extend([
                f"### 🧪 {cenario}",
                f"",
                f"| # | Nome do Passo | Enviado | Recebido | Tempo | Status |",
                f"| :---: | :--- | :--- | :--- | :---: | :---: |",
            ])
            for idx, r in enumerate(passos, 1):
                icone = "✅ PASSOU" if r["status"] == "PASSOU" else "❌ FALHOU"
                linhas_md.append(
                    f"| {idx} | **{r['nome']}** | `{r['comando']}` | `{r['resposta']}` | {r['tempo_ms']} ms | {icone} |"
                )
            linhas_md.append("")

        with open(md_caminho, "w", encoding="utf-8") as f:
            f.write("\n".join(linhas_md))

        # HTML
        html_caminho = os.path.join(pasta_relatorios, "relatorio_simplificado.html")
        html_conteudo = f"""<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <title>Relatório de Teste da Bancada</title>
    <style>
        body {{ font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background-color: #f1f5f9; margin: 0; padding: 30px 20px; color: #1e293b; }}
        .container {{ max-width: 1000px; margin: 0 auto; }}
        .header {{ background: #fff; padding: 24px 30px; border-radius: 10px; box-shadow: 0 1px 3px rgba(0,0,0,0.08); margin-bottom: 24px; }}
        h1 {{ margin: 0 0 12px 0; font-size: 22px; color: #0f172a; }}
        .meta {{ display: flex; gap: 20px; font-size: 14px; color: #475569; }}
        .badge-success {{ background: #dcfce7; color: #15803d; padding: 4px 12px; border-radius: 14px; font-weight: 600; }}
        .cenario-card {{ background: #fff; border-radius: 10px; box-shadow: 0 1px 3px rgba(0,0,0,0.08); margin-bottom: 24px; overflow: hidden; }}
        .cenario-header {{ background: #f8fafc; padding: 14px 20px; font-size: 15px; font-weight: 700; color: #1e293b; border-bottom: 1px solid #e2e8f0; display: flex; align-items: center; gap: 8px; }}
        table {{ width: 100%; border-collapse: collapse; }}
        th {{ background: #fff; color: #64748b; font-size: 12px; text-transform: uppercase; padding: 12px 18px; text-align: left; border-bottom: 1px solid #e2e8f0; }}
        td {{ padding: 12px 18px; font-size: 14px; border-bottom: 1px solid #f1f5f9; vertical-align: middle; }}
        tr:last-child td {{ border-bottom: none; }}
        tr:hover {{ background-color: #f8fafc; }}
        code {{ font-family: "Consolas", monospace; background: #f1f5f9; padding: 4px 8px; border-radius: 4px; font-size: 13px; color: #0f172a; }}
        .status-pass {{ display: inline-block; background: #dcfce7; color: #15803d; padding: 4px 10px; border-radius: 12px; font-weight: 600; font-size: 12px; }}
        .status-fail {{ display: inline-block; background: #fee2e2; color: #b91c1c; padding: 4px 10px; border-radius: 12px; font-weight: 600; font-size: 12px; }}
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>📊 Relatório de Teste da Bancada</h1>
            <div class="meta">
                <div><strong>Data/Hora:</strong> {data_hora}</div>
                <div><strong>Porta:</strong> {self.porta} ({self.baudrate} bps)</div>
                <div><strong>Resultado:</strong> <span class="badge-success">{aprovados}/{total} PASSOU ({taxa:.0f}%)</span></div>
            </div>
        </div>
"""
        for cenario, passos in cenarios_dict.items():
            html_conteudo += f"""
        <div class="cenario-card">
            <div class="cenario-header">
                <span>🧪</span>
                <span>{cenario}</span>
            </div>
            <table>
                <thead>
                    <tr>
                        <th style="width: 6%; text-align: center;">#</th>
                        <th style="width: 32%;">Nome do Passo</th>
                        <th style="width: 16%;">Enviado</th>
                        <th style="width: 30%;">Recebido da MCU</th>
                        <th style="width: 8%; text-align: center;">Tempo</th>
                        <th style="width: 8%; text-align: center;">Status</th>
                    </tr>
                </thead>
                <tbody>
"""
            for idx, r in enumerate(passos, 1):
                status_tag = '<span class="status-pass">✔ PASSOU</span>' if r["status"] == "PASSOU" else '<span class="status-fail">✖ FALHOU</span>'
                html_conteudo += f"""                    <tr>
                        <td style="text-align: center; color: #64748b; font-weight: bold;">{idx}</td>
                        <td style="font-weight: 600; color: #1e293b;">{r['nome']}</td>
                        <td><code>{r['comando']}</code></td>
                        <td><code>{r['resposta']}</code></td>
                        <td style="text-align: center; color: #64748b;">{r['latencia_ms']} ms</td>
                        <td style="text-align: center;">{status_tag}</td>
                    </tr>
"""
            html_conteudo += """                </tbody>
            </table>
        </div>
"""
        html_conteudo += """    </div>
</body>
</html>
"""
        with open(html_caminho, "w", encoding="utf-8") as f:
            f.write(html_conteudo)

        print(f"[+] Relatórios gerados em tests/relatorios/:\n    -> Visual: file:///{html_caminho.replace(os.sep, '/')}\n    -> Markdown: {md_caminho}\n")


def main():
    parser = argparse.ArgumentParser(
        description="Teste Automatizado Multi-Cenários com Relatório Simplificado"
    )
    parser.add_argument(
        "--port", "-p",
        default=None,
        help="Porta Serial (Ex: COM5 para Nano ou COM3 para ESP32). Se omitido, auto-detecta.",
    )
    parser.add_argument(
        "--baud", "-b",
        type=int,
        default=115200,
        help="Velocidade Serial em baud rate (padrão: 115200)",
    )
    parser.add_argument(
        "--interval", "-i",
        type=float,
        default=1.0,
        help="Intervalo entre comandos em segundos (padrão: 1.0s)",
    )

    args = parser.parse_args()
    porta = args.port or detectar_porta_serial()

    if not porta:
        print("[-] Nenhuma porta COM detectada. Conecte a placa via USB.")
        sys.exit(1)

    validador = ValidadorEmuladorCabo(
        porta=porta,
        baudrate=args.baud,
        intervalo=args.interval,
    )

    if validador.conectar():
        validador.executar_teste_unico()


if __name__ == "__main__":
    main()
