<!--
// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
-->

# open62541DeviceDriver

open62541DeviceDriver is a tool to support the development of "Device Drivers" for [open62541](https://www.open62541.org/). A "Device Driver" in this context is set of functions that represent a device from a [Companion Secification](https://opcfoundation.org/about/opc-technologies/opc-ua/ua-companion-specifications/)(CS).

As a user, you can choose a CS and then choose one ore more Device-Types. For each device you can set all the Variables, Objects and Methods that are specified in the CS, including inherited Nodes. You can then set things like a value, description or the display name. This tool then generates code-skeletons that are compilable out of the box. For each node there are "empty" callback-functions where you connect your device.
For a detailed walk-through, see [Example Workflow](#example-workflow).

## Features

- Generates a code skeleton that represent a device.
- You just need implement the callback-functions to connect your hardware or data source.
- Each callback has a "User-Code" block, that will not be overwritten if you generate the code again.
- Not all CS work flawlessly. They are not tested thoroughly.
- All data types are supported. Including custom types that are defined in the CS or referenced from an other CS.
- You can set static values for variables.
- There are some features that are not implemented. Take a look at the issues for more ideas/features that might be included in the future!


## Example Workflow

This is only a gif preview. You can find a mp4 file in the `doc` folder.

![Example Workflow](doc/workflow_screencast.gif)

## Example

The following code snippets are genereated for an RfidReaderDeviceType from the AutoID Companion specification.
It is the same code that was genereated in the [Example Workflow](#example-workflow).
1. Adding the RfidReaderDeviceType to the server
```c
static void addRfidReaderDeviceType(UA_Server *server) {
    UA_NodeId RfidReaderDeviceTypeTypeId = UA_NODEID_NUMERIC(ns[0], 1003);

    UA_ObjectAttributes nodeAttr = UA_ObjectAttributes_default;
    nodeAttr.displayName = UA_LOCALIZEDTEXT("en-US", "RfidReaderDeviceType");
    nodeAttr.description = UA_LOCALIZEDTEXT("en-US", "Test");

    // Save current lifecycle
    UA_GlobalNodeLifecycle lifecycle = UA_Server_getConfig(server)->nodeLifecycle;
    UA_Server_getConfig(server)->nodeLifecycle.createOptionalChild = createOptionalChildCallback;
    UA_StatusCode retval = UA_Server_addObjectNode(server, RfidReaderDeviceType1003NodeId,
                                UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                                UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                UA_QUALIFIEDNAME(ns[0], "RfidReaderDeviceType"),
                                RfidReaderDeviceTypeTypeId,
                                nodeAttr, NULL, &RfidReaderDeviceType1003NodeId);

    UA_Server_getConfig(server)->nodeLifecycle = lifecycle;

    printf("RfidReaderDeviceType added to server with statuscode: %s\n", UA_StatusCode_name(retval));
}
```

2. Example of the Scan-Method callback. There are info comments about the datatype member.
```c
// Scan Callback NodeId: ns=1;i=7013
static UA_StatusCode Scan7013Callback(UA_Server *server,
                  const UA_NodeId *sessionId, void *sessionContext,
                  const UA_NodeId *methodId, void *methodContext,
                  const UA_NodeId *objectId, void *objectContext,
                  size_t inputSize, const UA_Variant *input,
                  size_t outputSize, UA_Variant *output) {

    UA_ScanSettings setting = *(UA_ScanSettings*)input[0].data;
    // INFO Members of UA_ScanSettings:
    /*
    * Int32 cycles
    * Boolean dataAvailable
    * Duration duration
    * LocationTypeEnumeration locationType
    */


    UA_RfidScanResult results;
    UA_init(&results, &UA_TYPES_AUTOID[UA_TYPES_AUTOID_RFIDSCANRESULT]);
    // INFO Members of UA_RfidScanResult:
    // CodeTypeDataType codeType
    // Location location
    // ScanData scanData
    // RfidSighting sighting
    // UtcTime timestamp

    UA_AutoIdOperationStatusEnumeration status;
    UA_init(&status, &UA_TYPES_AUTOID[UA_TYPES_AUTOID_AUTOIDOPERATIONSTATUSENUMERATION]);
    // INFO Enum fields in UA_AutoIdOperationStatusEnumeration;
    /*
    * CODE_NOT_SUPPORTED = 13
    * DECODING_ERROR = 11
    * DEVICE_FAULT = 20
    * DEVICE_NOT_READY = 17
    * INVALID_CONFIGURATION = 18
    * MATCH_ERROR = 12
    * MISC_ERROR_PARTIAL = 2
    * MISC_ERROR_TOTAL = 1
    * MULTIPLE_IDENTIFIERS = 9
    * NOT_SUPPORTED_BY_DEVICE = 15
    * NOT_SUPPORTED_BY_TAG = 16
    * NO_IDENTIFIER = 8
    * OP_NOT_POSSIBLE_ERROR = 6
    * OUT_OF_RANGE_ERROR = 7
    * PASSWORD_ERROR = 4
    * PERMISSON_ERROR = 3
    * READ_ERROR = 10
    * REGION_NOT_FOUND_ERROR = 5
    * RF_COMMUNICATION_ERROR = 19
    * SUCCESS = 0
    * TAG_HAS_LOW_BATTERY = 21
    * WRITE_ERROR = 14
    */


    printf("Scan7013Callback called!\n");

    //BEGIN user code Scan7013
    
    //END user code Scan7013

    UA_Variant_setScalarCopy(&output[0], &results, &UA_TYPES_AUTOID[UA_TYPES_AUTOID_RFIDSCANRESULT]);
    UA_Variant_setScalarCopy(&output[1], &status, &UA_TYPES_AUTOID[UA_TYPES_AUTOID_AUTOIDOPERATIONSTATUSENUMERATION]);

    return UA_STATUSCODE_GOOD;
}
```

3. Example of a variable with a static value that has been set with the tool.
```c
// readAntennaNames NodeId: ns=1;i=6048
static UA_StatusCode readAntennaNames6048(UA_Server *server, const UA_NodeId *sessionId, void *sessionContext, const UA_NodeId *nodeId, void *nodeContext, UA_Boolean sourceTimeStamp, const UA_NumericRange *range, UA_DataValue *dataValue) {
    UA_AntennaNameIdPair antennaNames;
    // Int32
    antennaNames.antennaId=42;
    // String
    antennaNames.antennaName = UA_STRING("Test");
    UA_Variant_setScalarCopy(&dataValue->value, &antennaNames, &UA_TYPES_AUTOID[UA_TYPES_AUTOID_ANTENNANAMEIDPAIR]);
    dataValue->hasValue = true;

    //BEGIN user code read AntennaNames6048
    
    //END user code read AntennaNames6048
    printf("readAntennaNames got called\n");
    return UA_STATUSCODE_GOOD;
}

```

Keep in mind that you need to implement the read/write and method callbacks to get the data from the actual device. 

## Getting open62541DeviceDriver

This Project uses the [UA-Nodeset](https://github.com/OPCFoundation/UA-Nodeset) and the C++ Mustache Implementation [mstch](https://github.com/no1msd/mstch) as submodules.

Clone the Project and checkout the submodules

```
git clone --recursive XXX
```

## Dependencies

- Qt 6.8.2

## Building

Use the Qt-Creator with Qt 6.8.2 to open, build and run the Project.

## Building the Generated Code

### Dependencies

- Open62541 v1.4.10

### Building
You will find the generated code in `templates/output.c` along with a `CMakelists.txt`.

Keep in mind that you need to set the `open62541_SOURCE_DIR` to your local open62541 path.

You can build it like this:

```bash
mkdir build && cd build
cmake .. -Dopen62541_SOURCE_DIR=/path/to/open62541/
make
```
It should build right away and you can find the binary in the `bin` folder. The name of the binary is equal to your selected companion specification:

```c
./bin/projectName
```

## Contributing

Contributions are welcome! If you encounter any issues or would like to request a feature, feel free to open an issue or submit a pull request.

The support of nodesets has not been thoroughly tested. So you might face some issues. If you encounter any problems, opening an issue would be appreciated!

## License

This project is released under the LGPLv3.0-or-later License.
