(: Name: ForExpr005 :)
(: Description: For+Return - use sequence(security/right) as the predicate :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $f in $input-context/MyComputer//File[@creation_date="08/06/00"]/SecurityObject/Denies/Deny[security/right]
 return $f
