前几天做了一个规则较验功能， 需要往TextArea中光标所在位置插入文本，于是在网上找了一段代码，记一下。

```javascript
(function($) {
    $.fn.extend({
        insertContentAtCursor : function(myValue, t) {
            var $t = $(this)[0];
            if (document.selection) { // ie
                this.focus();
                var sel = document.selection.createRange();
                sel.text = myValue;
                this.focus();
                sel.moveStart('character', -l);
                var wee = sel.text.length;
                if (arguments.length == 2) {
                    var l = $t.value.length;
                    sel.moveEnd("character", wee + t);
                    t <= 0 ? sel.moveStart("character", wee - 2 * t
                            - myValue.length) : sel.moveStart(
                            "character", wee - t - myValue.length);
                    sel.select();
                }
            } else if ($t.selectionStart
                    || $t.selectionStart == '0') {
                var startPos = $t.selectionStart;
                var endPos = $t.selectionEnd;
                var scrollTop = $t.scrollTop;
                $t.value = $t.value.substring(0, startPos)
                        + myValue
                        + $t.value.substring(endPos,
                                $t.value.length);
                this.focus();
                $t.selectionStart = startPos + myValue.length;
                $t.selectionEnd = startPos + myValue.length;
                $t.scrollTop = scrollTop;
                if (arguments.length == 2) {
                    $t.setSelectionRange(startPos - t,
                            $t.selectionEnd + t);
                    this.focus();
                }
            } else {
                this.value += myValue;
                this.focus();
            }
        }
    })
})(jQuery);

```

代码测试:
```
<html>
<head>
$(document).ready(function(){
    $("#ch_button").click( function () { 
        $("#test_in").insertContentAtCursor("<upload/day_140627/201406271546349972.jpg>"); 
    });
});
</script>
</head>
 
<body >
<button id="ch_button" value="插入" >插入</button>
<textarea name="content" id="test_in" rows="30" cols="100">
 
</textarea>
</body>
</html>
```

需要注意的是，若传参的字符串由JQUERY对象传入，需要toString调用
```javascript
$("#test_in").insertContentAtCursor($("#id").val().toString());
```