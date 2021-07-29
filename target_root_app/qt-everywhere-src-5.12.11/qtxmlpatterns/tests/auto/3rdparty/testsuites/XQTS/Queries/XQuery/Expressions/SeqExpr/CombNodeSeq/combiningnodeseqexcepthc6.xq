(: Name: combiningnodeseqexcepthc6 :)
(: Description: Simple combination of node sequences involving multiple xml data sources.  Use "except" operator. :)

(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

for $h in (
($input-context1//hours) except ($input-context2//grade))
order by number($h)
return $h