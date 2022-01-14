from homegear import Homegear
import unittest
import sys
import time


class BlockIfValueChangeGreaterEqualLastOutputFlatValue(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [45, 38, 47.99, 38, 42.42, 47.8574, 54.35, 44.36]
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
        values = [54, 24, 30.99, 28, 52, 32, 54.35, 29.36]
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
        values = [[56, False], [28, False], [34, True], [44, False], [24, False], [38.657, True], [45.295, True], [56, False]]
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

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [33, True], [-2.76, False], [-32.294, False], [37.408, True]]
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


class BlockIfValueChangeGreaterEqualLastOutputFlatValueNegativeNumbers(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -47.99, -38, -42.42, -47.8574, -54.35, -44.36]
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
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -24, -30.99, -28, -52, -32, -54.35, -29.36]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-34, True], [-44, False], [-24, False], [-38.657, True], [-45.295, True], [-56, False]]
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


class BlockIfValueChangeGreaterEqualLastOutputPercent(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [43, 40, 43.99, 39.592, 37.658, 40.42, 36.75]
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
        values = [35, 48, 46.2, 37.8, 36.5473, 47.638, 51.63, 24.42]
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
        values = [[56, False], [28, False], [39, True], [37.4, True], [30.6, False], [35.657, True], [44.295, False], [56, False]]
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

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [38, True], [-2.76, False], [-32.294, False], [37.408, True]]
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


class BlockIfValueChangeGreaterEqualLastOutputPercentNegativeNumbers(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -43.99, -39.592, -37.658, -40.42, -36.75]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-35, -48, -46.2, -37.8, -36.5473, -47.638, -51.63, -24.42]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-39, True], [-37.4, True], [-30.6, False], [-35.657, True], [-44.295, False], [-56, False]]
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


class BlockIfValueChangeGreaterEqualLastInputFlatValue(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [45, 38, 47.99, 38, 42.42, 47.8574, 54.35, 44.36]
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
        values = [54, 40, 52.68, 34.64, 63, 53, 63, 42]
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
        values = [[56, False], [28, False], [32, True], [42, False], [32, False], [32.657, True], [46.295, False], [37, True]]
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

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-28.65, True], [24.843, False], [-57.83, False], [-66.976, True]]
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


class BlockIfValueChangeGreaterEqualLastInputFlatValueNegativeNumbers(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -47.99, -38, -42.42, -47.8574, -54.35, -44.36]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -40, -52.68, -34.64, -63, -53, -63, -42]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-32, True], [-42, False], [-32, False], [-32.657, True], [-46.295, False], [-37, True]]
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


class BlockIfValueChangeGreaterEqualLastInputPercent(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [43, 40, 43.99, 39.592, 37.658, 40.42, 36.75]
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
        values = [50, 35, 47.546, 24.657, 42, 46.2, 41.58]
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
        values = [[56, False], [28, False], [29, True], [42, False], [46.2, False], [41.58, False], [43.295, True], [35, False]]
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

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
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


class BlockIfValueChangeGreaterEqualLastInputPercentNegativeNumbers(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -43.99, -39.592, -37.658, -40.42, -36.75]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-50, -35, -47.546, -24.657, -42, -46.2, -41.58]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-29, True], [-42, False], [-46.2, False], [-41.58, False], [-43.295, True], [-35, False]]
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


class BlockIfValueChangeGreaterLastOutputFlatValue(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [45, 38, 48, 38, 42.42, 47.8574, 54.35, 44.35]
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
        values = [54, 24, 30.99, 28, 52.1, 31.9, 54.35, 29.36]
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
        values = [[56, False], [28, False], [34, True], [44.1, False], [23.9, False], [38.657, True], [45.295, True], [56, False]]
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

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [33, True], [-2.76, False], [-32.294, False], [37.408, True]]
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


class BlockIfValueChangeGreaterLastOutputFlatValueNegativeNumbers(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -48, -38, -42.42, -47.8574, -54.35, -44.35]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -24, -30.99, -28, -52.1, -31.9, -54.35, -29.36]
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

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-34, True], [-44.1, False], [-23.9, False], [-38.657, True], [-45.295, True], [-56, False]]
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


