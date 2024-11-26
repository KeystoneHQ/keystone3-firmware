import {readFile, writeFile} from 'fs'

const json_str_path = "assets/page_zcash.txt"
const file = "assets/page_zcash.json";

readFile(file, "utf-8", (err, data) => {
    const _data = JSON.parse(data);
    const str = JSON.stringify(JSON.stringify(_data));
    writeFile(json_str_path, str, "utf8", () => {
        
    })
})
