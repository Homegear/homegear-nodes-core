from homegear import Homegear
import unittest
import sys
import time


class BlockValueChangeGreaterEqualLastOutputFlatValue(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue
        startValue = 42
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
                "mode": "blockValueChangeGreaterEqual",
                "range": "10",
                "rangeType": "flatValue",
                "compareTo": "lastOutput",
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
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 14, 75, 24, 83, 73, 61.9363, 35.6584, 47.00001, 36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [47, 37, 51, 33, 40.364, 46.02, 38.94, 44.93, 51.99999, 32.00001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [38, True], [32, False], [36.7584, False], [27.999, True], [45.93, True], [42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -14, -75, -24, -83, -73, -61.9363, -35.6584, -47.00001, -36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-47, -37, -51, -33, -40.364, -46.02, -38.94, -44.93, -51.99999, -32.00001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-38, True], [-32, False], [-36.7584, False], [-27.999, True], [-45.93, True], [-42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [2, False], [6, True], [-2, False], [-32, True], [42, True], [0, True], [-9, False], [9, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterEqualLastOutputPercent(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue
        startValue = 42
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
                "mode": "blockValueChangeGreaterEqual",
                "range": "10",
                "rangeType": "percent",
                "compareTo": "lastOutput",
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
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 75, 82.5, 24, 83, 74.7, 65.9363, 35.6584, 47.00001, 36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [45, 39, 46, 38, 40.364, 43.02, 38.94, 44.93, 46.19999, 37.80001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [37, True], [33.5, False], [38.7584, False], [27.999, True], [45.93, True], [42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -75, -82.5, -24, -83, -74.7, -65.9363, -35.6584, -47.00001, -36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-45, -39, -46, -38, -40.364, -43.02, -38.94, -44.93, -46.19999, -37.80001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-37, True], [-33.5, False], [-38.7584, False], [-27.999, True], [-45.93, True], [-42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [-3.8, False], [6, True], [5.6, False], [-32, True], [42, True], [0, True], [-0.1, True], [0.1, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterEqualLastInputFlatValue(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue
        startValue = 42
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
                "mode": "blockValueChangeGreaterEqual",
                "range": "10",
                "rangeType": "flatValue",
                "compareTo": "lastInput",
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
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 14, 75, 24, 83, 73, 61.9363, 35.6584, 47.00001, 36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [47, 40, 31, 22, 28.364, 33.02, 43.01, 33.02, 37.99999, 32.00001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [38, True], [32, False], [36.7584, False], [27.999, False], [45.93, True], [42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -14, -75, -24, -83, -73, -61.9363, -35.6584, -47.00001, -36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-47, -40, -31, -22, -28.364, -33.02, -43.01, -33.02, -37.99999, -32.00001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-38, True], [-32, False], [-36.7584, False], [-27.999, False], [-45.93, True], [-42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [2, False], [6, False], [-2, False], [-32, True], [42, True], [0, True], [-9, False], [9, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterEqualLastInputPercent(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue
        startValue = 42
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
                "mode": "blockValueChangeGreaterEqual",
                "range": "10",
                "rangeType": "percent",
                "compareTo": "lastInput",
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
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 75, 82.5, 24, 83, 74.7, 65.9363, 35.6584, 47.00001, 36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [45, 42, 46, 44, 40.364, 43.02, 39.94, 41.93, 44.19999, 40.80001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [37, True], [33.5, False], [35.7584, False], [27.999, True], [45.93, True], [42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -75, -82.5, -24, -83, -74.7, -65.9363, -35.6584, -47.00001, -36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-45, -42, -46, -44, -40.364, -43.02, -39.94, -41.93, -44.19999, -40.80001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-37, True], [-33.5, False], [-35.7584, False], [-27.999, True], [-45.93, True], [-42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [-3.8, False], [6, True], [5.6, False], [-32, True], [42, True], [0, True], [-0.1, True], [0.1, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterLastOutputFlatValue(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue
        startValue = 42
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
                "mode": "blockValueChangeGreater",
                "range": "10",
                "rangeType": "flatValue",
                "compareTo": "lastOutput",
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
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 15, 75, 24, 83, 72.999, 61.9363, 35.6584, 47.00001, 36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [47, 37, 51, 33, 40.364, 46.02, 38.94, 44.93, 51.99999, 32.00001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [38, True], [32, False], [36.7584, False], [27.999, True], [45.93, True], [42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -15, -75, -24, -83, -72.999, -61.9363, -35.6584, -47.00001, -36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-47, -37, -51, -33, -40.364, -46.02, -38.94, -44.93, -51.99999, -32.00001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-38, True], [-32, False], [-36.7584, False], [-27.999, True], [-45.93, True], [-42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [2, False], [7, True], [-2, False], [-32, True], [42, True], [0, True], [-9, False], [9, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterLastOutputPercent(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue
        startValue = 42
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
                "mode": "blockValueChangeGreater",
                "range": "10",
                "rangeType": "percent",
                "compareTo": "lastOutput",
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
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 75, 82.6, 24, 83, 74.6, 65.9363, 35.6584, 47.00001, 36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [45, 39, 46, 38, 40.364, 43.02, 38.94, 44.93, 46.19999, 37.80001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [37, True], [33.5, False], [38.7584, False], [27.999, True], [45.93, True], [42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -75, -82.6, -24, -83, -74.6, -65.9363, -35.6584, -47.00001, -36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-45, -39, -46, -38, -40.364, -43.02, -38.94, -44.93, -46.19999, -37.80001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-37, True], [-33.5, False], [-38.7584, False], [-27.999, True], [-45.93, True], [-42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [-3.8, False], [6, True], [5.6, False], [-32, True], [42, True], [0, True], [-0.1, True], [0.1, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterLastInputFlatValue(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue
        startValue = 42
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
                "mode": "blockValueChangeGreater",
                "range": "10",
                "rangeType": "flatValue",
                "compareTo": "lastInput",
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
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 15, 75, 24, 83, 72, 61.9363, 35.6584, 47.00001, 36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [47, 40, 31, 22, 28.364, 33.02, 43.01, 33.01, 37.99999, 32.00001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [38, True], [32, False], [36.7584, False], [27.999, False], [45.93, True], [42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -15, -75, -24, -83, -72, -61.9363, -35.6584, -47.00001, -36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-47, -40, -31, -22, -28.364, -33.02, -43.01, -33.01, -37.99999, -32.00001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-38, True], [-32, False], [-36.7584, False], [-27.999, False], [-45.93, True], [-42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [2, False], [6, False], [-2, False], [-32, True], [42, True], [0, True], [-9, False], [9, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterLastInputPercent(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue
        startValue = 42
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
                "mode": "blockValueChangeGreater",
                "range": "10",
                "rangeType": "percent",
                "compareTo": "lastInput",
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
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 75, 82.6, 24, 83, 74.699, 65.9363, 35.6584, 47.00001, 36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [45, 42, 46, 44, 40.364, 43.02, 39.94, 41.93, 44.19999, 40.80001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [37, True], [33.5, False], [35.7584, False], [27.999, True], [45.93, True], [42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -75, -82.6, -24, -83, -74.699, -65.9363, -35.6584, -47.00001, -36.99999]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-45, -42, -46, -44, -40.364, -43.02, -39.94, -41.93, -44.19999, -40.80001]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-37, True], [-33.5, False], [-35.7584, False], [-27.999, True], [-45.93, True], [-42.42, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [-3.8, False], [6, True], [5.6, False], [-32, True], [42, True], [0, True], [-0.1, True], [0.1, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterEqualLastOutputFlatValueMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue, inputs
        startValue = 42
        inputs = 3
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
                "mode": "blockValueChangeGreaterEqual",
                "range": "10",
                "rangeType": "flatValue",
                "compareTo": "lastOutput",
                "differentTopics": "true",
                "inputs": inputs,
                "wires": [
                    [{"id": "n2", "port": 0}],
                    [{"id": "n2", "port": 1}],
                    [{"id": "n2", "port": 2}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": inputs
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
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 14, 75, 24, 83, 73, 61.9363, 35.6584, 47.00001, 36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [47, 37, 51, 33, 40.364, 46.02, 38.94, 44.93, 51.99999, 32.00001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [38, True], [32, False], [36.7584, False], [27.999, True], [45.93, True], [42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -14, -75, -24, -83, -73, -61.9363, -35.6584, -47.00001, -36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-47, -37, -51, -33, -40.364, -46.02, -38.94, -44.93, -51.99999, -32.00001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-38, True], [-32, False], [-36.7584, False], [-27.999, True], [-45.93, True], [-42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            value = (startValue * -1)
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [2, False], [6, True], [-2, False], [-32, True], [42, True], [0, True], [-9, False], [9, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterEqualLastOutputPercentMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue, inputs
        startValue = 42
        inputs = 3
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
                "mode": "blockValueChangeGreaterEqual",
                "range": "10",
                "rangeType": "percent",
                "compareTo": "lastOutput",
                "differentTopics": "true",
                "inputs": inputs,
                "wires": [
                    [{"id": "n2", "port": 0}],
                    [{"id": "n2", "port": 1}],
                    [{"id": "n2", "port": 2}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": inputs
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
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 75, 82.5, 24, 83, 74.7, 65.9363, 35.6584, 47.00001, 36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [45, 39, 46, 38, 40.364, 43.02, 38.94, 44.93, 46.19999, 37.80001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [37, True], [33.5, False], [38.7584, False], [27.999, True], [45.93, True], [42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -75, -82.5, -24, -83, -74.7, -65.9363, -35.6584, -47.00001, -36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-45, -39, -46, -38, -40.364, -43.02, -38.94, -44.93, -46.19999, -37.80001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-37, True], [-33.5, False], [-38.7584, False], [-27.999, True], [-45.93, True], [-42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            value = (startValue * -1)
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [-3.8, False], [6, True], [5.6, False], [-32, True], [42, True], [0, True], [-0.1, True], [0.1, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterEqualLastInputFlatValueMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue, inputs
        startValue = 42
        inputs = 3
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
                "mode": "blockValueChangeGreaterEqual",
                "range": "10",
                "rangeType": "flatValue",
                "compareTo": "lastInput",
                "differentTopics": "true",
                "inputs": inputs,
                "wires": [
                    [{"id": "n2", "port": 0}],
                    [{"id": "n2", "port": 1}],
                    [{"id": "n2", "port": 2}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": inputs
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
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 14, 75, 24, 83, 73, 61.9363, 35.6584, 47.00001, 36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [47, 40, 31, 22, 28.364, 33.02, 43.01, 33.02, 37.99999, 32.00001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [38, True], [32, False], [36.7584, False], [27.999, False], [45.93, True], [42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -14, -75, -24, -83, -73, -61.9363, -35.6584, -47.00001, -36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-47, -40, -31, -22, -28.364, -33.02, -43.01, -33.02, -37.99999, -32.00001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-38, True], [-32, False], [-36.7584, False], [-27.999, False], [-45.93, True], [-42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            value = (startValue * -1)
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [2, False], [6, False], [-2, False], [-32, True], [42, True], [0, True], [-9, False], [9, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterEqualLastInputPercentMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue, inputs
        startValue = 42
        inputs = 3
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
                "mode": "blockValueChangeGreaterEqual",
                "range": "10",
                "rangeType": "percent",
                "compareTo": "lastInput",
                "differentTopics": "true",
                "inputs": inputs,
                "wires": [
                    [{"id": "n2", "port": 0}],
                    [{"id": "n2", "port": 1}],
                    [{"id": "n2", "port": 2}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": inputs
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
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 75, 82.5, 24, 83, 74.7, 65.9363, 35.6584, 47.00001, 36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [45, 42, 46, 44, 40.364, 43.02, 39.94, 41.93, 44.19999, 40.80001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [37, True], [33.5, False], [35.7584, False], [27.999, True], [45.93, True], [42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -75, -82.5, -24, -83, -74.7, -65.9363, -35.6584, -47.00001, -36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-45, -42, -46, -44, -40.364, -43.02, -39.94, -41.93, -44.19999, -40.80001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-37, True], [-33.5, False], [-35.7584, False], [-27.999, True], [-45.93, True], [-42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            value = (startValue * -1)
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [-3.8, False], [6, True], [5.6, False], [-32, True], [42, True], [0, True], [-0.1, True], [0.1, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterLastOutputFlatValueMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue, inputs
        startValue = 42
        inputs = 3
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
                "mode": "blockValueChangeGreater",
                "range": "10",
                "rangeType": "flatValue",
                "compareTo": "lastOutput",
                "differentTopics": "true",
                "inputs": inputs,
                "wires": [
                    [{"id": "n2", "port": 0}],
                    [{"id": "n2", "port": 1}],
                    [{"id": "n2", "port": 2}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": inputs
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
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 15, 75, 24, 83, 72.999, 61.9363, 35.6584, 47.00001, 36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [47, 37, 51, 33, 40.364, 46.02, 38.94, 44.93, 51.99999, 32.00001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [38, True], [32, False], [36.7584, False], [27.999, True], [45.93, True], [42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -15, -75, -24, -83, -72.999, -61.9363, -35.6584, -47.00001, -36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-47, -37, -51, -33, -40.364, -46.02, -38.94, -44.93, -51.99999, -32.00001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-38, True], [-32, False], [-36.7584, False], [-27.999, True], [-45.93, True], [-42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            value = (startValue * -1)
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [2, False], [7, True], [-2, False], [-32, True], [42, True], [0, True], [-9, False], [9, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterLastOutputPercentMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue, inputs
        startValue = 42
        inputs = 3
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
                "mode": "blockValueChangeGreater",
                "range": "10",
                "rangeType": "percent",
                "compareTo": "lastOutput",
                "differentTopics": "true",
                "inputs": inputs,
                "wires": [
                    [{"id": "n2", "port": 0}],
                    [{"id": "n2", "port": 1}],
                    [{"id": "n2", "port": 2}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": inputs
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
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 75, 82.6, 24, 83, 74.6, 65.9363, 35.6584, 47.00001, 36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [45, 39, 46, 38, 40.364, 43.02, 38.94, 44.93, 46.19999, 37.80001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [37, True], [33.5, False], [38.7584, False], [27.999, True], [45.93, True], [42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -75, -82.6, -24, -83, -74.6, -65.9363, -35.6584, -47.00001, -36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-45, -39, -46, -38, -40.364, -43.02, -38.94, -44.93, -46.19999, -37.80001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-37, True], [-33.5, False], [-38.7584, False], [-27.999, True], [-45.93, True], [-42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            value = (startValue * -1)
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [-3.8, False], [6, True], [5.6, False], [-32, True], [42, True], [0, True], [-0.1, True], [0.1, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterLastInputFlatValueMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue, inputs
        startValue = 42
        inputs = 3
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
                "mode": "blockValueChangeGreater",
                "range": "10",
                "rangeType": "flatValue",
                "compareTo": "lastInput",
                "differentTopics": "true",
                "inputs": inputs,
                "wires": [
                    [{"id": "n2", "port": 0}],
                    [{"id": "n2", "port": 1}],
                    [{"id": "n2", "port": 2}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": inputs
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
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 15, 75, 24, 83, 72, 61.9363, 35.6584, 47.00001, 36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [47, 40, 31, 22, 28.364, 33.02, 43.01, 33.01, 37.99999, 32.00001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [38, True], [32, False], [36.7584, False], [27.999, False], [45.93, True], [42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -15, -75, -24, -83, -72, -61.9363, -35.6584, -47.00001, -36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-47, -40, -31, -22, -28.364, -33.02, -43.01, -33.01, -37.99999, -32.00001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-38, True], [-32, False], [-36.7584, False], [-27.999, False], [-45.93, True], [-42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            value = (startValue * -1)
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [2, False], [6, False], [-2, False], [-32, True], [42, True], [0, True], [-9, False], [9, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeGreaterLastInputPercentMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global startValue, inputs
        startValue = 42
        inputs = 3
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
                "mode": "blockValueChangeGreater",
                "range": "10",
                "rangeType": "percent",
                "compareTo": "lastInput",
                "differentTopics": "true",
                "inputs": inputs,
                "wires": [
                    [{"id": "n2", "port": 0}],
                    [{"id": "n2", "port": 1}],
                    [{"id": "n2", "port": 2}]
                ]
            },
            {
                "id": "n2",
                "type": "unit-test-helper",
                "inputs": inputs
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
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [4, 75, 82.6, 24, 83, 74.699, 65.9363, 35.6584, 47.00001, 36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [45, 42, 46, 44, 40.364, 43.02, 39.94, 41.93, 44.19999, 40.80001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[46, False], [4, True], [37, True], [33.5, False], [35.7584, False], [27.999, True], [45.93, True], [42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertEqual(len(inputHistory), 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-4, -75, -82.6, -24, -83, -74.699, -65.9363, -35.6584, -47.00001, -36.99999]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-45, -42, -46, -44, -40.364, -43.02, -39.94, -41.93, -44.19999, -40.80001]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-46, False], [-4, True], [-37, True], [-33.5, False], [-35.7584, False], [-27.999, True], [-45.93, True], [-42.42, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": (startValue * -1)})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
            value = (startValue * -1)
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[-4, True], [-3.8, False], [6, True], [5.6, False], [-32, True], [42, True], [0, True], [-0.1, True], [0.1, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


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
