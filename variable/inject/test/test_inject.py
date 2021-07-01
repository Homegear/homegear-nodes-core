from homegear import Homegear
import unittest
import sys
import time


class InjectTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        pass

    @classmethod
    def tearDownClass(cls):
        pass

    @classmethod
    def setUp(self):
        testFlow = [
            {
                "id": "n1",
                "type": "inject",
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

    @classmethod
    def tearDown(self):
        hg.removeNodesFromFlow("Average Unit test", "unit-test")

    def test(self):
        pass


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