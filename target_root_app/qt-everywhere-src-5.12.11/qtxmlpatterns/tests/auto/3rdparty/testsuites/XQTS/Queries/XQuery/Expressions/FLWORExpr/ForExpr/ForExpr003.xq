(: Name: ForExpr003 :)
(: Description: For+Return - use predicate in 'IN' Expr :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $f in $input-context/MyComputer/Drive4/Folder[@id="128"]
return <FolderName>{$f/@name}</FolderName>


