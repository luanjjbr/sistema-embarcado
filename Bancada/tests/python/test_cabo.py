"""
Automação de Testes da Bancada de Testes de Cabos (IEC 62196-2 / IEC 61851-1)
Executa varredura de comandos seriais e valida as respostas determinísticas do firmware.
"""

import sys
import time
import unittest
try:
    import serial
except ImportError:
    print("Aviso: 'pyserial' nao instalado. Execute: pip install pyserial")
    sys.exit(0)

PORTA_SERIAL = "COM3"  # Ajuste conforme a porta serial conectada
BAUD_RATE = 9600
TIMEOUT_S = 2.0


class TesteBancadaCabo(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        try:
            cls.ser = serial.Serial(PORTA_SERIAL, BAUD_RATE, timeout=TIMEOUT_S)
            time.sleep(2.0)  # Aguarda estabilização do bootloader Arduino
            cls.ser.reset_input_buffer()
        except serial.SerialException as e:
            raise unittest.SkipTest(f"Porta serial {PORTA_SERIAL} nao disponivel: {e}")

    @classmethod
    def tearDownClass(cls):
        if hasattr(cls, "ser") and cls.ser.is_open:
            cls.ser.write(b"cabo 0\n")  # Desliga todas as chaves ao finalizar
            cls.ser.close()

    def enviar_comando(self, comando: str) -> str:
        self.ser.reset_input_buffer()
        self.ser.write(f"{comando}\n".encode("utf-8"))
        time.sleep(0.1)
        resposta = self.ser.readline().decode("utf-8", errors="ignore").strip()
        return resposta

    def test_01_comando_help(self):
        """Valida que o comando 'help' retorna informacoes de comandos."""
        self.ser.reset_input_buffer()
        self.ser.write(b"help\n")
        time.sleep(0.2)
        linhas = self.ser.read_all().decode("utf-8", errors="ignore")
        self.assertIn("COMANDOS DA BANCADA", linhas)

    def test_02_comutacao_todos_canais(self):
        """Testa sequencialmente os canais 0 a 6 esperando resposta OK."""
        for canal in range(7):
            resposta = self.enviar_comando(f"cabo {canal}")
            self.assertEqual(resposta, "OK", f"Falha ao acionar canal {canal}: retorno '{resposta}'")

    def test_03_comando_status(self):
        """Valida retorno do relatorio com o comando 'cabo status'."""
        self.enviar_comando("cabo 2")  # Ativa 13A
        self.ser.reset_input_buffer()
        self.ser.write(b"cabo status\n")
        time.sleep(0.2)
        saida = self.ser.read_all().decode("utf-8", errors="ignore")
        self.assertIn("1500 Ohms", saida)
        self.assertIn("13 A", saida)

    def test_04_rejeicao_comando_invalido(self):
        """Valida que valores fora da faixa retornam erro padronizado."""
        resp1 = self.enviar_comando("cabo 9")
        self.assertEqual(resp1, "ERRO: Cabo")

        resp2 = self.enviar_comando("cabo")
        self.assertEqual(resp2, "ERRO: Cabo")

        resp3 = self.enviar_comando("desconhecido")
        self.assertEqual(resp3, "ERRO")


if __name__ == "__main__":
    unittest.main()
