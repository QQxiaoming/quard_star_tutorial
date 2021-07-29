(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $i2 := ($input-context//incision)[2]
for $a in ($input-context//action)[. >> $i2][position()<=2]
return $a//instrument 