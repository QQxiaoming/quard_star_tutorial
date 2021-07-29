(: Name: combiningnodeseqhc8 :)
(: Description: Simple combination of node sequences involving xml data source.  Uses "union" operator:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in (
($input-context1//hours[xs:integer(.) le 20]) union ($input-context1//hours[xs:integer(.) gt 20]))
order by number($h)
return $h