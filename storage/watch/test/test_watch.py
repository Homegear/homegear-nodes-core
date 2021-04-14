from homegear import Homegear
import sys
import time
import os


def setup():
    nodeIds = hg.addNodesToFlow("Watch Unit test", "unit-test", testFlow)

    if not nodeIds:
        raise SystemError('Error =>  Could not create flow.')

    n1 = nodeIds["n1"]
    n2 = nodeIds["n2"]

    if not hg.restartFlows():
        raise SystemError("Error => Could not restart flows.")

    while not hg.nodeBlueIsReady():
        time.sleep(1)

    return n1, n2


def test_create_directory():
    n1, n2 = setup()
    os.mkdir((path + "/foo"))
    time.sleep(1)
    inputHistory = hg.getNodeVariable(n2, "inputHistory0")
    print(inputHistory)
    try:
        assert len(inputHistory) >= 1, f"No message was passed on. Length is {len(inputHistory)}"
        assert inputHistory[0][1]['payload'] == "Create", f"Payload is {inputHistory[0][1]['payload']}, but should be Create)"

        hg.removeNodesFromFlow("Watch Unit test", "unit-test")
        os.rmdir((path + "/foo"))
        return 1
    except AssertionError as e:
        print(e)
        hg.removeNodesFromFlow("Watch Unit test", "unit-test")
        os.rmdir((path + "/foo"))
        return -1


def test_create_directories():
    pass


def test_create_file():
    pass


def test_create_files():
    pass

if len(sys.argv) > 1:
    hg = Homegear(sys.argv[1])
else:
    hg = Homegear("/var/run/homegear/homegearIPC.sock")

path = os.getcwd() + "/testingDirectory"
if not os.path.exists(path):
    os.mkdir(path)

testFlow = [
    {
        "id": "n1",
        "type": "watch",
        "path": path,
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

print("testing watch node")
print("test 1: creating directory")
if test_create_directory() == 1:
    print("test passed")
else:
    print("test failed")

if test_create_directories() == 1:
    print("test passed")
else:
    print("test failed")

if test_create_file() == 1:
    print("test passed")
else:
    print("test failed")

if test_create_files() == 1:
    print("test passed")
else:
    print("test failed")

os.rmdir(path)
