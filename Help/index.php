<html>
<head>
<title>Dscaler Help</title>
</head>
<frameset rows="*,60">
    <frameset cols="200,*">
         <frame name="contents" target="main" src="HelpContents.htm" scrolling="no">
         <frame name="main" src="<?php
                                       if (isset($helppage)) 
                                       {
                                           echo $helppage;
                                       }
                                       else
                                       {
                                           echo "Help.htm";
                                       }
                                 ?>" scrolling="auto">
    </frameset>
    <frame name="Nav" target="_top" src="Nav.htm" scrolling="no">
<noframes>
<body>
<p>This page uses frames, but your browser doesn't support
them.</p>
</body>
</noframes>
</frameset>
</html>

