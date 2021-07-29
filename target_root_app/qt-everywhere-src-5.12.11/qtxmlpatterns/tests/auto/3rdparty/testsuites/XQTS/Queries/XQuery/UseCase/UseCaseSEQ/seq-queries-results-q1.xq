(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $s in $input-context//section[section.title = "Procedure"]
return ($s//incision)[2]/instrument