$(document).ready(function () {
    // 定義一個 State 對象
    class State {
        constructor() {
            this.table = Array.from({ length: 8 }, () => Array(8).fill(-1));
            this.legalStepList = [19, 26, 37, 44];
            this.numberOfPieces = [2, 2];
            this.color = 1;
            this.gameOver = false;
            this.table[3][3] = this.table[4][4] = 0;
            this.table[4][3] = this.table[3][4] = 1;
        }
        copy(jsonData) {
            this.table = jsonData.table;
            this.legalStepList = jsonData.legalStepList;
            this.numberOfPieces = jsonData.numberOfPieces;
            this.color = jsonData.color;
            this.gameOver = jsonData.gameOver;
        }

    }
    class Pack {
        constructor(state, newPos) {
            this.gameState = state;
            this.newPosition = newPos;
        }
    }

    // 創建一個 State 對象的實例
    var gameStates = [];
    for (var i = 0; i < 60; i++) {
        gameStates.push(new State());
    }
    var step;//當先步數
    var isAI = [false, false];
    //AI切換
    $('#white-ai').change(() => {
        isAI[0] = !isAI[0];
    });
    $('#black-ai').change(() => {
        isAI[1] = !isAI[1];
    });
    function AImove() {
        if (idle && isAI[gameStates[step].color] && !gameStates[step].gameOver)
            move(-1);
    }
    setInterval(AImove, 1000);

    // 更新UI
    function update() {
        for (var i = 0; i < 64; i++) {
            if (gameStates[step].table[Math.floor(i / 8)][i % 8] == 1)
                $("#" + i).removeClass().addClass("box black");
            else if (gameStates[step].table[Math.floor(i / 8)][i % 8] == 0)
                $("#" + i).removeClass().addClass("box white");
            else if (gameStates[step].color)
                $("#" + i).removeClass().addClass("box empty");
            else
                $("#" + i).removeClass().addClass("box empty");
        }
        for (const e of gameStates[step].legalStepList) {
            $("#" + e).addClass((gameStates[step].color ? "black" : "white") + "-legal");
        }
        $("#n-white").html(gameStates[step].numberOfPieces[0]);
        $("#n-black").html(gameStates[step].numberOfPieces[1]);
        if (gameStates[step].color) {
            //$("#white").css("opacity", "0.3");
            //$("#black").css("opacity", "1");
            $("#message").html("現在輪到黑方");
        }
        else {
            //$("#white").css("opacity", "1");
            //$("#black").css("opacity", "0.3");
            $("#message").html("現在輪到白方");
        }
        if (gameStates[step].gameOver) {
            var delta = gameStates[step].numberOfPieces[1] - gameStates[step].numberOfPieces[0];
            //cout<<"rollout result:"<<delta<<'\n';
            if (delta > 0)
                $("#message").html("黑方勝利");
            else if (delta < 0)
                $("#message").html("白方勝利");
            else
                $("#message").html("平手!");
        }
    }
    // 初始化棋局
    function init() {
        step = 0;
        update();
    }
    function regret() {
        step -= 1;
        if (step < 0)
            step = 0;
        update();
    }
    //閒置狀態：閒置狀態時 按鈕才有效果
    var idle = 1;
    function workStart() {
        idle = 0;
        $("#message").html("...計算中...");
    }
    function workFinish() {
        idle = 1;
    }
    //落子
    function move(x) {
        var pos = parseInt(x);
        //判斷棋步合法
        var flag = 0
        for (const e of gameStates[step].legalStepList)
            if (e == pos)
                flag = 1
        if (pos == -1)
            flag = 1;
        if (!flag)
            return;
        workStart()//非閒置狀態 idle=0
        // 轉換為 JSON 字符串
        const pack = new Pack(gameStates[step], pos)
        const jsonPack = JSON.stringify(pack);
        console.log(jsonPack)
        $.ajax({
            type: "POST",
            url: "/move",
            contentType: "application/json",
            data: jsonPack,
            success: function (response) {
                // 處理成功響應
                state = new State()
                state.copy(JSON.parse(response));
                step += 1
                gameStates[step] = state;
                console.log("成功！", gameStates[step]);
                update();
                workFinish();//變回閒置狀態
            },
            error: function (error) {
                // 處理錯誤
                console.error("錯誤！", error);
            }
        });
    }
    //按下RESET
    $("#reset").on("click", function () {
        if (idle)
            init();
    });
    //按下悔其
    $("#regret").on("click", function () {
        if (idle)
            regret();
    });
    // 下子並將動作傳至後端
    $(".box").on("click", function () {
        if (idle && !isAI[gameStates[step].color])//閒置 與 非AI方才能觸發
            move(this.id);
    });
    console.log(gameStates);
    init();
});
