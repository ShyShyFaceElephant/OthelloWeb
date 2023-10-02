var express = require('express');
var bodyParser = require('body-parser');
const { exec, execFile } = require('child_process');
const fs = require('fs');
var app = express();

// set the view engine to ejs
app.set('view engine', 'ejs');
//靜態檔案目錄
app.use(express.static('public'));
// use res.render to load up an ejs view file

// index page
app.get('/', function (req, res) {
    res.render('index.ejs');
});
app.get('/introduce', (req, res) => {
    res.render('introduce.ejs');
})
app.use(bodyParser.json());
app.get('/cpp', (req, res) => {
    res.render('cpp.ejs');
})

app.post("/move", function (req, res) {
    const jsonData = req.body;
    const jsonString = JSON.stringify(jsonData);
    const exePath = __dirname + '/main';
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

port = process.env.PORT || 8080
app.listen(port, "0.0.0.0");
console.log('Server is listening on ' + port);