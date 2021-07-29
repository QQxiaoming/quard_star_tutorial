(: Name: combiningnodeseqhc6 :)
(: Description: Simple combination of node sequences involving integers and the empty sequence.  Uses "union" operator:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in (
() union ($input-context1//hours))
order by number($h)
return $h