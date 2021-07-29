(: Name: combiningnodeseqhc3 :)
(: Description: Simple combination of node sequences involving integers and repetition.  uses "|" operator:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in (
(<hours>0</hours>,<hours>40</hours>) | ($input-context1//hours))
order by number($h)
return $h