(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $p in $input-context//section[section.title = "Procedure"]
where not(some $a in $p//anesthesia satisfies
        $a << ($p//incision)[1] )
return $p 