(: Name: WhereExpr004 :)
(: Description: For+Where+Return - filters in 'Where' expr and in 'In' expr using predicate :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $f in $input-context/MyComputer/Drive1/Folder[@creator="Mani"]/File
where $f/@creation_date="08/06/00"
return $f

