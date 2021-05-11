from homegear import Homegear
import unittest
import sys
import time


class BlockValueChange(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global values
        values = ["homegear", "node-blue", "home", "gear", "node", "blue", "Blue"]
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
        self.assertTrue(len(inputHistory) == 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], values[0], f"Payload is {inputHistory[0][1]['payload']}, but should be {values[0]}")

    def test_blockNone(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": values[0]})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": values[1]})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], values[1], f"Payload is {inputHistory[0][1]['payload']}, but should be {values[1]}")
        self.assertEqual(inputHistory[1][1]['payload'], values[0], f"Payload is {inputHistory[1][1]['payload']}, but should be {values[0]}")

    def test_blockNoneExtended(self):
        for value in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": value})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeIgnore(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global values
        values = ["homegear", "node-blue", "home", "gear", "node", "blue", "Blue"]
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
        self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_blockNoneButFirst(self):
        hg.setNodeVariable(n1, "fixedInput0", {"payload": values[0]})
        hg.setNodeVariable(n1, "fixedInput0", {"payload": values[1]})
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) == 1, f"There should be one message, but length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], values[1], f"Payload is {inputHistory[0][1]['payload']}, but should be {values[1]}")

    def test_blockNoneButFirstExtended(self):
        for value in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": value})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            if value == values[0]:
                self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
            else:
                self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global values, inputs
        values = ["homegear", "node-blue", "home", "gear", "node", "blue", "Blue"]
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
                "mode": "blockValueChange",
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
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": values[0]})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": values[0]})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertTrue(len(inputHistory) == 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], values[0], f"Payload is {inputHistory[0][1]['payload']}, but should be {values[0]}")

    def test_blockNone(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": values[0]})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": values[1]})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], values[1], f"Payload is {inputHistory[0][1]['payload']}, but should be {values[1]}")
            self.assertEqual(inputHistory[1][1]['payload'], values[0], f"Payload is {inputHistory[1][1]['payload']}, but should be {values[0]}")

    def test_blockNoneExtended(self):
        for i in range(inputs):
            for value in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": value})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
                self.assertEqual(inputHistory[0][1]['payload'], value, f"Payload is {inputHistory[0][1]['payload']}, but should be {value}")


class BlockValueChangeIgnoreMultipleInputs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        global values, inputs
        values = ["homegear", "node-blue", "home", "gear", "node", "blue", "Blue"]
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
                "mode": "blockValueChangeIgnore",
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
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": values[0]})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": values[0]})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")

    def test_blockNoneButFirst(self):
        for i in range(inputs):
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": values[0]})
            hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": values[1]})
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
            self.assertTrue(len(inputHistory) == 1, f"There should be one message, but length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], values[1], f"Payload is {inputHistory[0][1]['payload']}, but should be {values[1]}")

    def test_blockNoneButFirstExtended(self):
        for i in range(inputs):
            for value in values:
                hg.setNodeVariable(n1, ("fixedInput" + str(i)), {"payload": value})
                time.sleep(1)
                inputHistory = hg.getNodeVariable(n2, ("inputHistory" + str(i)))
                if value == values[0]:
                    self.assertIsNone(inputHistory, f"No message expected. Length is {inputHistory}")
                else:
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
