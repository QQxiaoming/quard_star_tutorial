(: Name: Orderexpr-20 :)
(: Written by: Andreas Behm :)
(: Description: FLWOR unordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

unordered {
for $i in ($input-context//part[@partid = 1], $input-context//part[@partid = 2]),
    $j in ($input-context//part[@partof = $i/@partid])
where ($i/@partid + $j/@partid) < 7
return $i/@partid + $j/@partid
}
