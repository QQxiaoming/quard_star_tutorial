(: Name: WhereExpr002 :)
(: Description: For+Where+Return - error, variable in 'Where' Expr hasn't been defined :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $folder in $input-context//Folder
where $folde/@name = "ABC"
return <folder/>

