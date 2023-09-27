var express = require('express');
var bodyParser = require('body-parser');
const { exec, execFile } = require('child_process');
const fs = require('fs');
var app = express();

class State {
    constructor() {
        this.table = Array.from({ length: 8 }, () => Array(8).fill(-1));
        this.legalStepList = [20, 29, 34, 43];
        this.numberOfPieces = [2, 2];
        this.color = 1;
        this.gameOver = false;
        this.table[3][3] = this.table[4][4] = 1;
        this.table[4][3] = this.table[3][4] = 0;
    }

}
// set the view engine to ejs
app.set('view engine', 'ejs');
//靜態檔案目錄
app.use(express.static('public'));
// use res.render to load up an ejs view file

// index page
app.get('/', function (req, res) {
    res.render('index.ejs');
});

app.use(bodyParser.json());


app.post("/move", function (req, res) {
    const jsonData = req.body;
    const jsonString = JSON.stringify(jsonData);
    const exePath = __dirname + '/main.exe';
    // 添加執行權限（變更文件屬性）
    fs.chmod(exePath, '755', (chmodError) => {
        if (chmodError) {
            console.error(`添加執行權限時出現錯誤: ${chmodError}`);
            return;
        }

        console.log(`已成功添加執行權限: ${exePath}`);
    });
    // 将 JSON 字符串写入文件
    fs.writeFile('input.json', jsonString, (err) => {
        if (err) {
            console.error('写入文件时出错：', err);
            return res.status(500).send('写入文件时出错');
        }
        console.log(`JSON 数据已成功写入文件 ${'input.json'}`);

        // 要执行的可执行文件
        exec(exePath, (error, stdout, stderr) => {
            if (error) {
                console.error(`执行出错: ${error.message}`);
                return res.status(500).send('执行出错');
            }
            if (stderr) {
                console.error(`错误信息: ${stderr}`);
                return res.status(500).send('执行出错');
            }
            //console.log(`输出结果: ${stdout}`);

            // 读取并发送output.json
            fs.readFile('output.json', 'utf8', (err, data) => {
                if (err) {
                    console.error('读取文件时出错：', err);
                    return res.status(500).send('读取文件时出错');
                }
                console.log(`成功读取文件 ${'output.json'}`);

                // 发送文件内容给客户端
                res.send(data);
            });
        });
    });
});


app.listen(8080);
console.log('Server is listening on port 8080');