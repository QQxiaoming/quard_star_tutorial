(: Name: combiningnodeseqhc2 :)
(: Description: Simple combination of node sequences involving integers.  uses "union" operator:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in (
(<hours>0</hours>,<hours>1</hours>) union ($input-context1//hours))
order by number($h)
return $h