from homegear import Homegear
import unittest
import sys
import time


class BlockIfValueChangeGreaterEqualLastOutputFlatValue(unittest.TestCase):
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
                "mode": "blockIfValueChangeGreaterEqual",
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [45, 38, 47.99, 38, 42.42, 47.8574, 54.35, 44.36]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [54, 24, 30.99, 28, 52, 32, 54.35, 29.36]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [34, True], [44, False], [24, False], [38.657, True], [45.295, True], [56, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        self.assertEqual(inputHistory[1][1]['payload'], (startValue * -1), f"Payload is {inputHistory[1][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -47.99, -38, -42.42, -47.8574, -54.35, -44.36]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -24, -30.99, -28, -52, -32, -54.35, -29.36]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-34, True], [-44, False], [-24, False], [-38.657, True], [-45.295, True], [-56, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [33, True], [-2.76, False], [-32.294, False], [37.408, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterEqualLastOutputPercent(unittest.TestCase):
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
                "mode": "blockIfValueChangeGreaterEqual",
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [43, 40, 43.99, 39.592, 37.658, 40.42, 36.75]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [35, 48, 46.2, 37.8, 36.5473, 47.638, 51.63, 24.42]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [39, True], [37.4, True], [30.6, False], [35.657, True], [44.295, False], [56, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        self.assertEqual(inputHistory[1][1]['payload'], (startValue * -1), f"Payload is {inputHistory[1][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -43.99, -39.592, -37.658, -40.42, -36.75]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-35, -48, -46.2, -37.8, -38.5473, -47.638, -51.63, -24.42]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-39, True], [-37.4, True], [-30.6, False], [-35.657, True], [-44.295, False], [-56, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [38, True], [-2.76, False], [-32.294, False], [37.408, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterEqualLastInputFlatValue(unittest.TestCase):
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
                "mode": "blockIfValueChangeGreaterEqual",
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [45, 38, 47.99, 38, 42.42, 47.8574, 54.35, 44.36]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [54, 40, 52.68, 34.64, 63, 53, 63, 42]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [32, True], [42, False], [32, False], [32.657, True], [46.295, False], [37, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        self.assertEqual(inputHistory[1][1]['payload'], (startValue * -1), f"Payload is {inputHistory[1][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -47.99, -38, -42.42, -47.8574, -54.35, -44.36]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -40, -52.68, -34.64, -63, -53, -63, -42]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-32, True], [-42, False], [-32, False], [-32.657, True], [-46.295, False], [-37, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-28.65, True], [24.843, False], [-57.83, False], [-66.976, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterEqualLastInputPercent(unittest.TestCase):
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
                "mode": "blockIfValueChangeGreaterEqual",
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [43, 40, 43.99, 39.592, 37.658, 40.42, 36.75]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [50, 35, 47.546, 24.657, 42, 46.2, 41.58]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [29, True], [42, False], [46.2, False], [41.58, False], [43.295, True], [35, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        self.assertEqual(inputHistory[1][1]['payload'], (startValue * -1), f"Payload is {inputHistory[1][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -43.99, -39.592, -37.658, -40.42, -36.75]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-50, -35, -47.546, -24.657, -42, -46.2, -41.58]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-29, True], [-42, False], [-46.2, False], [-41.58, False], [-43.295, True], [-35, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterLastOutputFlatValue(unittest.TestCase):
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
                "mode": "blockIfValueChangeGreater",
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [45, 38, 48, 38, 42.42, 47.8574, 54.35, 44.35]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [54, 24, 30.99, 28, 52.1, 31.9, 54.35, 29.36]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [34, True], [44.1, False], [23.9, False], [38.657, True], [45.295, True], [56, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        self.assertEqual(inputHistory[1][1]['payload'], (startValue * -1), f"Payload is {inputHistory[1][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -48, -38, -42.42, -47.8574, -54.35, -44.35]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -24, -30.99, -28, -52.1, -31.9, -54.35, -29.36]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-34, True], [-44.1, False], [-23.9, False], [-38.657, True], [-45.295, True], [-56, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [33, True], [-2.76, False], [-32.294, False], [37.408, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterLastOutputPercent(unittest.TestCase):
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
                "mode": "blockIfValueChangeGreater",
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [43, 40, 44, 39.6, 37.658, 40.42, 36.75]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [35, 48, 46.3, 37.7, 36.5473, 47.638, 51.63, 24.42]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [39, True], [35.09, False], [42.91, False], [37.657, True], [44.295, False], [56, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        self.assertEqual(inputHistory[1][1]['payload'], (startValue * -1), f"Payload is {inputHistory[1][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -44, -39.6, -37.658, -40.42, -36.75]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-35, -48, -46.3, -37.7, -36.5473, -47.638, -51.63, -24.42]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-39, True], [-35.09, False], [-42.91, False], [-37.657, True], [-44.295, False], [-56, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [38, True], [-2.76, False], [-32.294, False], [37.408, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterLastInputFlatValue(unittest.TestCase):
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
                "mode": "blockIfValueChangeGreater",
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [45, 38, 48, 38, 42.42, 47.8574, 54.35, 44.35]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [54, 40, 52.68, 34.64, 63, 52.9, 63, 42]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [32, True], [42.1, False], [32, False], [32.657, True], [46.295, False], [35, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        self.assertEqual(inputHistory[1][1]['payload'], (startValue * -1), f"Payload is {inputHistory[1][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -48, -38, -42.42, -47.8574, -54.35, -44.35]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -40, -52.68, -34.64, -63, -52.9, -63, -42]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-32, True], [-42.1, False], [-32, False], [-32.657, True], [-46.295, False], [-35, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterLastInputPercent(unittest.TestCase):
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
                "mode": "blockIfValueChangeGreater",
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
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [43, 40, 44, 39.6, 37.658, 40.42, 36.75]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [50, 35, 47.546, 24.657, 42, 46.3, 41.66]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [29, True], [42, False], [46.21, False], [41.58, False], [43.295, True], [35, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        self.assertEqual(inputHistory[1][1]['payload'], (startValue * -1), f"Payload is {inputHistory[1][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -44, -39.6, -37.658, -40.42, -36.75]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-50, -35, -47.546, -24.657, -42, -46.3, -41.66]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-29, True], [-42, False], [-46.21, False], [-41.58, False], [-43.295, True], [-35, False]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": (startValue * -1)})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], (startValue * -1), f"Payload is {inputHistory[0][1]['payload']}, but should be {(startValue * -1)}")
        value = (startValue * -1)
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


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
