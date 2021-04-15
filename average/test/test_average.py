from homegear import Homegear
import unittest
import time
import random


def buildAverage(numbers):
    if numbers:
        return sum(numbers) / len(numbers)


class TestCurrentValueInteger(unittest.TestCase):
    global testValues, input
    testValues = [5, 10, 42, 20, 37, 2, 4, 0.2, 4.2, 42.42]
    input = "fixedInput"

    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "average",
                "averageOver": "currentValues",
                "deleteAfter": "20",
                "deleteAfterCheck": "true",
                "round": "integer",
                "inputs": "10",
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
        nodeIds = hg.addNodesToFlow("Average Unit test", "unit-test", testFlow)

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
        hg.removeNodesFromFlow("Average Unit test", "unit-test")

    def testAverageIntegers(self):
        values = []
        i = 0
        for value in testValues:
            hg.setNodeVariable(n1, (input + str(i)), {"payload": int(value)})
            values.append(int(value))
            i += 1

        average = buildAverage(values)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")

    def testAverageDouble(self):
        i = 0
        for value in testValues:
            hg.setNodeVariable(n1, (input + str(i)), {"payload": float(value)})
            i += 1

        average = buildAverage(testValues)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")

    def testAverage(self):
        i = 0
        for value in testValues:
            hg.setNodeVariable(n1, (input + str(i)), {"payload": value})
            i += 1

        average = buildAverage(testValues)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")

    def testAverageZero(self):
        values = [0, 0, 0, 0, 42.42, 74, 2.56, 0, 0, 0]
        i = 0
        for value in values:
            hg.setNodeVariable(n1, (input + str(i)), {"payload": value})
            i += 1

        average = buildAverage(values)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")

    def testAverageOverride(self):
        i = 0
        for value in testValues:
            hg.setNodeVariable(n1, (input + str(i)), {"payload": value})
            i += 1
        average = buildAverage(testValues)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")

        values = []
        for j in range(len(testValues)):
            if j % 2 == 0:
                hg.setNodeVariable(n1, (input + str(j)), {"payload": (testValues[j] * 2)})
                values.append(testValues[j] * 2)
            else:
                hg.setNodeVariable(n1, (input + str(j)), {"payload": testValues[j]})
                values.append(testValues[j])
        average = buildAverage(values)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")

    def testAverageNegative(self):
        values = []
        i = 0
        for value in testValues:
            hg.setNodeVariable(n1, (input + str(i)), {"payload": (value * (-1))})
            values.append(value * (-1))
            i += 1
        average = buildAverage(values)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")

    def testAverageRandom(self):
        values = [None] * 10
        for i in range(random.randint(20, 1000)):
            value = random.random() * random.random() * random.randint(0, 100)
            values[i % 10] = value
            hg.setNodeVariable(n1, (input + str(i % 10)), {"payload": value})
        average = buildAverage(values)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")


class TestCurrentValueDouble(unittest.TestCase):
    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "average",
                "averageOver": "currentValues",
                "deleteAfter": "20",
                "deleteAfterCheck": "true",
                "round": "double",
                "inputs": "10",
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
        nodeIds = hg.addNodesToFlow("Average Unit test", "unit-test", testFlow)

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
        hg.removeNodesFromFlow("Average Unit test", "unit-test")

    def testAverageInteger(self):
        values = []
        i = 0
        for value in testValues:
            hg.setNodeVariable(n1, (input + str(i)), {"payload": int(value)})
            i += 1
            values.append(int(value))

        average = buildAverage(values)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")

    def testAverageDouble(self):
        i = 0
        for value in testValues:
            hg.setNodeVariable(n1, (input + str(i)), {"payload": float(value)})
            i = i + 1

        average = buildAverage(testValues)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")

    def testAverage(self):
        i = 0
        for value in testValues:
            hg.setNodeVariable(n1, (input + str(i)), {"payload": value})
            i = i + 1

        average = buildAverage(testValues)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")

    def testAverageZero(self):
        values = [0, 0, 0, 0, 42.42, 74, 2.56, 0, 0, 0]
        i = 0
        for value in values:
            hg.setNodeVariable(n1, (input + str(i)), {"payload": value})
            i = i + 1

        average = buildAverage(values)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")

    def testAverageOverride(self):
        i = 0
        for value in testValues:
            hg.setNodeVariable(n1, (input + str(i)), {"payload": value})
            i = i + 1
        average = buildAverage(testValues)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")

        values = []
        for j in range(len(testValues)):
            if j % 2 == 0:
                hg.setNodeVariable(n1, (input + str(j)), {"payload": (testValues[j] * 2)})
                values.append(testValues[j] * 2)
            else:
                hg.setNodeVariable(n1, (input + str(j)), {"payload": testValues[j]})
                values.append(testValues[j])
        average = buildAverage(values)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")

    def testAverageNegative(self):
        values = []
        i = 0
        for value in testValues:
            hg.setNodeVariable(n1, (input + str(i)), {"payload": (value * (-1))})
            values.append(value * (-1))
            i = i + 1
        average = buildAverage(values)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")

    def testAverageRandom(self):
        values = [None] * 10
        for i in range(random.randint(20, 1000)):
            value = random.random() * random.random() * random.randint(0, 100)
            values[i % 10] = value
            hg.setNodeVariable(n1, (input + str(i % 10)), {"payload": value})
        average = buildAverage(values)
        time.sleep(1)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")


