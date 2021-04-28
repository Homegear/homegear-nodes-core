from homegear import Homegear
import unittest
import time
import os


class WriteToFileTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg, path
        hg = Homegear("/var/run/homegear/homegearIPC.sock")

        path = os.getcwd() + "/testingDirectory"
        if not os.path.exists(path):
            os.mkdir(path)

    @classmethod
    def tearDownClass(cls):
        os.rmdir(path)

    def setUp(self):
        global filePath
        filePath = path + "/foo.txt"
        file = open(filePath, "x")
        file.close()
        testFlow = [
            {
                "id": "n1",
                "type": "tail",
                "path": filePath,
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

        for root, dirs, files in os.walk(path, topdown=False):
            for name in files:
                os.remove(os.path.join(root, name))
            for name in dirs:
                os.rmdir(os.path.join(root, name))

    def test_writeToFile(self):
        text = "This is a test"
        file = open(filePath, "a")
        file.write(text + '\n')
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], text, f"Payload is '{inputHistory[0][1]['payload']}', but should be '{text}'")

    def test_writeMultipleToFile(self):
        text = "This is a test"
        while text:
            file = open(filePath, "a")
            file.write(text + '\n')
            file.close()
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], text, f"Payload is '{inputHistory[0][1]['payload']}', but should be '{text}'")
            text = text[:-2]

    def test_writeEmptyLineToFile(self):
        file = open(filePath, "a")
        file.write('' + '\n')
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], '', f"Payload is '{inputHistory[0][1]['payload']}', but should be ''")


class AppendToFileTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg, path
        hg = Homegear("/var/run/homegear/homegearIPC.sock")

        path = os.getcwd() + "/testingDirectory"
        if not os.path.exists(path):
            os.mkdir(path)

    @classmethod
    def tearDownClass(cls):
        os.rmdir(path)

    def setUp(self):
        global filePath
        filePath = path + "/foo.txt"
        file = open(filePath, "x")
        file.write("This is the first line\nThis is the second line\n")
        file.close()
        testFlow = [
            {
                "id": "n1",
                "type": "tail",
                "path": filePath,
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

        for root, dirs, files in os.walk(path, topdown=False):
            for name in files:
                os.remove(os.path.join(root, name))
            for name in dirs:
                os.rmdir(os.path.join(root, name))

    def test_appendToFile(self):
        text = "This is a test"
        file = open(filePath, "a")
        file.write(text + '\n')
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], text, f"Payload is '{inputHistory[0][1]['payload']}', but should be '{text}'")

    def test_appendMultipleToFile(self):
        text = "This is a test"
        while text:
            file = open(filePath, "a")
            file.write(text + '\n')
            file.close()
            time.sleep(1)
            inputHistory = hg.getNodeVariable(n2, "inputHistory0")
            self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
            self.assertEqual(inputHistory[0][1]['payload'], text, f"Payload is '{inputHistory[0][1]['payload']}', but should be '{text}'")
            text = text[:-2]

    def test_appendEmptyLineToFile(self):
        file = open(filePath, "a")
        file.write('' + '\n')
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], '', f"Payload is '{inputHistory[0][1]['payload']}', but should be ''")


if __name__ == '__main__':
    unittest.main()
