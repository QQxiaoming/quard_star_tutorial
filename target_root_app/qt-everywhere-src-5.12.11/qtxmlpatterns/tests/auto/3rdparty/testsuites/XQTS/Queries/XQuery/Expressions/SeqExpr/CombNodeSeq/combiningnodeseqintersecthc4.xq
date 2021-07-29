(: Name: combiningnodeseqintersecthc4 :)
(: Description: Simple combination of sequences, where second sequence intersect first sequence (multiple data sources).  Use "intersect" operator. :)

(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

for $h in (
($input-context1//hours) intersect ($input-context1//hours, $input-context2//grade))
order by number($h)
return $h