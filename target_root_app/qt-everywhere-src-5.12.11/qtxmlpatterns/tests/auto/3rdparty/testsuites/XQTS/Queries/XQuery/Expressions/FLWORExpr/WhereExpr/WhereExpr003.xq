(: Name: WhereExpr003 :)
(: Description: For+Where+Return - using same predicate (as FLWOR003) in 'Where' predicate :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $d in $input-context/MyComputer/Drive4
where $d/Folder[@id="128"]
return <FolderName>{$d/Folder/@name}</FolderName>

