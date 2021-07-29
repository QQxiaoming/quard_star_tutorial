(: Name: combiningnodeseqexcepthc5 :)
(: Description: Simple combination of node sequences involving multiple xml data sources and repetition of a sequence.  Use "except" operator. :)
(: Use count to avoid empty file results :)
(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

for $h in (
count(($input-context1//hours) except ($input-context2//grade,$input-context1//hours)))
order by number($h)
return $h