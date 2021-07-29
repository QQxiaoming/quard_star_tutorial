(: Name: XQueryComment010 :)
(: Description: Comments inside a conditional expression :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

if (: comment :) 
  ( $input-context//fs:Folder[1]/fs:FolderName/text() = "Folder00000000000" ) 
then (: this is the then case :) ( <true/> )
else (: this is the else case :) ( <false/> )