class BlockIfValueChangeGreaterLastOutputPercent(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [43, 40, 44, 39.6, 37.658, 40.42, 36.75]
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
        values = [35, 48, 46.3, 37.7, 36.5473, 47.638, 51.63, 24.42]
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
        values = [[56, False], [28, False], [39, True], [35.09, False], [42.91, False], [37.657, True], [44.295, False], [56, False]]
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

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [38, True], [-2.76, False], [-32.294, False], [37.408, True]]
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


class BlockIfValueChangeGreaterLastOutputPercentNegativeNumbers(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -44, -39.6, -37.658, -40.42, -36.75]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-35, -48, -46.3, -37.7, -36.5473, -47.638, -51.63, -24.42]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-39, True], [-35.09, False], [-42.91, False], [-37.657, True], [-44.295, False], [-56, False]]
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


class BlockIfValueChangeGreaterLastInputFlatValue(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [45, 38, 48, 38, 42.42, 47.8574, 54.35, 44.35]
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
        values = [54, 40, 52.68, 34.64, 63, 52.9, 63, 42]
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
        values = [[56, False], [28, False], [32, True], [42.1, False], [32, False], [32.657, True], [46.295, False], [35, False]]
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

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
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


class BlockIfValueChangeGreaterLastInputFlatValueNegativeNumbers(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -48, -38, -42.42, -47.8574, -54.35, -44.35]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -40, -52.68, -34.64, -63, -52.9, -63, -42]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-32, True], [-42.1, False], [-32, False], [-32.657, True], [-46.295, False], [-35, False]]
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


class BlockIfValueChangeGreaterLastInputPercent(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [43, 40, 44, 39.6, 37.658, 40.42, 36.75]
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
        values = [50, 35, 47.546, 24.657, 42, 46.3, 41.66]
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
        values = [[56, False], [28, False], [29, True], [42, False], [46.21, False], [41.58, False], [43.295, True], [35, False]]
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

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
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


class BlockIfValueChangeGreaterLastInputPercentNegativeNumbers(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -44, -39.6, -37.658, -40.42, -36.75]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-50, -35, -47.546, -24.657, -42, -46.3, -41.66]
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-29, True], [-42, False], [-46.21, False], [-41.58, False], [-43.295, True], [-35, False]]
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


