import {readFile, writeFile} from 'fs'

const json_str_path = "assets/page_eth.txt"
const file = "assets/page_eth.json";

readFile(json_str_path, "utf-8", (err, data) => {
    const _data = JSON.parse(data);
    writeFile(file, _data, "utf8", () => {
        
    })
})
