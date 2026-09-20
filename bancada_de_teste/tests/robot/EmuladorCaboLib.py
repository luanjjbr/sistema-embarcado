#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
==============================================================================
Biblioteca Robot Framework: EmuladorCaboLib
Módulo de comunicação Serial e geração de relatório multi-cenários:
Cenário -> Passo -> Nome da Ação -> Comando Enviado -> Recebido -> Tempo -> Status
==============================================================================
"""

import os
import time
from datetime import datetime
import serial
import serial.tools.list_ports


class EmuladorCaboLib:
    ROBOT_LIBRARY_SCOPE = "GLOBAL"

    def __init__(self):
        self.conexao = None
        self.porta = "COM5"
        self.baudrate = 115200
        self.historico = []

    def _obter_nome_cenario_atual(self):
        """Obtém dinamicamente o nome do cenário de teste ativo no Robot Framework."""
        try:
            from robot.libraries.BuiltIn import BuiltIn
            nome = BuiltIn().get_variable_value("${TEST NAME}")
            if nome:
                return nome
        except Exception:
            pass
        return "Cenário Geral"

    def conectar_emulador(self, porta="COM5", baudrate=115200):
        """Conecta à porta serial do microcontrolador."""
        self.porta = porta
        self.baudrate = int(baudrate)
        self.historico = []  # Reinicia o histórico a cada suite

        print(f"[*] Conectando a {self.porta} a {self.baudrate} bps...")
        self.conexao = serial.Serial(self.porta, self.baudrate, timeout=0.5)
        time.sleep(2.0)  # Aguarda estabilização do bootloader
        self.conexao.reset_input_buffer()
        print("[+] Conexão serial estabelecida.")

    def testar_passo(self, nome_passo, comando, esperado, cenario=None):
        """
        Executa um passo com nome descritivo dentro de um cenário.
        Mede a latência, valida a resposta esperada e registra tudo no histórico.
        """
        if not self.conexao or not self.conexao.is_open:
            raise RuntimeError("Porta serial não está conectada.")

        cenario_nome = cenario or self._obter_nome_cenario_atual()
        horario = time.strftime("%H:%M:%S")
        t_inicio = time.time()

        self.conexao.write(f"{comando}\n".encode("utf-8"))
        self.conexao.flush()

        time.sleep(0.12)
        linhas = []
        while self.conexao.in_waiting > 0:
            l = self.conexao.readline().decode("utf-8", errors="ignore").strip()
            if l and not l.startswith("bancada>"):
                linhas.append(l)

        latencia_ms = int((time.time() - t_inicio) * 1000)
        resposta = " | ".join(linhas) if linhas else "(sem resposta)"

        passou = esperado in resposta
        status_str = "PASSOU" if passou else "FALHOU"

        registro = {
            "cenario": cenario_nome,
            "passo": len(self.historico) + 1,
            "nome": nome_passo,
            "comando": comando,
            "enviado": comando,
            "resposta": resposta,
            "recebido": resposta,
            "esperado": esperado,
            "tempo_ms": latencia_ms,
            "horario": horario,
            "status": status_str,
        }
        self.historico.append(registro)

        if not passou:
            raise AssertionError(
                f"FALHOU no passo '{nome_passo}'!\n"
                f"Enviado: '{comando}' | Recebido: '{resposta}' | Esperado: '{esperado}' ({latencia_ms} ms)"
            )

        return f"[{nome_passo}] Enviado: '{comando}' | Recebido: '{resposta}' | Tempo: {latencia_ms} ms"

    def desconectar_emulador(self):
        """Desliga todos os MOSFETs, fecha a serial e gera o relatório estruturado."""
        if self.conexao and self.conexao.is_open:
            try:
                self.conexao.write(b"cabo 0\n")
                time.sleep(0.1)
            except Exception:
                pass
            self.conexao.close()
            print("[*] Porta serial fechada.")

        self.gerar_relatorio_simplificado()

    def gerar_relatorio_simplificado(self):
        """Gera relatório visual em HTML e Markdown agrupado por cenários e com nomes nos passos."""
        if not self.historico:
            return

        pasta_relatorios = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "..", "relatorios")
        )
        os.makedirs(pasta_relatorios, exist_ok=True)

        data_hora = datetime.now().strftime("%d/%m/%Y às %H:%M:%S")
        total_passos = len(self.historico)
        passaram = sum(1 for r in self.historico if r["status"] == "PASSOU")
        taxa = (passaram / total_passos * 100) if total_passos > 0 else 0

        # Agrupa os passos por cenário
        cenarios_dict = {}
        for r in self.historico:
            cenarios_dict.setdefault(r["cenario"], []).append(r)

        total_cenarios = len(cenarios_dict)

        # ====================================================================
        # 1. ARQUIVO MARKDOWN ESTRUTURADO POR CENÁRIOS
        # ====================================================================
        md_caminho = os.path.join(pasta_relatorios, "relatorio_simplificado.md")
        linhas_md = [
            f"# Relatório de Teste da Bancada",
            f"",
            f"**Data e Hora:** {data_hora} | **Porta Serial:** `{self.porta}` ({self.baudrate} bps)  ",
            f"**Resultado Geral:** **{passaram}/{total_passos} passos aprovados ({taxa:.0f}%) em {total_cenarios} cenários**",
            f"",
            f"---",
            f"",
        ]

        for cenario, passos in cenarios_dict.items():
            linhas_md.extend([
                f"### 🧪 {cenario}",
                f"",
                f"| # | Nome do Passo | Horário | Comando Enviado | Resposta Recebida | Tempo | Status |",
                f"| :---: | :--- | :---: | :--- | :--- | :---: | :---: |",
            ])
            for idx, r in enumerate(passos, 1):
                icone = "✅ PASSOU" if r["status"] == "PASSOU" else "❌ FALHOU"
                linhas_md.append(
                    f"| {idx} | **{r['nome']}** | {r['horario']} | `{r['enviado']}` | `{r['recebido']}` | {r['tempo_ms']} ms | {icone} |"
                )
            linhas_md.append("")

        with open(md_caminho, "w", encoding="utf-8") as f:
            f.write("\n".join(linhas_md))

        # ====================================================================
        # 2. ARQUIVO HTML LIMPO E MODERNO AGRUPADO POR CENÁRIOS
        # ====================================================================
        html_caminho = os.path.join(pasta_relatorios, "relatorio_simplificado.html")
        html_conteudo = f"""<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <title>Relatório de Teste da Bancada</title>
    <style>
        body {{
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background-color: #f1f5f9;
            margin: 0;
            padding: 30px 20px;
            color: #1e293b;
        }}
        .container {{
            max-width: 1000px;
            margin: 0 auto;
        }}
        .header {{
            background: #ffffff;
            padding: 24px 30px;
            border-radius: 10px;
            box-shadow: 0 1px 3px rgba(0,0,0,0.08);
            margin-bottom: 24px;
        }}
        h1 {{
            margin: 0 0 14px 0;
            font-size: 24px;
            color: #0f172a;
        }}
        .meta {{
            display: flex;
            flex-wrap: wrap;
            gap: 20px;
            font-size: 14px;
            color: #475569;
        }}
        .meta strong {{ color: #0f172a; }}
        .badge-success {{
            background: #dcfce7;
            color: #15803d;
            padding: 4px 12px;
            border-radius: 14px;
            font-weight: 600;
        }}
        .cenario-card {{
            background: #ffffff;
            border-radius: 10px;
            box-shadow: 0 1px 3px rgba(0,0,0,0.08);
            margin-bottom: 24px;
            overflow: hidden;
        }}
        .cenario-header {{
            background: #f8fafc;
            padding: 16px 24px;
            font-size: 16px;
            font-weight: 700;
            color: #1e293b;
            border-bottom: 1px solid #e2e8f0;
            display: flex;
            align-items: center;
            gap: 8px;
        }}
        table {{
            width: 100%;
            border-collapse: collapse;
        }}
        th {{
            background: #ffffff;
            color: #64748b;
            font-size: 12px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            padding: 12px 20px;
            text-align: left;
            border-bottom: 1px solid #e2e8f0;
        }}
        td {{
            padding: 14px 20px;
            font-size: 14px;
            border-bottom: 1px solid #f1f5f9;
            vertical-align: middle;
        }}
        tr:last-child td {{ border-bottom: none; }}
        tr:hover {{ background-color: #f8fafc; }}
        code {{
            font-family: "Consolas", "Courier New", monospace;
            background: #f1f5f9;
            padding: 4px 8px;
            border-radius: 4px;
            font-size: 13px;
            color: #0f172a;
        }}
        .status-pass {{
            display: inline-block;
            background: #dcfce7;
            color: #15803d;
            padding: 4px 10px;
            border-radius: 12px;
            font-weight: 600;
            font-size: 12px;
        }}
        .status-fail {{
            display: inline-block;
            background: #fee2e2;
            color: #b91c1c;
            padding: 4px 10px;
            border-radius: 12px;
            font-weight: 600;
            font-size: 12px;
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>📊 Relatório de Homologação da Bancada de Testes</h1>
            <div class="meta">
                <div><strong>Data e Hora:</strong> {data_hora}</div>
                <div><strong>Porta Serial:</strong> {self.porta} ({self.baudrate} bps)</div>
                <div><strong>Cenários:</strong> {total_cenarios}</div>
                <div><strong>Resultado Geral:</strong> <span class="badge-success">{passaram}/{total_passos} PASSOU ({taxa:.0f}%)</span></div>
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
                        <th style="width: 5%; text-align: center;">#</th>
                        <th style="width: 25%;">Nome do Passo</th>
                        <th style="width: 10%; text-align: center;">Horário</th>
                        <th style="width: 18%;">Comando Enviado</th>
                        <th style="width: 27%;">Resposta da Bancada</th>
                        <th style="width: 8%; text-align: center;">Tempo</th>
                        <th style="width: 7%; text-align: center;">Status</th>
                    </tr>
                </thead>
                <tbody>
"""
            for idx, r in enumerate(passos, 1):
                status_tag = '<span class="status-pass">✔ PASSOU</span>' if r["status"] == "PASSOU" else '<span class="status-fail">✖ FALHOU</span>'
                html_conteudo += f"""                    <tr>
                        <td style="text-align: center; color: #64748b; font-weight: bold;">{idx}</td>
                        <td style="font-weight: 600; color: #1e293b;">{r['nome']}</td>
                        <td style="text-align: center; color: #64748b; font-size: 13px;">{r['horario']}</td>
                        <td><code>{r['enviado']}</code></td>
                        <td><code>{r['recebido']}</code></td>
                        <td style="text-align: center; color: #64748b;">{r['tempo_ms']} ms</td>
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

        print(f"\n[+] Relatório estruturado por cenários gerado com sucesso:")
        print(f"    -> Visual HTML: file:///{html_caminho.replace(os.sep, '/')}")
        print(f"    -> Texto MD:    {md_caminho}\n")
