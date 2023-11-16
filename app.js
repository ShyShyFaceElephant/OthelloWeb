var express = require('express');
var bodyParser = require('body-parser');
const { exec, execFile } = require('child_process');
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

    /************更新棋局*************/
    //將json(內容:棋局、新步)寫入input.json
    fs.writeFile('input.json', jsonString, (err) => {
        if (err) {
            console.error('input.json寫檔錯誤:', err);
            return res.status(500).send('input.json寫檔錯誤');
        }
        console.log('input.json寫檔成功');
        // 執行CPP更新棋局
        exec(exePath, (error, stdout, stderr) => {
            if (error) {
                console.error(`CPP執行錯誤: ${error.message}`);
                return res.status(500).send('CPP執行錯誤');
            }
            if (stderr) {
                console.error(`CPP執行錯誤: ${stderr}`);
                return res.status(500).send('CPP執行錯誤');
            }
            // 讀取output.json並傳回前端
            fs.readFile('output.json', 'utf8', (err, data) => {
                if (err) {
                    console.error('output.json讀檔錯誤:', err);
                    return res.status(500).send('output.json讀檔錯誤');
                }
                console.log('output.json讀檔成功:');
                res.send(data);//新棋局傳回前端
            });
        });
    });
});

port = process.env.PORT || 8080
app.listen(port, "0.0.0.0");
console.log('Server is listening on ' + port);