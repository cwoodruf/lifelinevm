<?php
define('DEF_LIMIT',20);
$def_limit = DEF_LIMIT;
define('DEF_THUMB_DIM',125);
$def_thumb_dim = DEF_THUMB_DIM;
define('DEF_QUALITY',90);
$def_quality = DEF_QUALITY;
$img_ext_pat = "#\.(jpg|JPG|mp2|mpeg)$#";
$im_convert = "/usr/local/bin/convert";
############################### functions #####################################
# process directory
function mk_thumbs_dir ($dir,$overwrite = 0,$max_dim = DEF_THUMB_DIM,$quality = DEF_QUALITY) {
	global $im_convert;
        if (is_array($dir)) {
		$c = 0;
		$tmp = $dir[$c++];
		$overwrite = $dir[$c++];
                $max_dim = $dir[$c++];
                $quality = $dir[$c++];
                $dir = $tmp;
        }
        if (!is_dir("$dir/thumbs")) mkdir("$dir/thumbs") or die("can't make thumbs directory!");
	$cwd = getcwd();
        chdir($dir) or die("can't cd to $dir!");
        if (!@stat('thumbs')) mkdir(thumbs);
        if (!is_dir('thumbs')) die("thumbs is not a directory!");
        $dh = opendir('.') or die("can't open directory $dir!");
        while (($item=readdir($dh)) !== false) {
                if (!preg_match("#\.jpg$#",$item,$parts)) continue;
                if (!is_file($item)) continue;
                if ($overwrite or !is_file("thumbs/$item")) {
			if (isset($im_convert)) {
				$dims = $max_dim."x".$max_dim;
				$pre_opts = "-size $dims";
				$post_opts = "-resize $dims -quality $quality";
				$err = exec("$im_convert $pre_opts $item $post_opts thumbs/$item");
			} else {
				$err = makeThumbnail($item,"thumbs/$item",$max_dim,$quality);
			}
			if($err != '') die($err);
                }
                $thumbs["$dir/thumbs/$item"] = "$dir/$item";
        }
	chdir($cwd) or die("can't cd back to $cwd!");
        return $thumbs;
}
#============================================================================#
function fake_thumbs_table(
	$dir,
	$offset = 0,
	$limit = DEF_LIMIT,
	$url="/process",
	$thumb_scr="thumb.php",
	$width=5,
	$max_dim = DEF_THUMB_DIM,
	$quality = DEF_QUALITY
	) {
	global $img_ext_pat;
        if (is_array($dir)) {
		$c = 0;
                $tmp = $dir[$c++];
		$offset = $dir[$c++];
		$limit = $dir[$c++];
		$url = $dir[$c++];
		$width = $dir[$c++];
		$thumb_scr = $dir[$c++];
                $max_dim = $dir[$c++];
                $quality = $dir[$c++];
		$dir = $tmp;
        }
        $cwd = getcwd();
        chdir($dir) or die("can't cd to $dir!");
        $dh = opendir('.') or die("can't open directory $dir!");
	$count = 0;
	$divide = 1;
	$end = $offset + $limit;
        while (($item=readdir($dh)) !== false) {
                if (!is_file($item)) continue;
                if (!preg_match($img_ext_pat,$item,$parts)) continue;
		if ($offset > $count++) continue;
		if ($count > $end) break;
                $thumb = <<<HTML

<a href="$url/$dir/$item"><img src="$url/$thumb_scr?img=$dir/$item&max_dim=$max_dim&quality=$quality"></a>
&nbsp;&nbsp;
HTML;
		if (($divide++ % $width) == 0) $thumb .= "<br>\n";
		$thumbs .= $thumb;
        }
        chdir($cwd) or die("can't cd back to $cwd!");
        return $thumbs;
}
#=============================================================================#
# from a NY php presentation on graphics programming
# http://www.nyphp.org/content/presentations/GDintro/gd20.php
/**
* @return string
* @param $o_file string    Filename of image to make thumbnail of
* @param $t_file string    Filename to use for thumbnail
* @desc Takes an image and makes a jpeg thumbnail defaulted to 100px high
*/
function makeThumbnail($o_file, $t_file, $t_dim = DEF_THUMB_DIM, $quality = DEF_QUALITY) {
    $image_info = getImageSize($o_file) ; // see EXIF for faster way
    
    switch ($image_info['mime']) {
        case 'image/gif':
            if (imagetypes() & IMG_GIF)  { // not the same as IMAGETYPE
                $o_im = imageCreateFromGIF($o_file) ;
            } else {
                $ermsg = 'GIF images are not supported<br />';
            }
            break;
        case 'image/jpeg':
            if (imagetypes() & IMG_JPG)  {
                $o_im = imageCreateFromJPEG($o_file) ;
            } else {
                $ermsg = 'JPEG images are not supported<br />';
            }
            break;
        case 'image/png':
            if (imagetypes() & IMG_PNG)  {
                $o_im = imageCreateFromPNG($o_file) ;
            } else {
                $ermsg = 'PNG images are not supported<br />';
            }
            break;
        case 'image/wbmp':
            if (imagetypes() & IMG_WBMP)  {
                $o_im = imageCreateFromWBMP($o_file) ;
            } else {
                $ermsg = 'WBMP images are not supported<br />';
            }
            break;
        default:
            $ermsg = $image_info['mime'].' images are not supported<br />';
            break;
    }
    
    if (!isset($ermsg)) {
        $o_wd = imagesx($o_im) ;
        $o_ht = imagesy($o_im) ;
	if ($o_wd > $o_ht) {
		$t_wd = $t_dim;
		$t_ht = round($o_ht * $t_dim/$o_wd);
	} else {
		$t_ht = $t_dim;
		$t_wd = round($o_wd * $t_dim/$o_ht);
	}
        $t_im = imageCreateTrueColor($t_wd,$t_ht);
        # make smaller image and print it
        imageCopyResampled($t_im, $o_im, 0, 0, 0, 0, $t_wd, $t_ht, $o_wd, $o_ht);
        imageJPEG($t_im,$t_file,$quality);
        # cleanup
        imageDestroy($o_im);
        imageDestroy($t_im);
    }
    return isset($ermsg)?$ermsg:NULL;
}

