var express = require('express');
var bodyParser = require('body-parser');
const { exec, execFile, spawn } = require('child_process');
const fs = require('fs');
var app = express();


app.set('view engine', 'ejs');//使用ejs渲染引擎
app.use(express.static('public'));//設定靜態檔案目錄


app.get('/', function (req, res) {//首頁
    res.render('index.ejs');
});
app.get('/introduce', (req, res) => {//專案簡介
    res.render('introduce.ejs');
})
app.get('/cpp', (req, res) => {
    res.render('cpp.ejs');
})

app.use(bodyParser.json());//引入解析json的工具

app.post("/move", function (req, res) {
    const jsonData = req.body;
    const kernelName = jsonData.kernel || "kernel_v02_pthread";
    delete jsonData.kernel;  // 不傳 kernel 給 C++，只傳遊戲資料

    // ✅ 根據 kernelName 決定 exe 路徑
    const kernelMap = {
        "kernel_v02": "v02.exe",
        "kernel_v02_pthread": "v02-pthread.exe"
    };

    const selectedExe = kernelMap[kernelName];
    if (!selectedExe) {
        return res.status(400).send("未知的 kernel: " + kernelName);
    }

    const exePath = __dirname + '\\kernel\\' + selectedExe;

    const cpp = spawn(exePath);

    let cppOutput = '';
    let cppError = '';

    cpp.stdout.on('data', (data) => {
        cppOutput += data.toString();
    });

    cpp.stderr.on('data', (data) => {
        cppError += data.toString();
    });

    cpp.on('close', (code) => {
        if (code !== 0) {
            console.error('CPP 程式錯誤:', cppError);
            return res.status(500).send('C++執行失敗');
        }
        try {
            const result = JSON.parse(cppOutput);
            res.send(result);
        } catch (parseError) {
            console.error('C++ 輸出解析錯誤:', parseError.message);
            res.status(500).send('C++輸出解析失敗');
        }
    });

    // ✅ 傳入純遊戲資料給 C++
    cpp.stdin.write(JSON.stringify(jsonData));
    cpp.stdin.end();
});


port = process.env.PORT || 8080
app.listen(port, "0.0.0.0");
console.log('Server is listening on ' + port);