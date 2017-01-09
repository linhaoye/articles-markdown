前几天同事发现一页上的bug, 从后台编辑框添加的产品详请描述, 在前台样式有乱.
查了下模板文件, 发现代码如下

```html
<div class="allPj">
    <span class="tit m">
        <i class="icon tit-log"></i>
        <em class="tit-1">其它</em>
    </span>
    <p class="con"><?php echo $pro_info['other_infomation'];?></p>
</div>
```

之前调了好久css样式,没发现问题, 后来网查了下, 发现原来是&lt;p&gt;标签的嵌套问题.

p标签不能嵌套块级元素甚至不能嵌套p元素,只能嵌套内联元素,不然对p嵌套的块级元素设置css不起作用.