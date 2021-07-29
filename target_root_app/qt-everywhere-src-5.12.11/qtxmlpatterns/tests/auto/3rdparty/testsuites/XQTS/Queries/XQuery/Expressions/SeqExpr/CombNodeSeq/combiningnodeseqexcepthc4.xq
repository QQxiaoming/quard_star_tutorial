(: Name: combiningnodeseqexcepthc4 :)
(: Description: Simple combination of node sequences that evaluates to the empty sequence.  Use "except" operator. :)
(: Use count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in (
 count(($input-context1//hours) except ($input-context1//hours)))
order by number($h)
return $h