(: Name: ReturnExpr001 :)
(: Description: For+Return - use element's text to construct new element :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $folder in $input-context/MyComputer/Drive2//Folder
return 
<newFolder>{ $folder/FolderName/text() }</newFolder>

