from homegear import Homegear
import unittest
import time
import os


class CreateTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        global hg
        hg = Homegear("/var/run/homegear/homegearIPC.sock")

        global path
        path = os.getcwd() + "/testingDirectory"
        if not os.path.exists(path):
            os.mkdir(path)

    @classmethod
    def tearDownClass(cls):
        os.rmdir(path)

    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "watch",
                "path": path,
                "recursive": "false",
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
        if os.path.exists(path + "/foo"):
            os.rmdir((path + "/foo"))
        elif os.path.exists(path + "/foo.txt"):
            os.remove(path + "/foo.txt")

    def test_createDirectory(self):
        os.mkdir((path + "/foo"))
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "Create", f"Payload is {inputHistory[0][1]['payload']}, but should be Create)")

    def test_createDirectories(self):
        pass

    def test_createFile(self):
        file = open(path + "/foo.txt", "x")
        file.close()
        time.sleep(1)
        inputHistory = hg.getNodeVariable(n2, "inputHistory0")
        self.assertTrue(len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}")
        self.assertEqual(inputHistory[0][1]['payload'], "Create", f"Payload is {inputHistory[0][1]['payload']}, but should be Create)")

    def test_createFiles(self):
        pass


if __name__ == '__main__':
    unittest.main()