class BlockIfValueChangeGreaterEqualLastOutputFlatValueStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
                "startValue": str(startValue),
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
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) == 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [45, 38, 47.99, 38, 42.42, 47.8574, 54.35, 44.36]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [54, 24, 30.99, 28, 52, 32, 54.35, 29.36]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [34, True], [44, False], [24, False], [38.657, True], [45.295, True], [56, False]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [33, True], [-2.76, False], [-32.294, False], [37.408, True]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterEqualLastOutputFlatValueNegativeNumbersStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) == 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -47.99, -38, -42.42, -47.8574, -54.35, -44.36]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -24, -30.99, -28, -52, -32, -54.35, -29.36]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-34, True], [-44, False], [-24, False], [-38.657, True], [-45.295, True], [-56, False]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterEqualLastOutputPercentStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
                "startValue": str(startValue),
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
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) == 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [43, 40, 43.99, 39.592, 37.658, 40.42, 36.75]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [35, 48, 46.2, 37.8, 36.5473, 47.638, 51.63, 24.42]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [39, True], [37.4, True], [30.6, False], [35.657, True], [44.295, False], [56, False]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [38, True], [-2.76, False], [-32.294, False], [37.408, True]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterEqualLastOutputPercentNegativeNumbersStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -43.99, -39.592, -37.658, -40.42, -36.75]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-35, -48, -46.2, -37.8, -36.5473, -47.638, -51.63, -24.42]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-39, True], [-37.4, True], [-30.6, False], [-35.657, True], [-44.295, False], [-56, False]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterEqualLastInputFlatValueStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
                "startValue": str(startValue),
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
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [45, 38, 47.99, 38, 42.42, 47.8574, 54.35, 44.36]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [54, 40, 52.68, 34.64, 63, 53, 63, 42]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [32, True], [42, False], [32, False], [32.657, True], [46.295, False], [37, True]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-28.65, True], [24.843, False], [-57.83, False], [-66.976, True]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterEqualLastInputFlatValueNegativeNumbersStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -47.99, -38, -42.42, -47.8574, -54.35, -44.36]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -40, -52.68, -34.64, -63, -53, -63, -42]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-32, True], [-42, False], [-32, False], [-32.657, True], [-46.295, False], [-37, True]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterEqualLastInputPercentStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
                "startValue": str(startValue),
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
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [43, 40, 43.99, 39.592, 37.658, 40.42, 36.75]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [50, 35, 47.546, 24.657, 42, 46.2, 41.58]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [29, True], [42, False], [46.2, False], [41.58, False], [43.295, True], [35, False]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterEqualLastInputPercentNegativeNumbersStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -43.99, -39.592, -37.658, -40.42, -36.75]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-50, -35, -47.546, -24.657, -42, -46.2, -41.58]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-29, True], [-42, False], [-46.2, False], [-41.58, False], [-43.295, True], [-35, False]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterLastOutputFlatValueStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
                "startValue": str(startValue),
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
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [45, 38, 48, 38, 42.42, 47.8574, 54.35, 44.35]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [54, 24, 30.99, 28, 52.1, 31.9, 54.35, 29.36]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [34, True], [44.1, False], [23.9, False], [38.657, True], [45.295, True], [56, False]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [33, True], [-2.76, False], [-32.294, False], [37.408, True]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterLastOutputFlatValueNegativeNumbersStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -48, -38, -42.42, -47.8574, -54.35, -44.35]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -24, -30.99, -28, -52.1, -31.9, -54.35, -29.36]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-34, True], [-44.1, False], [-23.9, False], [-38.657, True], [-45.295, True], [-56, False]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterLastOutputPercentStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
                "startValue": str(startValue),
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
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [43, 40, 44, 39.6, 37.658, 40.42, 36.75]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [35, 48, 46.3, 37.7, 36.5473, 47.638, 51.63, 24.42]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [39, True], [35.09, False], [42.91, False], [37.657, True], [44.295, False], [56, False]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [38, True], [-2.76, False], [-32.294, False], [37.408, True]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterLastOutputPercentNegativeNumbersStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -44, -39.6, -37.658, -40.42, -36.75]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-35, -48, -46.3, -37.7, -36.5473, -47.638, -51.63, -24.42]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-39, True], [-35.09, False], [-42.91, False], [-37.657, True], [-44.295, False], [-56, False]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterLastInputFlatValueStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
                "startValue": str(startValue),
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
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [45, 38, 48, 38, 42.42, 47.8574, 54.35, 44.35]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [54, 40, 52.68, 34.64, 63, 52.9, 63, 42]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [32, True], [42.1, False], [32, False], [32.657, True], [46.295, False], [35, False]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterLastInputFlatValueNegativeNumbersStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -48, -38, -42.42, -47.8574, -54.35, -44.35]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -40, -52.68, -34.64, -63, -52.9, -63, -42]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-32, True], [-42.1, False], [-32, False], [-32.657, True], [-46.295, False], [-35, False]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterLastInputPercentStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = 42

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
                "startValue": str(startValue),
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
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_blockNone(self):
        values = [43, 40, 44, 39.6, 37.658, 40.42, 36.75]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_blockAll(self):
        values = [50, 35, 47.546, 24.657, 42, 46.3, 41.66]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [29, True], [42, False], [46.21, False], [41.58, False], [43.295, True], [35, False]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterLastInputPercentNegativeNumbersStartValueSet(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue
        startValue = -42

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": startValue})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertIsNotNone(inputHistory, "No message was passed on.")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -44, -39.6, -37.658, -40.42, -36.75]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNotNone(inputHistory, "No message was passed on.")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}")

    def test_negativeNumbersBlockAll(self):
        values = [-50, -35, -47.546, -24.657, -42, -46.3, -41.66]
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-29, True], [-42, False], [-46.21, False], [-41.58, False], [-43.295, True], [-35, False]]
        value = startValue
        for v in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": v[0]})
            if v[1]:
                value = v[0]
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == startValue:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertIsNotNone(inputHistory, "No message was passed on.")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockIfValueChangeGreaterEqualLastOutputFlatValueMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [45, 38, 47.99, 38, 42.42, 47.8574, 54.35, 44.36]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [54, 24, 30.99, 28, 52, 32, 54.35, 29.36]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [34, True], [44, False], [24, False], [38.657, True], [45.295, True], [56, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [33, True], [-2.76, False], [-32.294, False], [37.408, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastOutputFlatValueNegativeNumbersMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42

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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -47.99, -38, -42.42, -47.8574, -54.35, -44.36]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -24, -30.99, -28, -52, -32, -54.35, -29.36]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-34, True], [-44, False], [-24, False], [-38.657, True], [-45.295, True], [-56, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastOutputPercentMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [43, 40, 43.99, 39.592, 37.658, 40.42, 36.75]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [35, 48, 46.2, 37.8, 36.5473, 47.638, 51.63, 24.42]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [39, True], [37.4, True], [30.6, False], [35.657, True], [44.295, False], [56, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [38, True], [-2.76, False], [-32.294, False], [37.408, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastOutputPercentNegativeNumbersMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -43.99, -39.592, -37.658, -40.42, -36.75]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-35, -48, -46.2, -37.8, -36.5473, -47.638, -51.63, -24.42]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-39, True], [-37.4, True], [-30.6, False], [-35.657, True], [-44.295, False], [-56, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastInputFlatValueMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [45, 38, 47.99, 38, 42.42, 47.8574, 54.35, 44.36]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [54, 40, 52.68, 34.64, 63, 53, 63, 42]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [32, True], [42, False], [32, False], [32.657, True], [46.295, False], [37, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-28.65, True], [24.843, False], [-57.83, False], [-66.976, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastInputFlatValueNegativeNumbersMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -47.99, -38, -42.42, -47.8574, -54.35, -44.36]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -40, -52.68, -34.64, -63, -53, -63, -42]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-32, True], [-42, False], [-32, False], [-32.657, True], [-46.295, False], [-37, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastInputPercentMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [43, 40, 43.99, 39.592, 37.658, 40.42, 36.75]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [50, 35, 47.546, 24.657, 42, 46.2, 41.58]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [29, True], [42, False], [46.2, False], [41.58, False], [43.295, True], [35, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastInputPercentNegativeNumbersMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -43.99, -39.592, -37.658, -40.42, -36.75]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-50, -35, -47.546, -24.657, -42, -46.2, -41.58]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-29, True], [-42, False], [-46.2, False], [-41.58, False], [-43.295, True], [-35, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastOutputFlatValueMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [45, 38, 48, 38, 42.42, 47.8574, 54.35, 44.35]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [54, 24, 30.99, 28, 52.1, 31.9, 54.35, 29.36]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [34, True], [44.1, False], [23.9, False], [38.657, True], [45.295, True], [56, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [33, True], [-2.76, False], [-32.294, False], [37.408, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastOutputFlatValueNegativeNumbersMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -48, -38, -42.42, -47.8574, -54.35, -44.35]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -24, -30.99, -28, -52.1, -31.9, -54.35, -29.36]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-34, True], [-44.1, False], [-23.9, False], [-38.657, True], [-45.295, True], [-56, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastOutputPercentMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [43, 40, 44, 39.6, 37.658, 40.42, 36.75]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [35, 48, 46.3, 37.7, 36.5473, 47.638, 51.63, 24.42]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [39, True], [35.09, False], [42.91, False], [37.657, True], [44.295, False], [56, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [38, True], [-2.76, False], [-32.294, False], [37.408, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastOutputPercentNegativeNumbersMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -44, -39.6, -37.658, -40.42, -36.75]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-35, -48, -46.3, -37.7, -36.5473, -47.638, -51.63, -24.42]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-39, True], [-35.09, False], [-42.91, False], [-37.657, True], [-44.295, False], [-56, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastInputFlatValueMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [45, 38, 48, 38, 42.42, 47.8574, 54.35, 44.35]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [54, 40, 52.68, 34.64, 63, 52.9, 63, 42]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [32, True], [42.1, False], [32, False], [32.657, True], [46.295, False], [35, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastInputFlatValueNegativeNumbersMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -48, -38, -42.42, -47.8574, -54.35, -44.35]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -40, -52.68, -34.64, -63, -52.9, -63, -42]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-32, True], [-42.1, False], [-32, False], [-32.657, True], [-46.295, False], [-35, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastInputPercentMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(inputHistory[1][1]['payload'], startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [43, 40, 44, 39.6, 37.658, 40.42, 36.75]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [50, 35, 47.546, 24.657, 42, 46.3, 41.66]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [29, True], [42, False], [46.21, False], [41.58, False], [43.295, True], [35, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastInputPercentNegativeNumbersMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            self.assertEqual(round(inputHistory[1][1]['payload'], 7), startValue, f"Payload is {inputHistory[1][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -44, -39.6, -37.658, -40.42, -36.75]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-50, -35, -47.546, -24.657, -42, -46.3, -41.66]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-29, True], [-42, False], [-46.21, False], [-41.58, False], [-43.295, True], [-35, False]]
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastOutputFlatValueStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
                "startValue": str(startValue),
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
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertTrue(len(inputHistory) == 1, f"No message was passed on. Length is {len(inputHistory)}, input: {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [45, 38, 47.99, 38, 42.42, 47.8574, 54.35, 44.36]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [54, 24, 30.99, 28, 52, 32, 54.35, 29.36]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [34, True], [44, False], [24, False], [38.657, True], [45.295, True], [56, False]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [33, True], [-2.76, False], [-32.294, False], [37.408, True]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastOutputFlatValueNegativeNumbersStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertTrue(len(inputHistory) == 1, f"No message was passed on. Length is {len(inputHistory)}, input: {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -47.99, -38, -42.42, -47.8574, -54.35, -44.36]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -24, -30.99, -28, -52, -32, -54.35, -29.36]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-34, True], [-44, False], [-24, False], [-38.657, True], [-45.295, True], [-56, False]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastOutputPercentStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
                "startValue": str(startValue),
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
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertTrue(len(inputHistory) == 1, f"No message was passed on. Length is {len(inputHistory)}, input: {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [43, 40, 43.99, 39.592, 37.658, 40.42, 36.75]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [35, 48, 46.2, 37.8, 36.5473, 47.638, 51.63, 24.42]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [39, True], [37.4, True], [30.6, False], [35.657, True], [44.295, False], [56, False]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [38, True], [-2.76, False], [-32.294, False], [37.408, True]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastOutputPercentNegativeNumbersStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -43.99, -39.592, -37.658, -40.42, -36.75]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-35, -48, -46.2, -37.8, -36.5473, -47.638, -51.63, -24.42]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-39, True], [-37.4, True], [-30.6, False], [-35.657, True], [-44.295, False], [-56, False]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastInputFlatValueStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
                "startValue": str(startValue),
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
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [45, 38, 47.99, 38, 42.42, 47.8574, 54.35, 44.36]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [54, 40, 52.68, 34.64, 63, 53, 63, 42]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [32, True], [42, False], [32, False], [32.657, True], [46.295, False], [37, True]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-28.65, True], [24.843, False], [-57.83, False], [-66.976, True]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastInputFlatValueNegativeNumbersStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -47.99, -38, -42.42, -47.8574, -54.35, -44.36]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -40, -52.68, -34.64, -63, -53, -63, -42]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-32, True], [-42, False], [-32, False], [-32.657, True], [-46.295, False], [-37, True]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastInputPercentStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
                "startValue": str(startValue),
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
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [43, 40, 43.99, 39.592, 37.658, 40.42, 36.75]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [50, 35, 47.546, 24.657, 42, 46.2, 41.58]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [29, True], [42, False], [46.2, False], [41.58, False], [43.295, True], [35, False]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterEqualLastInputPercentNegativeNumbersStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -43.99, -39.592, -37.658, -40.42, -36.75]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-50, -35, -47.546, -24.657, -42, -46.2, -41.58]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-29, True], [-42, False], [-46.2, False], [-41.58, False], [-43.295, True], [-35, False]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastOutputFlatValueStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
                "startValue": str(startValue),
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
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [45, 38, 48, 38, 42.42, 47.8574, 54.35, 44.35]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [54, 24, 30.99, 28, 52.1, 31.9, 54.35, 29.36]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [34, True], [44.1, False], [23.9, False], [38.657, True], [45.295, True], [56, False]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [33, True], [-2.76, False], [-32.294, False], [37.408, True]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastOutputFlatValueNegativeNumbersStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -48, -38, -42.42, -47.8574, -54.35, -44.35]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -24, -30.99, -28, -52.1, -31.9, -54.35, -29.36]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-34, True], [-44.1, False], [-23.9, False], [-38.657, True], [-45.295, True], [-56, False]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastOutputPercentStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
                "startValue": str(startValue),
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
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [43, 40, 44, 39.6, 37.658, 40.42, 36.75]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [35, 48, 46.3, 37.7, 36.5473, 47.638, 51.63, 24.42]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [39, True], [35.09, False], [42.91, False], [37.657, True], [44.295, False], [56, False]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-42, False], [38, True], [-2.76, False], [-32.294, False], [37.408, True]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastOutputPercentNegativeNumbersStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -44, -39.6, -37.658, -40.42, -36.75]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-35, -48, -46.3, -37.7, -36.5473, -47.638, -51.63, -24.42]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-39, True], [-35.09, False], [-42.91, False], [-37.657, True], [-44.295, False], [-56, False]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastInputFlatValueStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
                "startValue": str(startValue),
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
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [45, 38, 48, 38, 42.42, 47.8574, 54.35, 44.35]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [54, 40, 52.68, 34.64, 63, 52.9, 63, 42]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [32, True], [42.1, False], [32, False], [32.657, True], [46.295, False], [35, False]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastInputFlatValueNegativeNumbersStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-45, -38, -48, -38, -42.42, -47.8574, -54.35, -44.35]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-54, -40, -52.68, -34.64, -63, -52.9, -63, -42]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-32, True], [-42.1, False], [-32, False], [-32.657, True], [-46.295, False], [-35, False]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastInputPercentStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = 42
        inputs = 3

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
                "startValue": str(startValue),
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
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(inputHistory[0][1]['payload'], startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_blockNone(self):
        values = [43, 40, 44, 39.6, 37.658, 40.42, 36.75]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(inputHistory[0][1]['payload'], v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_blockAll(self):
        values = [50, 35, 47.546, 24.657, 42, 46.3, 41.66]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_blockSome(self):
        values = [[56, False], [28, False], [29, True], [42, False], [46.21, False], [41.58, False], [43.295, True], [35, False]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")

    def test_mixedNumbers(self):
        values = [[54, False], [-32, False], [-31.65, True], [24.843, False], [-57.83, False], [-53.976, True]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


class BlockIfValueChangeGreaterLastInputPercentNegativeNumbersStartValueSetMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global startValue, inputs
        startValue = -42
        inputs = 3

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
                "startValue": str(startValue),
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

    def test_negativeNumbersSameValue(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": startValue})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
            self.assertEqual(round(inputHistory[0][1]['payload'], 7), startValue, f"Payload is {inputHistory[0][1]['payload']}, but should be {startValue}, input: {i}")

    def test_negativeNumbersBlockNone(self):
        values = [-43, -40, -44, -39.6, -37.658, -40.42, -36.75]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                self.assertEqual(round(inputHistory[0][1]['payload'], 7), v, f"Payload is {inputHistory[0][1]['payload']}, but should be {v}, input: {i}")

    def test_negativeNumbersBlockAll(self):
        values = [-50, -35, -47.546, -24.657, -42, -46.3, -41.66]
        for i in range(inputs):
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")

    def test_negativeNumbersBlockSome(self):
        values = [[-56, False], [-28, False], [-29, True], [-42, False], [-46.21, False], [-41.58, False], [-43.295, True], [-35, False]]
        for i in range(inputs):
            value = startValue
            for v in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": v[0]})
                if v[1]:
                    value = v[0]
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == startValue:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}, input: {i}")
                else:
                    self.assertIsNotNone(inputHistory, "No message was passed on to input {i}")
                    self.assertEqual(round(inputHistory[0][1]['payload'], 7), value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}, input: {i}")


if __name__ == '__main__':
    global socketPath
    global hg
    socketPath = ''
    if len(sys.argv) > 1:
        for arg in sys.argv:
            if arg.startswith("/") and not arg == sys.argv[0]:
                socketPath = arg
                sys.argv.remove(arg)
                break

    if socketPath:
        hg = Homegear(socketPath)
    else:
        hg = Homegear("/var/run/homegear/homegearIPC.sock")

    unittest.main()
