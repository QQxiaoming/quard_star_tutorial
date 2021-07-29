(: Name: combiningnodeseqhc4 :)
(: Description: Simple combination of node sequences involving integers and repetition.  uses "union" operator:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in (
(<hours>0</hours>,<hours>40</hours>) union ($input-context1//hours))
order by number($h)
return $h