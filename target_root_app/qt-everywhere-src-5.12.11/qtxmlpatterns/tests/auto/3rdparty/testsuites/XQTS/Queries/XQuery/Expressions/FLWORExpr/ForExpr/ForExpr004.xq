(: Name: ForExpr004 :)
(: Description: For+Return - use more than one predicates in 'IN' Expr :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $f in $input-context/MyComputer/Drive1/Folder[@creator="Mani"]/File[@creation_date="08/06/00"]
return  <file>{$f/../@creator}{$f/@creation_date}</file>


