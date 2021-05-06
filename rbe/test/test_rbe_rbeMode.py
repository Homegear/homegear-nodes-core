from homegear import Homegear
import unittest
import sys
import time


class BlockValueChange(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global values
        values = [42, 24, 65, 38, 9999, 5.89, 2.95, 97.42, 57.03, 54.97, 0.764, 5.03]
        if socketPath:
            hg = Homegear(socketPath)
        else:
            hg = Homegear("/var/run/homegear/homegearIPC.sock")

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "rbe",
                "mode": "blockValueChange",
                "wires": [
                    [{"id": "n2", "port": 0}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": 1
            }
        ]
        nodeIds = hg.addNodesToFlow("Watch Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    def tearDown(self):
        hg.removeNodesFromFlow("Watch Unit test", "unit-test")

    def test_sameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": values[0]})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": values[0]})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], values[0], f"Payload is {inputHistory[0][1]['payload']}, but should be {values[0]}")

    def test_blockNone(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": values[0]})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": values[1]})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[1][1]['payload'], values[0], f"Payload is {inputHistory[1][1]['payload']}, but should be {values[0]}")
        self.assertEqual(inputHistory[0][1]['payload'], values[1], f"Payload is {inputHistory[0][1]['payload']}, but should be {values[1]}")

    def test_blockNoneExtended(self):
        for value in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": value})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        for i in range(10):
            hg.setNodeVariable(n1, "fixedInput0", {"payload": (values[0] * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (values[0] * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(values[0] * -1)}")

    def test_negativeNumbersBlockNone(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (values[0] * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (values[1] * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[1][1]['payload'], (values[0] * -1), f"Payload is {inputHistory[1][1]['payload']}, but should be {(values[0] * -1)}")
        self.assertEqual(inputHistory[0][1]['payload'], (values[1] * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(values[1] * -1)}")

    def test_negativeNumbersBlockNoneExtended(self):  # fail rounding issue
        for value in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": (value * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], (value * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(value * -1)}")

    def test_mixedNumbers(self):  # fail rounding issue
        for i in range(len(values)):
            if i % 2:
                hg.setNodeVariable(n1, "fixedInput0", {"payload": (values[i] * -1)})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, "inputHistory0")
                self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
                self.assertEqual(inputHistory[0][1]['payload'], (values[i] * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(values[i] * -1)}")
            else:
                hg.setNodeVariable(n1, "fixedInput0", {"payload": values[i]})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, "inputHistory0")
                self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
                self.assertEqual(inputHistory[0][1]['payload'], values[i], f"Payload is {inputHistory[0][1]['payload']}, but should be {values[i]}")


class BlockValueChangeIgnore(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global values
        values = [42, 24, 65, 38, 9999, 5.89, 2.95, 97.42, 57.03, 54.97, 0.764, 5.03]
        if socketPath:
            hg = Homegear(socketPath)
        else:
            hg = Homegear("/var/run/homegear/homegearIPC.sock")

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "rbe",
                "mode": "blockValueChangeIgnore",
                "wires": [
                    [{"id": "n2", "port": 0}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": 1
            }
        ]
        nodeIds = hg.addNodesToFlow("Watch Unit test", "unit-test", testFlow)

        if not nodeIds:
            raise SystemError('Error =>  Could not create flow.')

        global n1, n2
        n1 = nodeIds["n1"]
        n2 = nodeIds["n2"]

        if not hg.restartFlows():
            raise SystemError("Error => Could not restart flows.")

        while not hg.nodeBlueIsReady():
            time.sleep(1)

    def tearDown(self):
        hg.removeNodesFromFlow("Watch Unit test", "unit-test")

    def test_sameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": values[0]})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": values[0]})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNone(inputHistory, f"No message was passed on. Length is {inputHistory}")

    def test_blockNoneButFirst(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": values[0]})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": values[1]})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) == 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], values[1], f"Payload is {inputHistory[0][1]['payload']}, but should be {values[1]}")

    def test_blockNoneButFirstExtended(self):
        for value in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": value})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == values[0]:
                self.assertIsNone(inputHistory, f"No message was passed on. Length is {inputHistory}")
            else:
                self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        for i in range(10):
            hg.setNodeVariable(n1, "fixedInput0", {"payload": (values[0] * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNone(inputHistory, f"No message was passed on. Length is {inputHistory}")

    def test_negativeNumbersBlockNoneButFirst(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (values[0] * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (values[1] * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) == 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (values[1] * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(values[1] * -1)}")

    def test_negativeNumbersBlockNoneButFirstExtended(self):  # fail rounding issue
        for value in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": (value * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == values[0]:
                self.assertIsNone(inputHistory, f"No message was passed on. Length is {inputHistory}")
            else:
                self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
                self.assertEqual(inputHistory[0][1]['payload'], (value * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(value * -1)}")

    def test_mixedNumbers(self):  # fail rounding issue
        for i in range(len(values)):
            if i == 0:
                hg.setNodeVariable(n1, "fixedInput0", {"payload": (values[i] * -1)})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, "inputHistory0")
                self.assertIsNone(inputHistory, f"No message was passed on. Length is {inputHistory}")
            elif i % 2:
                hg.setNodeVariable(n1, "fixedInput0", {"payload": (values[i] * -1)})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, "inputHistory0")
                self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
                self.assertEqual(inputHistory[0][1]['payload'], (values[i] * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(values[i] * -1)}")
            else:
                hg.setNodeVariable(n1, "fixedInput0", {"payload": values[i]})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, "inputHistory0")
                self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
                self.assertEqual(inputHistory[0][1]['payload'], values[i], f"Payload is {inputHistory[0][1]['payload']}, but should be {values[i]}")


if __name__ == '__main__':
    global socketPath
    socketPath = ''
    if len(sys.argv) > 1:
        for arg in sys.argv:
            if arg.startswith("/") and not arg == sys.argv[0]:
                socketPath = arg
                sys.argv.remove(arg)
                break

    unittest.main()
