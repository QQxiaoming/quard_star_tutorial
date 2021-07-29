(: Name: combiningnodeseqhc7 :)
(: Description: Simple combination of node sequences involving different children of xml data source.  Uses "|" operator:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in (
($input-context1//hours[xs:integer(.) le 20]) | ($input-context1//hours[xs:integer(.) gt 20]))
order by number($h)
return $h