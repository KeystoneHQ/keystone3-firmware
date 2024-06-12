import {readFile, writeFile} from 'fs'

const json_str_path = "assets/page_ton.txt"
const file = "assets/page_ton.json";

readFile(json_str_path, "utf-8", (err, data) => {
    const _data = JSON.parse(data);
    writeFile(file, _data, "utf8", () => {
        
    })
})
