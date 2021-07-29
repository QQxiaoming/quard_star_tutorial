(: Name: combiningnodeseqhc9 :)
(: Description: Simple combination of node sequences involving multiple xml data sources.  Uses "|" operator:)

(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

for $h in (
($input-context1//hours) | ($input-context2//grade[xs:integer(.) gt 12]))
order by number($h)
return $h