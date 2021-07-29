(: Name: ReturnExpr002 :)
(: Description: For+Return - use existing attribute to generate new attribute for new element :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $folder in $input-context/MyComputer/Drive2//Folder
return 
<newFolder>{ $folder/@name, $folder/FolderName/text() }</newFolder>

