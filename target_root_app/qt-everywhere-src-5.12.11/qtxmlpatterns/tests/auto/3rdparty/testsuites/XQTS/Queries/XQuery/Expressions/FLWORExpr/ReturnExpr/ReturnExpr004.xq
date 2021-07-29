(: Name: ReturnExpr004 :)
(: Description: For+Where+Return - filters in 'Where' expr and in 'Return' expr using predicate :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $f in $input-context/MyComputer//File
where $f/@creation_date="08/06/00"
return $f/SecurityObject/Denies/Deny[security/right]
