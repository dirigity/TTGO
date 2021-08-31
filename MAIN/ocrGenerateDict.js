let dots = [
    [0, 0],
    [3, 0],
    [1, 1],
    [0, 3],
    [2, 2],
    [1, 3],
    [0, 4],
    [2, 4],
];

let dotCodes = [
    "0", "1", "2", "3", "4", "5", "6", "7"
];

function pairs(ret, arr) {
    if (arr.length == 0) return ret;
    for (let i = 1; i < arr.length; i++) {
        ret.push([arr[0], arr[i]]);

    }
    return pairs(ret, arr.slice(1));
}

// 0   1
//   2
// 3   4
//   5   
// 6   7

console.log(pairs([], dots))
let pairsOfCodes = pairs([], dotCodes)
console.log(pairsOfCodes);

let UsableData = [
    {
        "char": "1",
        "links": ["14", "47"]
    },
    {
        "char": "2",
        "links": ["01", "14", "34", "36", "67"]
    },
    {
        "char": "3",
        "links": ["10", "41", "34", "47", "67"]
    },
    {
        "char": "4",
        "links": ["03", "34", "14", "47"]
    },
    {
        "char": "5",
        "links": ["01", "03", "34", "47", "67"]
    },
    {
        "char": "6",
        "links": ["01", "03", "34", "47", "67", "36"]
    },
    {
        "char": "7",
        "links": ["01", "14", "47"]
    },
    {
        "char": "8",
        "links": ["01", "03", "14", "34", "36", "47", "67"]
    },
    {
        "char": "9",
        "links": ["01", "03", "14", "34", "47"]
    },
    {
        "char": "0",
        "links": ["01", "14", "47", "67", "36", "03"]
    },
    {
        "char": "+",
        "links": ["25", "34"]
    },
    {
        "char": "-",
        "links": ["34"]
    },
    {
        "char": "(",
        "links": ["12", "25", "57"]
    },
    {
        "char": ")",
        "links": ["02", "25", "56"]
    },
    {
        "char": "/",
        "links": ["16"]
    },
    {
        "char": "^",
        "links": ["32", "24"]
    },
    {
        "char": "¬",
        "links": ["36", "65", "52", "21"]
    },
    {
        "char": "*",
        "links": ["32", "24", "45", "53"]
    },
    {
        "char": "x",
        "links": ["35", "45", "46", "57"]
    },
    {
        "char":"=",
        "links":["34","67"]
    }
]

// 0   1
//   2
// 3   4
//   5   
// 6   7

function indexOfCustom(arr, elm) {
    for (let i = 0; i < arr.length; i++) {
        if (JSON.stringify(elm) == JSON.stringify(arr[i])) {
            return i
        }

    }
    return -1;
}


let linkCodesPerChar = [];

for (let i = 0; i < UsableData.length; i++) {
    let linkCodes = {
        "char": UsableData[i].char,

        "binaryEncode": 0
        // "links": [],
    }

    for (let j = 0; j < UsableData[i].links.length; j++) {
        const link = UsableData[i].links[j];
        let nodeA = link[0]
        let nodeB = link[1];

        if (dotCodes.indexOf(nodeA) > dotCodes.indexOf(nodeB)) {
            let buff = nodeA;
            nodeA = nodeB;
            nodeB = buff;
        }

        let index = indexOfCustom(pairsOfCodes, [nodeA, nodeB]);

        //linkCodes.links.push(index)
        linkCodes.binaryEncode += 2 ** (index + 1);
    }


    linkCodesPerChar.push(linkCodes);
}

let printOut = "{";

for (let i = 0; i < linkCodesPerChar.length; i++) {
    printOut += "{'" + linkCodesPerChar[i].char + "',"
    printOut += linkCodesPerChar[i].binaryEncode + "},\n"
    
}

printOut += "};"

console.log(linkCodesPerChar);

console.log(printOut);