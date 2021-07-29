(: Name: Orderexpr-19 :)
(: Written by: Andreas Behm :)
(: Description: FLWOR ordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

ordered {
for $i in ($input-context//part[@partid = 1], $input-context//part[@partid = 2]),
    $j in ($input-context//part[@partof = $i/@partid])
where ($i/@partid + $j/@partid) < 7
return $i/@partid + $j/@partid
}
