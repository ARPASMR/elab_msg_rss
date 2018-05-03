<?php
if(isset($_GET)) foreach ($_GET as $key => $value) $$key = $value;


//-----------------------------------------------------------------------------------------
// PHP slide
//-----------------------------------------------------------------------------------------

// getting present gal from titlebar
header("Content-type: text/html; charset=utf-8");
if($_GET) { extract($_GET, EXTR_PREFIX_SAME, "get_"); }
if (!isset($gal)) { $gal = 1; }

// getting gif and jpg images from the current dir
//$dir= "gallery".$gal;
$dir= ".";
@$d = dir($dir);
if ($d) {
        while($entry=$d->read()) {
                $entry = preg_replace("/ /","%20",$entry);
                $posa = strpos (strtolower($entry), ".jpg");
		$posa = strpos (strtolower($entry), ".png");
                $posb = strpos (strtolower($entry), ".gif");
                if (!($posa === false) || !($posb === false)) {
                        $pics[] = $dir."/".$entry;
                }
        }
        $d->close();
        sort($pics);
	$aaa = count($pics);
}

?>

<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">


<html>
<head>
<title>RSS ENHANCED IR 10.8</title>

<!-- Stye Sheet -->
<style type="text/css">
  body { background-color: #eeeeee; font:10px/16px verdana; color: black; text-align:center; }
  a { color: black; }
  a:hover { color: #cc0000; }
  div { margin:20px; }
  #pic { border:1px solid black; }
</style>

<body onKeyPress="OnKeyPressCounter(event)" />

<!-- Javascript for gallery navigation -->
<script type="text/javascript">

var curr = <?php if($aaa) { echo (sizeof($pics)-1); } else { echo (sizeof($pics)-1) ; } ?>;

var pics = new Array(<?php
// php loop for printing all the pics into javascript
for ($i=0; $i<sizeof($pics); $i++) {
        echo "\"$pics[$i]\"";
        if ($i != sizeof($pics)-1) { echo ","; }
}  ?>)

var comments = new Array(<?php
// function to slash strange characters for javascript
function addjsslashes($str) { return addcslashes($str, "\0..\37!@\@\177..\377\'\""); }
// php loop for printing all the comments into javascript
for ($i=0; $i<sizeof($comments); $i++) {
        echo "\"".addjsslashes($comments[$i])."\"";
        if ($i != sizeof($comments)-1) { echo ","; }
} ?>)

function nextPic() {
	if (curr == pics.length-1) { curr = pics.length-1;} else { curr++; }
        showPic(curr);
};

function prevPic() {
        if (curr == 0) { curr = 0; } else { curr--; }
        showPic(curr);
};

function skipPrev() {
        if (curr < 24) { curr = 0; } else { curr-=24; }
        showPic(curr);
};

function skipNext() {
        if (curr > pics.length-24) { curr = pics.length-1; } else { curr+=24; }
        showPic(curr);
};

function showPic (curr) {
        document.getElementById('cnt').childNodes[0].nodeValue = (curr+1)+"-"+pics.length;
        document.getElementById('pic').src = pics[curr];
        document.getElementById('pic').alt = comments[curr];
//        document.getElementById('desc').childNodes[0].nodeValue = comments[curr];
        return false;
};

function lastPic() {
        { curr=pics.length-1; }
        showPic(curr);
};

function firstPic() {
        { curr=0; }
        showPic(curr);
};

function OnKeyPressCounter (event) {
            var characterCode = event.charCode;
            if (characterCode == undefined) {
                characterCode = event.keyCode;
            }

            if (characterCode == 43 /* + */) {
                nextPic();
                }

            if (characterCode == 45 /* - */) {
                prevPic();
                }

        };

</script>


</head>
<body>

<!-- layer with the navigation -->

<div id="nav">
	<a href="../"><font color="red">[Parent Directory]</font></a>
	&nbsp&nbsp<a onclick="firstPic()" href="#">First</a> |
        <?php if(sizeof($pics) > 26)  ?> <a onclick="skipPrev()" href="#">Skip 24</a> |
        <a onclick="prevPic()" href="#">Prev</a>
        | <span id="cnt"><?php echo $aaa ?> di <?php echo $aaa ?></span> |
        <a onclick="nextPic()" href="#">Next</a> |
        <?php if(sizeof($pics) > 26) ?><a onclick="skipNext()" href="#">Skip 24</a> |
	<a onclick="lastPic()" href="#">Last</a>
</div>

<!-- layer with the links -->
<div id="links">
<?php
for ($i=1; $i<=sizeof($links); $i++) {
	echo "<a href=\"?gal=".$links[$i]."\">".$links[$i]."</a>";
	if ( $i<sizeof($links)) { echo " | "; }
}
?>
</div>


<!-- layer with the image -->
<div id="picture">
<img id="pic" src="<?php if($pic) { echo $pics[$pic]; } else { echo $pics[$aaa-1]; } ?>" alt="" />
</div>


<!-- layer with the comments 
<div id="desc"><?=$comments[0]?></div>-->





</body>
</html>