class TestTimeInteger(unittest.TestCase):
    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "average",
                "averageOver": "time",
                "interval": "5",
                "round": "integer",
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
        nodeIds = hg.addNodesToFlow("Average Unit test", "unit-test", testFlow)

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
        hg.removeNodesFromFlow("Average Unit test", "unit-test")

    def testAverageInteger(self):
        values = []
        for value in testValues:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": int(value)})
            values.append(int(value))
        average = buildAverage(values)
        time.sleep(6)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")

    def testAverageDouble(self):
        for value in testValues:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": float(value)})
        average = buildAverage(testValues)
        time.sleep(6)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")

    def testAverage(self):
        for value in testValues:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": value})
        average = buildAverage(testValues)
        time.sleep(6)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")

    def testAverageZero(self):
        values = [0, 0, 0, 0, 42.42, 74, 2.56, 0, 0, 0]
        for value in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": value})
        average = buildAverage(values)
        time.sleep(6)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")

    def testAverageAppend(self):
        values = testValues
        for value in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": value})
        time.sleep(1)
        for i in range(len(testValues)):
            if i % 2 == 0:
                hg.setNodeVariable(n1, "fixedInput0", {"payload": testValues[i]})
                values.append(testValues[i])
        average = buildAverage(values)
        time.sleep(5)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")

    def testAverageNegative(self):
        values = []
        for value in testValues:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": (value * (-1))})
            values.append(value * (-1))
        average = buildAverage(values)
        time.sleep(6)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")

    def testAverageRandom(self):
        values = []
        for i in range(random.randint(20, 100)):
            value = random.random() * random.random() * random.randint(0, 100)
            hg.setNodeVariable(n1, "fixedInput0", {"payload": value})
            values.append(value)
        average = buildAverage(values)
        time.sleep(6)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], round(average, 0), f"Payload is {inputHistory[0][1]['payload']}, but should be {round(average, 0)}")


class TestTimeDouble(unittest.TestCase):
    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "average",
                "averageOver": "time",
                "interval": "5",
                "round": "double",
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
        nodeIds = hg.addNodesToFlow("Average Unit test", "unit-test", testFlow)

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
        hg.removeNodesFromFlow("Average Unit test", "unit-test")

    def testAverageInteger(self):
        values = []
        for value in testValues:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": int(value)})
            values.append(int(value))
        average = buildAverage(values)
        time.sleep(6)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")

    def testAverageDouble(self):
        for value in testValues:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": float(value)})
        average = buildAverage(testValues)
        time.sleep(6)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")

    def testAverage(self):
        for value in testValues:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": value})
        average = buildAverage(testValues)
        time.sleep(6)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")

    def testAverageZero(self):
        values = [0, 0, 0, 0, 42.42, 74, 2.56, 0, 0, 0]
        for value in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": value})
        average = buildAverage(values)
        time.sleep(6)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")

    def testAverageAppend(self):
        values = testValues
        for value in values:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": value})
        time.sleep(1)
        for i in range(len(testValues)):
            if i % 2 == 0:
                hg.setNodeVariable(n1, "fixedInput0", {"payload": testValues[i]})
                values.append(testValues[i])
        average = buildAverage(values)
        time.sleep(5)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")

    def testAverageNegative(self):
        values = []
        for value in testValues:
            hg.setNodeVariable(n1, "fixedInput0", {"payload": (value * (-1))})
            values.append(value * (-1))
        average = buildAverage(values)
        time.sleep(6)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")

    def testAverageRandom(self):
        values = []
        for i in range(random.randint(20, 100)):
            value = random.random() * random.random() * random.randint(0, 100)
            hg.setNodeVariable(n1, "fixedInput0", {"payload": value})
            values.append(value)
        average = buildAverage(values)
        time.sleep(6)

        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(round(inputHistory[0][1]['payload'], 7), round(average, 7), f"Payload is {round(inputHistory[0][1]['payload'], 7)}, but should be {round(average, 7)}")


if __name__ == '__main__':
    global hg
    hg = Homegear("/var/run/homegear/homegearIPC.sock")
    unittest.main()
